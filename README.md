# ACLIC Testing (post-2026-rework)

Test suite for the Whisper RISC-V ISS exercising the ACLIC implementation
updated to track spec PRs #828, #831, #833, and #834
(https://github.com/riscv/riscv-fast-interrupt).

## Layout

```
common/        shared test framework, CSR helpers, JSON config, linker script
NN_<name>/     numbered test programs (main.c + Makefile stub)
run-tests.sh   harness: iterates [0-9]* and reports PASS/FAIL per directory
```

## Build & run

Each test is a standalone ELF that runs on Whisper.

```
cd 01_basic
make run            # builds test.elf, runs whisper, writes test.log
```

Run the whole suite:

```
./run-tests.sh
```

Requires:
- `riscv64-unknown-elf-gcc` / `riscv64-unknown-elf-objdump` on `PATH`
- `whisper` on `PATH` (or override via `WHISPER=...`)

## Coverage

See `test-plan.md` for the per-directory scope.  The numbering matches the
phase ordering of the implementation rework in `swerv-iss-aclic-pr-833`.
