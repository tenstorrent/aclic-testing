// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// Run-time feature detection.  An ACLIC sub-extension that is not enabled
// leaves its CSR unimplemented (or its instruction illegal), so we detect a
// feature by attempting the access under the M-mode trap handler's "expected
// exception" path: if it raises illegal-instruction the feature is absent.
//
// This lets one ELF run unchanged under many extension configurations: each
// test skips (via skip_unless) the blocks whose feature is not present.

#ifndef PROBE_H
#define PROBE_H

#include "handlers.h"
#include "interrupt.h"
#include "csr.h"
#include "ireg.h"
#include "xlen.h"

// Run `stmt` with the trap handler set to catch (and step over) a synchronous
// exception.  Evaluates to 1 if `stmt` did not trap, 0 if it raised one.
#define probe_legal(stmt) ({          \
        set_excp_ok();                 \
        stmt;                          \
        uint64_t _m = excp_mcause;     \
        excp_ok = 0;                   \
        _m == 0;                       \
    })

// ACLIC machine indirect access (Smaia/Smcsrind): miselect + mireg* usable.
static inline int have_aclic_m(void)
{
    return probe_legal({ csr_write(miselect, 0x1000);
                         xlreg_t v = 0; csr_read_num(MIREG2, v); (void)v; });
}

// ACLIC supervisor indirect access through sireg3 (Sscsrind): sireg3 usable.
static inline int have_sireg3(void)
{
    return probe_legal({ csr_write(siselect, 0x1000);
                         xlreg_t v = 0; csr_read_num(SIREG3, v); (void)v; });
}

// Smaia external-interrupt top (mtopei).
static inline int have_mtopei(void)
{
    return probe_legal({ xlreg_t v = 0; csr_read(mtopei, v); (void)v; });
}

// Smnip: mtopsi / mistatus / mithreshold implemented.
static inline int have_smnip(void)
{
    return probe_legal({ xlreg_t v = 0; csr_read_num(MTOPSI, v); (void)v; });
}

// Smcsps: mspcs implemented.
static inline int have_smcsps(void)
{
    return probe_legal({ xlreg_t v = 0; csr_read_num(MSPCS, v); (void)v; });
}

// Smijt: mijt implemented.
static inline int have_smijt(void)
{
    return probe_legal({ xlreg_t v = 0; csr_read_num(MIJT, v); (void)v; });
}

// Smip (+Smcsps): the mcspspush instruction is legal in M-mode.
// mistatus.sppush is 0 here (reset / not yet set), so this does not swap sp.
static inline int have_mcspspush(void)
{
    return probe_legal({ asm volatile(".4byte 0x30910173"); });  // mcspspush x2, x2
}

// Smijt or Smeihv: xtvec.MODE bit 1 (vectoring modes 10/11) is writable.
static inline int mtvec_mode_writable(void)
{
    xlreg_t saved = 0; csr_read(mtvec, saved);
    csr_write(mtvec, (saved & ~(xlreg_t)3) | 2);
    xlreg_t got = 0; csr_read(mtvec, got);
    csr_write(mtvec, saved);
    return (got & 3) == 2;
}

#endif
