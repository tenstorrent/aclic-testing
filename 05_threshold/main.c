// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 05_threshold — 9-bit mithreshold/sithreshold round-trip, threshold gating
// of lower-priority interrupts in xtopsi.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"

static inline void write_mithreshold(uint64_t v) { csr_write_num(MITHRESHOLD, v); }
static inline uint64_t read_mithreshold(void)    { uint64_t v; csr_read_num(MITHRESHOLD, v); return v; }
static inline uint64_t read_mtopsi(void)         { uint64_t v; csr_read_num(MTOPSI, v); return v; }

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");  // mask interrupts at hart

    test("threshold_9bit_roundtrip") {
        // Threshold field is 9 bits (IPRIOLEN+1).  Try values within range.
        for (uint64_t t = 0; t <= 0x1FF; t += 0x40) {
            write_mithreshold(t);
            fail_if((read_mithreshold() & 0x1FF) != t);
        }
    }

    test("threshold_above_2_to_IPRIOLEN_disables_gating") {
        // Setting threshold >= 2^IPRIOLEN (256) means "no suppression".
        write_mithreshold(0x100);
        int id = 2;
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_iprio(DOMAIN_M, id, 255);  // lowest priority
        set_enabled(DOMAIN_M, id, 1);
        set_pending(DOMAIN_M, id, 1);
        uint64_t v = read_mtopsi();
        // Source should still be visible despite IPRIO=255 (highest numeric).
        fail_if((int64_t)v >> 16 != id);
        set_pending(DOMAIN_M, id, 0);
        set_enabled(DOMAIN_M, id, 0);
        set_source_mode(DOMAIN_M, id, SM_INACTIVE);
    }

    test("threshold_suppresses_lower_priority_source") {
        write_mithreshold(5);   // gate everything with iprio >= 5
        int id = 2;
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_iprio(DOMAIN_M, id, 7);
        set_enabled(DOMAIN_M, id, 1);
        set_pending(DOMAIN_M, id, 1);
        uint64_t v = read_mtopsi();
        // Source suppressed -> xtopsi reports SIID=0, IPRIO=threshold(=5).
        fail_if((int64_t)v >> 16 != 0);
        fail_if((v & 0x1FF) != 5);
        set_pending(DOMAIN_M, id, 0);
        set_enabled(DOMAIN_M, id, 0);
        set_source_mode(DOMAIN_M, id, SM_INACTIVE);
        write_mithreshold(0);
    }

    return 0;
}
