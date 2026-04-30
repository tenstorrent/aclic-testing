# ACLIC Testing

This is a test suite for the upcoming [ACLIC specification](https://github.com/riscv/riscv-fast-interrupt) being drafted by the Fast Interrupts Task Group.

The tests are RISC-V programs and can be compiled using the RISC-V toolchain.

The ACLIC specification is still in draft and details are still changing frequently. For the purposes of testing, some temporary assumptions have been made that are likely to change before ratification:

- The CSR bits MIPU and SIPU have not yet been allocated. The test suite assumes they are allocated in `mstatus` at bits 25 and 26 respectively
- The ACLIC spec currently allocates CSR address 0x308 for `meivt`, but this is already allocated for `mvien` from AIA. The test suite assumes `meivt` is allocated to 0x398.
