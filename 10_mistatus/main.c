// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 10_mistatus — mistatus aliases mstatus.{mpie,mpp,VS,FS,mie} bidirectionally
// and owns pithreshprio + sppush/psppush bits.

#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"
#include <stdint.h>

static inline uint64_t read_mistatus(void)    { xlreg_t v; csr_read_num(MISTATUS, v); return v; }
static inline void     write_mistatus(uint64_t v) { csr_write_num(MISTATUS, v); }
static inline uint64_t read_mstatus(void)     { xlreg_t v; csr_read(mstatus, v); return v; }

int main(void)
{
    test("mistatus_mpp_alias_reads_mstatus_mpp") {
        skip_unless(have_smnip());
        // mstatus.MPP at bits[12:11] -> mistatus.mpp at bits[29:28].
        csr_set(mstatus, 3ull << 11);     // MPP = 3 (machine)
        uint64_t mis = read_mistatus();
        fail_if(((mis >> 28) & 0x3) != 3);
    }

    test("mistatus_mpie_alias_reads_mstatus_mpie") {
        skip_unless(have_smnip());
        csr_set(mstatus, 1ull << 7);      // MPIE = 1
        uint64_t mis = read_mistatus();
        fail_if(((mis >> 27) & 0x1) != 1);
    }

    test("mistatus_pithreshprio_owned_writable") {
        skip_unless(have_smnip());
        uint64_t val = (0x55ull << 8);    // pithreshprio = 0x55
        write_mistatus(val);
        uint64_t got = read_mistatus();
        fail_if(((got >> 8) & 0x1FF) != 0x55);
    }

    test("mistatus_mie_alias_writethrough") {
        skip_unless(have_smnip());
        // Writing mistatus.mie (bit 0) should propagate to mstatus.MIE (bit 3).
        // Start with MIE=0 (csrci above), then write mistatus[0]=1.
        csr_clear(mstatus, 1ull << 3);
        uint64_t cur = read_mistatus();
        write_mistatus(cur | 1ull);
        uint64_t mst = read_mstatus();
        fail_if(((mst >> 3) & 1) != 1);
        csr_clear(mstatus, 1ull << 3);    // restore
    }

    return 0;
}
