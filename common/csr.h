// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef CSR_H
#define CSR_H

#define MSTATEEN0       0x30c
#define MNSTATUS        0x744
#define MIREG2          0x352
#define SIREG2          0x152
#define MIREG3          0x353
#define SIREG3          0x153
#define MIREG4          0x355
#define SIREG4          0x155
#define MIREG5          0x356
#define SIREG5          0x156
#define MIREG6          0x357
#define SIREG6          0x157
#define MPISTATUS       0x346
#define SPISTATUS       0x146
#define MITHRESHOLD     0x347
#define SITHRESHOLD     0x147
#define MIPREEMPTCFG    0x348
#define MSPCS           0x349
#define SSPCS           0x149
#define MIVT            0x307
#define SIVT            0x107
#define MEIVT           0x308
#define SEIVT           0x108

#define STR(X) #X

#define csr_write(csr, val)  asm volatile("csrw " STR(csr) ", %0" :: "r"(val)  : "memory")
#define csr_read(csr, val)   asm volatile("csrr %0, " STR(csr)    : "=r"(val) :: "memory")
#define csr_set(csr, bits)   asm volatile("csrs " STR(csr) ", %0" :: "r"(bits) : "memory")
#define csr_clear(csr, bits) asm volatile("csrc " STR(csr) ", %0" :: "r"(bits) : "memory")

//#define csrrw(csr, old, new) asm volatile("csrrw %0, " STR(csr) ", %1" : "=r"(old) : "r"(new) : "memory")

#define mclaim(val)          asm volatile("csrrw %0, mtopei, x0" : "=r"(val) :: "memory")
#define sclaim(val)          asm volatile("csrrw %0, stopei, x0" : "=r"(val) :: "memory")

#endif
