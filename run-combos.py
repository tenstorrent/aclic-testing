#!/usr/bin/env python3
# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Build every test once per XLEN, then run each test under every valid
# combination of the toggleable ACLIC extensions.  Tests detect at run time
# which extensions are present (see common/probe.h) and SKIP the blocks whose
# feature is absent, so a single ELF runs unchanged under all configurations.
#
# Both the build stage and the run stage run in parallel (ThreadPoolExecutor);
# the build stage fully completes before the run stage starts.  The per-combo
# ISA is passed via whisper's --isa (which overrides the config-file isa), so a
# single static config (common/aclic.json, the aclic block only) is shared by
# all runs -- no per-run temp files, and runs are independent.
#
# Only failing (test, xlen, config) runs are printed; a summary follows.
#
#   ./run-combos.py                         # all combos, both XLENs
#   XLENS="64" ./run-combos.py              # rv64 only
#   JOBS=32 ./run-combos.py                 # cap parallelism (default: nproc)
#   WHISPER=/path/to/whisper ./run-combos.py
#   ./run-combos.py 05_threshold 06_xtopsi  # only these tests
#
# The whisper used must contain the ACLIC fixes (mireg4 miconfig, mireg5/6,
# sireg3, 9-bit mithreshold); these are on master, so the default PATH whisper
# is correct.  Override with WHISPER if needed.

import os
import re
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor, as_completed

HERE = os.path.dirname(os.path.abspath(__file__))
ACLIC_JSON = os.path.join(HERE, "common", "aclic.json")
JOBS = int(os.environ.get("JOBS", os.cpu_count() or 8))

# --- ISA substrate (everything except the toggleable feature extensions) -----
BASE = {
    64: ("rv64imafdcvhus_zicbom_zicbop_zicboz_zicntr_zicond_zicsr_zihintpause_"
         "zihpm_zimop_zawrs_zfa_zfbfmin_zfh_zkr_zcb_zcmop_zba_zbb_zbs_zvbb_zvbc_"
         "zvfbfmin_zvfbfwma_zvfh_zvkg_zvkned_zvknhb_zvqdot_sdtrig_smaia_smcsrind_"
         "smidctrl_smmpm_smnpm_smrnmi_smstateen_ssaia_sscsrind_ssidctrl_sscofpmf_"
         "ssnpm_ssqosid_"
         "sstc_svadu_svinval_svnapot_svpbmt_zifencei"),
    32: ("rv32imacsu_zicsr_zicntr_zicond_zihintpause_zihpm_zimop_zawrs_zcb_zcmop_"
         "zba_zbb_zbs_sdtrig_smaia_smcsrind_smidctrl_smmpm_smnpm_smrnmi_smstateen_"
         "ssaia_sscsrind_ssidctrl_sscofpmf_ssnpm_ssqosid_sstc_svadu_svinval_"
         "svnapot_svpbmt_"
         "zifencei"),
}

# Toggleable feature extensions and their prerequisite (which must also be on).
# Prerequisites match whisper's Isa.cpp dependency rules.
TOGGLES = ["smnip", "ssnip", "smcsps", "sscsps", "smip", "ssip",
           "smijt", "ssijt", "smehv", "ssehv", "smeihv", "sseihv"]
PREREQ = {"ssnip": "smnip", "sscsps": "smcsps", "smip": "smcsps", "ssip": "sscsps",
          "ssijt": "smijt", "smehv": "smijt", "ssehv": "ssijt", "sseihv": "smeihv"}

ANSI = re.compile(r"\x1b\[[0-9;]*m")
CRASH_MARKERS = ("Unexpected exception", "Unexpected interrupt",
                 "Failed stop", "consecutive illegal")


def valid_combos():
    """All dependency-closed subsets of TOGGLES."""
    out = []
    for bits in range(1 << len(TOGGLES)):
        enabled = {t for i, t in enumerate(TOGGLES) if bits & (1 << i)}
        if all(PREREQ[t] in enabled for t in enabled if t in PREREQ):
            out.append(tuple(sorted(enabled)))
    return out


def combo_name(combo):
    return "+".join(combo) if combo else "baseline"


def find_whisper():
    return os.environ.get("WHISPER", "whisper")


def find_tests(argv):
    if argv:
        return argv
    return sorted(d for d in os.listdir(HERE)
                  if re.match(r"\d", d) and os.path.isdir(os.path.join(HERE, d)))


def classify(rc, out):
    out = ANSI.sub("", out)
    fails = [ln.strip() for ln in out.splitlines() if ln.strip().endswith(": FAIL.")]
    skips = sum(1 for ln in out.splitlines() if ln.strip().endswith(": SKIP."))
    passes = sum(1 for ln in out.splitlines() if ln.strip().endswith(": PASS."))
    if fails:
        return "FAIL", fails[0], skips, passes
    for m in CRASH_MARKERS:
        if m in out:
            return "FAIL", m, skips, passes
    if rc != 0:
        return "FAIL", f"whisper exit {rc}", skips, passes
    return "OK", "", skips, passes


def build_all(tests, xlens):
    """Compile every (test, XLEN) in parallel.  Returns list of failures."""
    print(f"Building {len(tests)} tests x {len(xlens)} XLEN "
          f"({JOBS}-way) ...", file=sys.stderr)

    def one(t, x):
        r = subprocess.run(["make", "-s", "-C", os.path.join(HERE, t),
                            "all", f"XLEN={x}"], capture_output=True, text=True)
        return (t, x, r.returncode, r.stdout + r.stderr)

    failed = []
    with ThreadPoolExecutor(max_workers=JOBS) as ex:
        futs = [ex.submit(one, t, x) for t in tests for x in xlens]
        for f in as_completed(futs):
            t, x, rc, log = f.result()
            if rc != 0:
                failed.append((t, x, log))
    for t, x, log in failed:
        print(f"BUILD FAILED  {t}  xlen={x}\n{log}", file=sys.stderr)
    return failed


def run_one(whisper, name, x, t, isa):
    """Run one (config, XLEN, test) and classify the result."""
    elf = os.path.join(HERE, t, f"test-{x}.elf")
    r = subprocess.run([whisper, "--isa", isa, "--configfile", ACLIC_JSON,
                        "-m", "100000", elf], capture_output=True, text=True)
    status, reason, skips, passes = classify(r.returncode, r.stdout + r.stderr)
    return (status, name, x, t, reason, skips, passes)


def main():
    whisper = find_whisper()
    xlens = [int(x) for x in os.environ.get("XLENS", "64 32").split()]
    tests = find_tests(sys.argv[1:])

    if build_all(tests, xlens):
        print("Aborting: build failures.", file=sys.stderr)
        return 1

    combos = valid_combos()
    jobs = [(combo_name(c), x, t, BASE[x] + "".join("_" + e for e in c))
            for c in combos for x in xlens for t in tests]
    total = len(jobs)
    print(f"{len(combos)} configs x {len(xlens)} XLEN x {len(tests)} tests "
          f"= {total} runs ({JOBS}-way)\n", file=sys.stderr)

    results = []
    done = nfail = 0
    # Process pool (not threads): each run is a short subprocess plus Python-side
    # output parsing; a process pool keeps the parsing off the GIL.
    with ProcessPoolExecutor(max_workers=JOBS) as ex:
        futs = [ex.submit(run_one, whisper, name, x, t, isa)
                for (name, x, t, isa) in jobs]
        for f in as_completed(futs):
            rec = f.result()
            results.append(rec)
            done += 1
            if rec[0] == "FAIL":
                nfail += 1
            if done % 64 == 0 or done == total:
                print(f"\r  {done}/{total} runs, {nfail} failed ...",
                      end="", file=sys.stderr, flush=True)
    print("", file=sys.stderr)

    # Only failures, in a deterministic order (test, xlen, config).
    for status, name, x, t, reason, _s, _p in sorted(
            (r for r in results if r[0] == "FAIL"), key=lambda r: (r[3], r[2], r[1])):
        print(f"FAIL  {t:<16} xlen={x}  config={name}\n      {reason}")

    nskip = sum(r[5] for r in results)
    npass_blocks = sum(r[6] for r in results)
    print("\n=== summary ===")
    print(f"runs:           {total}")
    print(f"failed runs:    {nfail}")
    print(f"passed blocks:  {npass_blocks}")
    print(f"skipped blocks: {nskip}")
    print(f"configs:        {len(combos)}  (XLENs: {xlens})")
    if nfail == 0:
        print("ALL PASS")
    return 1 if nfail else 0


if __name__ == "__main__":
    sys.exit(main())
