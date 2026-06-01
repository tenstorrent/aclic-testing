// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// XLEN-portability helpers.  The register width is fixed at compile time by the
// target ABI (an ELF is XLEN-specific), so everything here resolves to a
// compile-time constant -- there is no run-time XLEN detection or cost.

#ifndef XLEN_H
#define XLEN_H

#ifndef __riscv_xlen
#error "__riscv_xlen is not defined by the compiler"
#endif

#define XLEN __riscv_xlen          // 32 or 64

#ifdef __ASSEMBLER__

// XLEN-wide register store/load and register size, for trap save/restore.
#if __riscv_xlen == 64
# define REG_S sd
# define REG_L ld
# define SZREG 8
#else
# define REG_S sw
# define REG_L lw
# define SZREG 4
#endif

#else  /* C */

#include <stdint.h>

// XLEN-wide unsigned register value: uint32_t on RV32 (ILP32), uint64_t on RV64 (LP64).
typedef unsigned long xlreg_t;
_Static_assert(sizeof(xlreg_t) * 8 == XLEN, "xlreg_t must be XLEN bits wide");

// Layout of the ACLIC packed indirect registers, mirroring whisper's Aclic.cpp:
//   sourcecfg: 16-bit fields, (XLEN/16) per register -> xireg2 (low half), xireg3 (high half)
//   iprio:      8-bit fields, (XLEN/8)  per register -> xireg
#define SCFG_FIELDS  (XLEN / 16)   // 2 (RV32) or 4 (RV64)
#define IPRIO_FIELDS (XLEN / 8)    // 4 (RV32) or 8 (RV64)

#endif /* __ASSEMBLER__ */

#endif
