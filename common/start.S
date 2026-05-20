// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "common/csr.h"

.section .text.init, "ax"
.globl _start
_start:
    li t0, 0x1f
    csrw pmpcfg0, t0
    li t0, -1
    csrw pmpaddr0, t0

    // scratch buffers are full-descending stacks
    la t0, mscratch_end
    csrw mscratch, t0
    la t0, sscratch_end
    csrw sscratch, t0

    la t0, mhandler
    csrw mtvec, t0

    la t0, shandler
    csrw stvec, t0

    li t0, 1 << 3 // mnstatus.NMIE
    csrs MNSTATUS, t0

    li t0, (1 << 60) | (1 << 53) // CSRIND, ACLIC
    csrs MSTATEEN0, t0

    li t0, 1 << 3 // mstatus.MIE
    csrs mstatus, t0

    li t0, (1 << 11) | (1 << 9) // mie.MEIE, mie.SEIE
    csrs mie, t0

    li a0, 0
    la sp, _estack
    jal main
    j exit

.globl exit
exit:
    li t0, 0xffffffffffff
    slli a0, a0, 1
    and a0, a0, t0
    ori a0, a0, 1
    sd a0, tohost, t0
1:  j 1b

.weak main
main:
    ret

.weak mhandler
mhandler:
    mret

.weak shandler
shandler:
    sret

.data
.align 12
mscratch_begin: .skip 4096
mscratch_end:
sscratch_begin: .skip 4096
sscratch_end:

.section .tohost
.globl tohost
.globl fromhost
tohost: .dword 0
fromhost: .dword 0
