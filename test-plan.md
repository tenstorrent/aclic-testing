# ACLIC Test Plan

This suite targets the May 2026 ACLIC spec rework (PRs #828, #831, #833, #834).
Each numbered directory contains a single ELF that exercises one slice of the
spec; the harness counts a directory as PASS iff `make -C $DIR run` exits 0
(Whisper terminates with `tohost & 1`).

## Coverage map

1. **01_basic** — Source modes, sourcecfg, eip/eie/iprio round-trip. Regression
   that the basic ACLIC interrupt source machinery still works after the rework.
2. **02_delegation** — `sourcecfg.D` bit; S-domain visibility after delegation;
   that M↔S transitions clear stale state.
3. **03_read_only_zero** — `target[0]` read-only zero; `sourcecfg[0]` read-only
   zero; inactive sources read back zero.
4. **04_illegal** — Odd `xiselect` on RV64; `xireg5`/`xireg6` at `xiselect`
   0x1000–0x10FF illegal; `mstateen0.ACLIC` bit 53 gates S-mode ACLIC access.
5. **05_threshold** — Wider 9-bit `mithreshold` / `sithreshold` round-trip;
   threshold suppresses lower-priority interrupts; `xistatus.pithreshprio`
   captured on trap and restored on `xret`.
6. **06_xtopsi** — **NEW**: signed `SIID` encoding (positive for external,
   negative for major); idle returns `SIID=0, IPRIO=xithreshold.iprio`;
   `CSRRW xtopsi` claims + raises threshold (3-step write semantics).
7. **07_xtopei** — `mtopei` / `stopei` still report external-only highest
   pending+enabled per spec PR #831; coexist with xtopsi.
8. **08_miconfig** — `miconfig` via `mireg4` at `xiselect=0x1000`; `siconfig`
   is the S-mode-accessible subset alias; `mnipen`/`snipen` gate Smnip auto
   threshold raise; `mnipbits`/`snipbits` define NIPPRIO_MASK.
9. **09_preemption** — With `miconfig.mnipen=1` and `mnipbits>0`, higher-prio
   sources preempt; pithreshprio is captured/restored across nested traps
   (mirrors the spec's nested-threshold example).
10. **10_mistatus** — `mistatus` aliases `mstatus.{mpp,mpie,mpelp,VS,FS,mie}`
    bidirectionally; `pithreshprio[16:8]` and `sppush[6]`/`psppush[7]` owned.
11. **11_smeihv** — `xtvec.mode=10`: PC = OBASE + 4*SIID for external (positive)
    and major (negative).  Synchronous exception @ OBASE+0.
12. **12_smijt** — `xtvec.mode=11` + `xijt`: BASE alignment, SHAMT shifts
    offset, VEN bit gates per-entry vectoring (VEN=0 falls back to OBASE),
    JTMASK trims handler-address low bits.
13. **13_smehv** — `xijt.EHV` non-zero + mode=11: synchronous exceptions are
    vectored by exception code (PC = OBASE + 4*exccode).
14. **14_smcsps** — `mspcs`/`sspcs` round-trip; `mcspspush` conditionally swaps
    based on `mistatus.sppush`, latches `psppush`; `mcspspop` is the reverse;
    nested handler example from spec §Smcsps reproduces.
15. **15_smip** — With `miconfig.mipu=1`: trap pushes a0–a5 + decrements sp;
    `mipopret` pops them and returns; works with Smcsps swap; store-fault
    during push takes a precise interrupt fault trap.
