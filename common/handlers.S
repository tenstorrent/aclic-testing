// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "csr.h"
#include "xlen.h"

// 16 XLEN-wide registers are saved on each trap.
#define FRAME (16 * SZREG)

.globl mhandler
.align 3
mhandler:
    csrrw sp, mscratch, sp
    addi sp, sp, -FRAME

    REG_S ra,  0*SZREG(sp)
    REG_S t0,  1*SZREG(sp)
    REG_S t1,  2*SZREG(sp)
    REG_S t2,  3*SZREG(sp)
    REG_S t3,  4*SZREG(sp)
    REG_S t4,  5*SZREG(sp)
    REG_S t5,  6*SZREG(sp)
    REG_S t6,  7*SZREG(sp)

    REG_S a0,  8*SZREG(sp)
    REG_S a1,  9*SZREG(sp)
    REG_S a2, 10*SZREG(sp)
    REG_S a3, 11*SZREG(sp)
    REG_S a4, 12*SZREG(sp)
    REG_S a5, 13*SZREG(sp)
    REG_S a6, 14*SZREG(sp)
    REG_S a7, 15*SZREG(sp)

    jal mhandler_body

    REG_L ra,  0*SZREG(sp)
    REG_L t0,  1*SZREG(sp)
    REG_L t1,  2*SZREG(sp)
    REG_L t2,  3*SZREG(sp)
    REG_L t3,  4*SZREG(sp)
    REG_L t4,  5*SZREG(sp)
    REG_L t5,  6*SZREG(sp)
    REG_L t6,  7*SZREG(sp)

    REG_L a0,  8*SZREG(sp)
    REG_L a1,  9*SZREG(sp)
    REG_L a2, 10*SZREG(sp)
    REG_L a3, 11*SZREG(sp)
    REG_L a4, 12*SZREG(sp)
    REG_L a5, 13*SZREG(sp)
    REG_L a6, 14*SZREG(sp)
    REG_L a7, 15*SZREG(sp)

    addi sp, sp, FRAME
    csrrw sp, mscratch, sp
    mret

.globl shandler
.align 3
shandler:
    csrrw sp, sscratch, sp
    REG_S t1, 0*SZREG(sp)
    REG_S t2, 1*SZREG(sp)

    csrr t1, scause
    bgez t1, 1f

    // interrupt: claim via xtopei (legacy) for compatibility
    csrrw t1, stopei, x0
    j 2f

1:  csrr t2, sepc
    addi t2, t2, 4
    csrw sepc, t2

2:  REG_L t2, 1*SZREG(sp)
    REG_L t1, 0*SZREG(sp)
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
