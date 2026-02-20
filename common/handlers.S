.globl mhandler
.align 3
mhandler:
    csrrw sp, mscratch, sp
    addi sp, sp, -24
    sd t1, (sp)
    sd t2, 8(sp)
    sd t3, 16(sp)

    csrr t1, mcause
    bgez t1, 1f

    // interrupt
    ld t3, num_intr_seen
    addi t3, t3, 1
    sd t3, num_intr_seen, t2
    ld t2, num_exp_intr
    bgtu t3, t2, 3f

    addi t3, t3, -1
    slli t3, t3, 3

    li t2, (1 << 63) | 9
    beq t1, t2, 2f
    csrrw t1, mtopei, x0
    j 4f
2:  csrrw t1, stopei, x0
4:  srli t1, t1, 16
    andi t1, t1, 0x7ff
    la t2, intr_ids_seen
    add t2, t2, t3
    sd t1, (t2)
    j 2f

    // exception
1:  li t2, 9 // ecall from S-mode
    beq t1, t2, 1f

    lb t2, excp_ok
    beqz t2, 5f

    sd t1, excp_mcause, t2

    // mepc += 4 (or 2 for C)
    csrr t2, mepc
    lw t1, (t2)
    addi t1, t1, 1
    andi t1, t1, 3
    bnez t1, 4f
    addi t2, t2, 2
4:  addi t2, t2, 2
    csrw mepc, t2
    j 2f

1:  ld t1, ecall_dest
    csrw mepc, t1

    li t1, 0x1800
    csrs mstatus, t1

2:  ld t3, 16(sp)
    ld t2, 8(sp)
    ld t1, (sp)
    addi sp, sp, 24
    csrrw sp, mscratch, sp
    mret

3:  la a0, unexp_intr
    j 1f
5:  la a0, unexp_excp
1:  jal print_str
    li a0, 1
    j exit

.globl shandler
.align 3
shandler:
    csrrw sp, sscratch, sp
    sd t1, (sp)
    sd t2, 8(sp)

    csrr t1, scause
    bgez t1, 1f

    // interrupt
    csrrw t1, stopei, x0
    j 2f

    // exception
    li a0, 1
    j exit

1:  csrr t2, sepc
    addi t2, t2, 8
    csrw sepc, t2

2:  ld t2, 8(sp)
    ld t1, (sp)
    csrrw sp, sscratch, sp
    sret

.data

unexp_intr: .string "Unexpected interrupt\n"
unexp_excp: .string "Unexpected exception\n"

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

