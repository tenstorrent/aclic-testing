# ACLIC Testing

This is a test suite for the upcoming [ACLIC
specification](https://github.com/riscv/riscv-fast-interrupt) being drafted by
the Fast Interrupts Task Group.

To compile and run the tests, you will need:

- The RISC-V toolchain
- [Whisper ISS](https://github.com/tenstorrent/whisper/)

## Installing RISC-V Toolchain

The RISC-V toolchain is available via a variety of package managers:

Debian:

```bash
sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf
```

Arch:

```bash
sudo pacman -S riscv64-elf-gcc riscv64-elf-binutils
```

RedHat:

```bash
sudo dnf install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf
```

MacOS:

```bash
brew tap riscv-software-src/riscv
brew install riscv-tools
```

You can confirm that the installation was successful with:

```bash
riscv64-unknown-elf-gcc --version
```

## Installing Whisper

The steps to obtain and compile whisper can be found below. Whisper requires
Boost and a C++20 compiler. The Makefile assumes that the `BOOST_ROOT`
environment variable points to your Boost installation.

```bash
git clone https://github.com/tenstorrent/whisper.git
cd whisper
make
```

Once Whisper has been compiled, you may wish to add it to your `PATH`
environment variable. It can be found in the subdirectory beginning with
`build-`.

## Compiling and Running Tests

Each test directory includes a Makefile which points to the shared Makefile in
the `common` subdirectory. This Makefile assumes that the RISC-V toolchain and
Whisper are available on your `PATH` environment variable. The toolchain is
assumed to be called `riscv64-unknown-elf-gcc`, though this may vary.

To compile and run the tests, run `./run-tests.sh`.

## ACLIC Specification

The ACLIC specification is still in draft and details are still changing
frequently. For the purposes of testing, some temporary assumptions have been
made that are likely to change before ratification:

- The CSR bits MIPU and SIPU have not yet been allocated. The test suite
  assumes they are allocated in `mstatus` at bits 25 and 26 respectively
- The ACLIC spec currently allocates CSR address 0x308 for `meivt`, but this is
  already allocated for `mvien` from AIA. The test suite assumes `meivt` is
  allocated to 0x398.
