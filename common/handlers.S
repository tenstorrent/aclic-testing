// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "csr.h"

.globl mhandler
.align 3
mhandler:
    csrrw sp, mscratch, sp
    addi sp, sp, -128

    sd ra,   0(sp)
    sd t0,   8(sp)
    sd t1,  16(sp)
    sd t2,  24(sp)
    sd t3,  32(sp)
    sd t4,  40(sp)
    sd t5,  48(sp)
    sd t6,  56(sp)

    sd a0,  64(sp)
    sd a1,  72(sp)
    sd a2,  80(sp)
    sd a3,  88(sp)
    sd a4,  96(sp)
    sd a5, 104(sp)
    sd a6, 112(sp)
    sd a7, 120(sp)

    jal mhandler_body

    ld ra,   0(sp)
    ld t0,   8(sp)
    ld t1,  16(sp)
    ld t2,  24(sp)
    ld t3,  32(sp)
    ld t4,  40(sp)
    ld t5,  48(sp)
    ld t6,  56(sp)

    ld a0,  64(sp)
    ld a1,  72(sp)
    ld a2,  80(sp)
    ld a3,  88(sp)
    ld a4,  96(sp)
    ld a5, 104(sp)
    ld a6, 112(sp)
    ld a7, 120(sp)

    addi sp, sp, 128
    csrrw sp, mscratch, sp
    mret

.globl shandler
.align 3
shandler:
    csrrw sp, sscratch, sp
    sd t1, (sp)
    sd t2, 8(sp)

    csrr t1, scause
    bgez t1, 1f

    // interrupt: claim via xtopei (legacy) for compatibility
    csrrw t1, stopei, x0
    j 2f

1:  csrr t2, sepc
    addi t2, t2, 4
    csrw sepc, t2

2:  ld t2, 8(sp)
    ld t1, (sp)
    csrrw sp, sscratch, sp
    sret

.data

.globl ecall_dest
ecall_dest: .dword 0

.globl exp_intr_ids
.globl num_exp_intr
.globl intr_ids_seen
.globl num_intr_seen
exp_intr_ids: .skip 64
num_exp_intr: .dword 0
intr_ids_seen: .skip 64
num_intr_seen: .dword 0

.globl excp_ok
.globl excp_mcause
excp_ok: .byte 0
excp_mcause: .dword 0
