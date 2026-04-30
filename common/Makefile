# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0

.PHONY: all run

CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
GDB = riscv64-unknown-elf-gdb
WHISPER = whisper

SRC = $(wildcard *.[Sc]) $(wildcard ../common/*.[Sc])
HDR = $(wildcard ../common/*.h)

CFLAGS = -g -O3 -flto -mcmodel=medany -nostdlib -Wall -Wextra -pedantic 
CPPFLAGS = -I../
CPPFLAGS += -DASSERT_PRINT

# Uncomment this if Whisper is compiled with ACLIC_HINTS
#CPPFLAGS += -DACLIC_HINTS

MAX_INST = 10000

all: test.elf

test.elf: $(SRC) $(HDR) ../common/test.ld
	$(CC) $(CFLAGS) -T ../common/test.ld -o test.elf $(CPPFLAGS) $(SRC)
	$(OBJDUMP) -dl test.elf > test.dis

run: test.elf
	$(WHISPER) --hintops --configfile ../common/whisper.json -m $(MAX_INST) -f test.log test.elf

debug:
	gdb --args $(WHISPER) --hintops --configfile ../common/whisper.json -m $(MAX_INST) -l test.elf
	$(GDB) test.elf -ex "target remote | $(WHISPER) --gdb --hintops --configfile ../common/whisper.json test.elf"

clean:
	rm -f *.log *.elf *.dis
