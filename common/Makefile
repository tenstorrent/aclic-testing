# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0

.PHONY: all run clean

CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
GDB = riscv64-unknown-elf-gdb
WHISPER ?= whisper

SRC = $(wildcard *.[Sc]) $(wildcard ../common/*.[Sc])
HDR = $(wildcard ../common/*.h)

CFLAGS = -g -O3 -flto -mcmodel=medany -nostdlib -Wall -Wextra -pedantic
CPPFLAGS = -I../
CPPFLAGS += -DASSERT_PRINT

MAX_INST = 10000

# XLEN selects the register width.  An ELF is XLEN-specific, so each width gets
# its own toolchain flags, whisper config, and (uniquely named) artifacts.
XLEN ?= 64
ifeq ($(XLEN),32)
  ARCHFLAGS = -march=rv32imac_zicsr -mabi=ilp32
  WHISPER_CONFIG = ../common/whisper-rv32.json
else
  ARCHFLAGS = -march=rv64imac_zicsr -mabi=lp64
  WHISPER_CONFIG = ../common/whisper.json
endif

ELF = test-$(XLEN).elf
DIS = test-$(XLEN).dis
LOG = test-$(XLEN).log

all: $(ELF)

$(ELF): $(SRC) $(HDR) ../common/test.ld
	$(CC) $(CFLAGS) $(ARCHFLAGS) -T ../common/test.ld -o $(ELF) $(CPPFLAGS) $(SRC)
	$(OBJDUMP) -dl $(ELF) > $(DIS)

run: $(ELF)
	$(WHISPER) --configfile $(WHISPER_CONFIG) -m $(MAX_INST) -f $(LOG) $(ELF)

clean:
	rm -f *.log *.elf *.dis
