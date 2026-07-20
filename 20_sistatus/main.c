// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 20_sistatus — sistatus aliases sstatus.{spp,spie,sie} and owns
// pithreshprio + sppush/psppush (Ssnip), the S-mode counterpart of 10_mistatus.
//
// sistatus layout (per whisper defineAclicRegs sistatusMask):
//   [28] spp (alias sstatus.SPP), [27] spie (alias sstatus.SPIE),
//   [16:8] pithreshprio (owned), [0] sie (alias sstatus.SIE).

#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"
#include <stdint.h>

static inline uint64_t read_sistatus(void)        { xlreg_t v; csr_read_num(SISTATUS, v); return v; }
static inline void     write_sistatus(uint64_t v) { csr_write_num(SISTATUS, v); }
static inline uint64_t read_sstatus(void)         { xlreg_t v; csr_read(sstatus, v); return v; }

int main(void)
{
    test("sistatus_spp_alias_reads_sstatus_spp") {
        skip_unless(have_ssnip());
        csr_set(sstatus, 1ull << 8);      // SPP = 1
        uint64_t sis = read_sistatus();
        fail_if(((sis >> 28) & 0x1) != 1);
        csr_clear(sstatus, 1ull << 8);
    }

    test("sistatus_spie_alias_reads_sstatus_spie") {
        skip_unless(have_ssnip());
        csr_set(sstatus, 1ull << 5);      // SPIE = 1
        uint64_t sis = read_sistatus();
        fail_if(((sis >> 27) & 0x1) != 1);
        csr_clear(sstatus, 1ull << 5);
    }

    test("sistatus_pithreshprio_owned_writable") {
        skip_unless(have_ssnip());
        write_sistatus(0x55ull << 8);     // pithreshprio = 0x55
        uint64_t got = read_sistatus();
        fail_if(((got >> 8) & 0x1FF) != 0x55);
        write_sistatus(0);
    }

    test("sistatus_sie_alias_writethrough") {
        skip_unless(have_ssnip());
        csr_clear(sstatus, 1ull << 1);    // SIE = 0
        uint64_t cur = read_sistatus();
        write_sistatus(cur | 1ull);       // sistatus.sie (bit 0) -> sstatus.SIE (bit 1)
        uint64_t sst = read_sstatus();
        fail_if(((sst >> 1) & 1) != 1);
        csr_clear(sstatus, 1ull << 1);
    }

    return 0;
}
