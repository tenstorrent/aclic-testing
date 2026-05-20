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

all: test.elf

test.elf: $(SRC) $(HDR) ../common/test.ld
	$(CC) $(CFLAGS) -T ../common/test.ld -o test.elf $(CPPFLAGS) $(SRC)
	$(OBJDUMP) -dl test.elf > test.dis

run: test.elf
	$(WHISPER) --configfile ../common/whisper.json -m $(MAX_INST) -f test.log test.elf

clean:
	rm -f *.log *.elf *.dis
