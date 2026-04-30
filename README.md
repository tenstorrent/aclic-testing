# ACLIC Testing

This is a test suite for the upcoming [ACLIC specification](https://github.com/riscv/riscv-fast-interrupt) being drafted by the Fast Interrupts Task Group.

To compile and run the tests, you will need:

- The RISC-V toolchain
- [Whisper ISS](https://github.com/tenstorrent/whisper/)

The Makefile assumes that these are on your path. The toolchain is assumed to be the embedded toolchain, `riscv64-unknown-elf-gcc`.

To compile and run the tests, run `./run-tests.sh`.

The ACLIC specification is still in draft and details are still changing frequently. For the purposes of testing, some temporary assumptions have been made that are likely to change before ratification:

- The CSR bits MIPU and SIPU have not yet been allocated. The test suite assumes they are allocated in `mstatus` at bits 25 and 26 respectively
- The ACLIC spec currently allocates CSR address 0x308 for `meivt`, but this is already allocated for `mvien` from AIA. The test suite assumes `meivt` is allocated to 0x398.
