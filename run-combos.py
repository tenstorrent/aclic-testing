#!/usr/bin/env python3
# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Build every test once per XLEN, then run each test under every valid
# combination of the toggleable ACLIC extensions.  Tests detect at run time
# which extensions are present (see common/probe.h) and SKIP the blocks whose
# feature is absent, so a single ELF runs unchanged under all configurations.
#
# Only failing (test, xlen, config) runs are printed; a summary follows.
#
#   WHISPER=/path/to/whisper ./run-combos.py            # all combos, both XLENs
#   XLENS="64" ./run-combos.py                          # rv64 only
#   ./run-combos.py 05_threshold 06_xtopsi              # only these tests
#
# The whisper used must contain the ACLIC fixes (mireg4 miconfig, mireg5/6,
# sireg3); set WHISPER accordingly (defaults to ./whisper-fixed if present).

import itertools
import json
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))

# --- ISA substrate (everything except the toggleable feature extensions) -----
BASE = {
    64: ("rv64imafdcvhus_zicbom_zicbop_zicboz_zicntr_zicond_zicsr_zihintpause_"
         "zihpm_zimop_zawrs_zfa_zfbfmin_zfh_zkr_zcb_zcmop_zba_zbb_zbs_zvbb_zvbc_"
         "zvfbfmin_zvfbfwma_zvfh_zvkg_zvkned_zvknhb_zvqdot_sdtrig_smaia_smidctrl_"
         "smmpm_smnpm_smrnmi_smstateen_ssaia_ssidctrl_sscofpmf_ssnpm_ssqosid_"
         "sstc_svadu_svinval_svnapot_svpbmt_zifencei"),
    32: ("rv32imacsu_zicsr_zicntr_zicond_zihintpause_zihpm_zimop_zawrs_zcb_zcmop_"
         "zba_zbb_zbs_sdtrig_smaia_smidctrl_smmpm_smnpm_smrnmi_smstateen_ssaia_"
         "ssidctrl_sscofpmf_ssnpm_ssqosid_sstc_svadu_svinval_svnapot_svpbmt_"
         "zifencei"),
}

ACLIC = {"num_sources": 16, "has_supervisor_domain": True, "ipriolen": 8}

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
    idx = {t: i for i, t in enumerate(TOGGLES)}
    out = []
    for bits in range(1 << len(TOGGLES)):
        enabled = {t for i, t in enumerate(TOGGLES) if bits & (1 << i)}
        if all(PREREQ[t] in enabled for t in enabled if t in PREREQ):
            out.append(tuple(sorted(enabled)))
    return out


def combo_name(combo):
    return "+".join(combo) if combo else "baseline"


def find_whisper():
    w = os.environ.get("WHISPER")
    if w:
        return w
    local = os.path.join(HERE, "whisper-fixed")
    return local if os.path.exists(local) else "whisper"


def find_tests(argv):
    if argv:
        return argv
    return sorted(d for d in os.listdir(HERE)
                  if re.match(r"\d", d) and os.path.isdir(os.path.join(HERE, d)))


def build(tests, xlens):
    print(f"Building {len(tests)} tests x {len(xlens)} XLEN ...", file=sys.stderr)
    failed = []
    for t in tests:
        for x in xlens:
            r = subprocess.run(["make", "-s", "-C", os.path.join(HERE, t),
                                "all", f"XLEN={x}"],
                               capture_output=True, text=True)
            if r.returncode != 0:
                failed.append((t, x, r.stdout + r.stderr))
    for t, x, log in failed:
        print(f"BUILD FAILED  {t}  xlen={x}\n{log}", file=sys.stderr)
    return not failed


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


def main():
    whisper = find_whisper()
    xlens = [int(x) for x in os.environ.get("XLENS", "64 32").split()]
    tests = find_tests(sys.argv[1:])

    if not build(tests, xlens):
        print("Aborting: build failures.", file=sys.stderr)
        return 1

    combos = valid_combos()
    total = len(combos) * len(xlens) * len(tests)
    print(f"{len(combos)} configs x {len(xlens)} XLEN x {len(tests)} tests "
          f"= {total} runs\n", file=sys.stderr)

    n = nfail = nskip = npass_blocks = 0
    cfg_path = os.path.join(tempfile.gettempdir(), f"aclic-combo-{os.getpid()}.json")
    for combo in combos:
        name = combo_name(combo)
        for x in xlens:
            isa = BASE[x] + "".join("_" + t for t in combo)
            with open(cfg_path, "w") as f:
                json.dump({"isa": isa, "aclic": ACLIC}, f)
            for t in tests:
                elf = os.path.join(HERE, t, f"test-{x}.elf")
                r = subprocess.run([whisper, "--configfile", cfg_path,
                                    "-m", "100000", elf],
                                   capture_output=True, text=True)
                status, reason, skips, passes = classify(r.returncode,
                                                          r.stdout + r.stderr)
                n += 1
                nskip += skips
                npass_blocks += passes
                if status == "FAIL":
                    nfail += 1
                    print(f"FAIL  {t:<16} xlen={x}  config={name}\n"
                          f"      {reason}")
            print(f"\r  {n}/{total} runs, {nfail} failed ...",
                  end="", file=sys.stderr, flush=True)
    print("", file=sys.stderr)
    try:
        os.remove(cfg_path)
    except OSError:
        pass

    print(f"\n=== summary ===")
    print(f"runs:           {n}")
    print(f"failed runs:    {nfail}")
    print(f"passed blocks:  {npass_blocks}")
    print(f"skipped blocks: {nskip}")
    print(f"configs:        {len(combos)}  (XLENs: {xlens})")
    if nfail == 0:
        print("ALL PASS")
    return 1 if nfail else 0


if __name__ == "__main__":
    sys.exit(main())
