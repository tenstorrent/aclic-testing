.PHONY: all run

SRC = $(wildcard *.[Sc]) $(wildcard ../common/*.[Sc])
HDR = $(wildcard ../common/*.h)

CFLAGS = -g -O3 -flto -mcmodel=medany -nostdlib -Wall -Wextra -pedantic 
CPPFLAGS = -I../
CPPFLAGS += -DASSERT_PRINT
CPPFLAGS += -DACLIC_HINTS

WHISPER = /proj_risc/user_dev/pkennedy/swerv-iss-aclic/build-Linux/whisper

MAX_INST = 10000

all: test.elf

test.elf: $(SRC) $(HDR) ../common/test.ld
	riscv64-unknown-elf-gcc $(CFLAGS) -T ../common/test.ld -o test.elf $(CPPFLAGS) $(SRC)
	riscv64-unknown-elf-objdump -dl test.elf > test.dis

run: test.elf
	$(WHISPER) --hintops --configfile ../common/whisper.json -m $(MAX_INST) -f test.log test.elf

debug:
	gdb --args $(WHISPER) --hintops --configfile ../common/whisper.json -m $(MAX_INST) -l test.elf
	riscv64-unknown-elf-gdb test.elf -ex "target remote | $(WHISPER) --gdb --hintops --configfile ../common/whisper.json test.elf"

clean:
	rm -f *.log *.elf *.dis
