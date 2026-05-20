OUTPUT_ARCH(riscv)
ENTRY(_start)

STACK_SIZE = 0x1000;

SECTIONS {
    . = 0x70000000;
    .tohost : { *(.tohost) }

    . = 0x80000000;
    .text.init : { *(.text.init) }
    .text : { *(.text) }

    . = ALIGN(0x1000);
    .data : { *(.data) }
    .sbss : { *(.sbss) }

    . = ALIGN(0x1000);
    .stack (NOLOAD): {
        _sstack = .;
        . = . + STACK_SIZE;
        _estack = .;
    }
}
