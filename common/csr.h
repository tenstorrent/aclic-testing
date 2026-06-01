// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// CSR numbers for the May 2026 ACLIC spec rework (PRs #828, #831, #833, #834).

#ifndef CSR_H
#define CSR_H

// Standard CSRs we touch from startup code.
#define MSTATEEN0       0x30c
#define MNSTATUS        0x744

// Indirect-access ACLIC CSRs.  Spec §Smidctrl + §Smnip use mireg/2/3 for
// {iprio, sourcecfg} and mireg4 for miconfig.
#define MISELECT        0x350
#define MIREG           0x351
#define MIREG2          0x352
#define MIREG3          0x353
#define MIREG4          0x355
#define MIREG5          0x356
#define MIREG6          0x357
#define SISELECT        0x150
#define SIREG           0x151
#define SIREG2          0x152
#define SIREG3          0x153
#define SIREG4          0x155
#define SIREG5          0x156
#define SIREG6          0x157

// Smidctrl/Ssidctrl
#define MTOPEI          0x35c
#define STOPEI          0x15c
#define MTOPSI          0x348
#define STOPSI          0x148

// Smnip/Ssnip
#define MISTATUS        0x346
#define SISTATUS        0x146
#define MITHRESHOLD     0x347
#define SITHRESHOLD     0x147

// Smcsps/Sscsps
#define MSPCS           0x349
#define SSPCS           0x149

// Smijt/Ssijt
#define MIJT            0x307
#define SIJT            0x107

#define STR(X) #X
#define XSTR(X) STR(X)     // double-expand: XSTR(macro) -> "value-of-macro"

#define csr_write(csr, val)  asm volatile("csrw " STR(csr) ", %0" :: "r"(val)  : "memory")
#define csr_read(csr, val)   asm volatile("csrr %0, " STR(csr)    : "=r"(val) :: "memory")
#define csr_set(csr, bits)   asm volatile("csrs " STR(csr) ", %0" :: "r"(bits) : "memory")
#define csr_clear(csr, bits) asm volatile("csrc " STR(csr) ", %0" :: "r"(bits) : "memory")

// Numeric-CSR variants for CSRs the assembler doesn't know by name (e.g. MTOPSI).
#define csr_write_num(csr, val)  asm volatile("csrw " XSTR(csr) ", %0" :: "r"(val) : "memory")
#define csr_read_num(csr, val)   asm volatile("csrr %0, " XSTR(csr)    : "=r"(val) :: "memory")
#define csr_set_num(csr, bits)   asm volatile("csrs " XSTR(csr) ", %0" :: "r"(bits) : "memory")
#define csr_clear_num(csr, bits) asm volatile("csrc " XSTR(csr) ", %0" :: "r"(bits) : "memory")

#define mclaim(val)          asm volatile("csrrw %0, mtopei, x0" : "=r"(val) :: "memory")
#define sclaim(val)          asm volatile("csrrw %0, stopei, x0" : "=r"(val) :: "memory")

// xtopsi claim-and-raise.  Source bits[16:8] become the new xithreshold; the
// returned value is the pre-side-effect xtopsi snapshot.
#define mclaim_topsi(out, raise) \
    asm volatile("csrrw %0, " XSTR(MTOPSI) ", %1" : "=r"(out) : "r"(raise) : "memory")
#define sclaim_topsi(out, raise) \
    asm volatile("csrrw %0, " XSTR(STOPSI) ", %1" : "=r"(out) : "r"(raise) : "memory")

#endif
