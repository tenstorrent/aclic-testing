// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 17_sithreshold — 9-bit sithreshold round-trip and threshold gating of
// lower-priority S-domain sources in stopsi (Ssnip), the S-mode counterpart
// of 05_threshold.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"

static inline void write_sithreshold(uint64_t v) { csr_write_num(SITHRESHOLD, v); }
static inline uint64_t read_sithreshold(void)    { xlreg_t v; csr_read_num(SITHRESHOLD, v); return v; }
static inline uint64_t read_stopsi(void)         { xlreg_t v; csr_read_num(STOPSI, v); return v; }

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");
    csr_set(mideleg, 1u << 9);    // delegate S-external to S so stopsi sees it

    test("sithreshold_9bit_roundtrip") {
        skip_unless(have_ssnip());
        for (uint64_t t = 0; t <= 0x1FF; t += 0x40) {
            write_sithreshold(t);
            fail_if((read_sithreshold() & 0x1FF) != t);
        }
    }

    test("sithreshold_above_2_to_IPRIOLEN_disables_gating") {
        skip_unless(have_ssnip());
        write_sithreshold(0x100);     // >= 2^IPRIOLEN -> no suppression
        int id = 2;
        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_iprio(DOMAIN_S, id, 255);
        set_enabled(DOMAIN_S, id, 1);
        set_pending(DOMAIN_S, id, 1);
        xlreg_t v = read_stopsi();
        fail_if((intptr_t)v >> 16 != id);
        set_pending(DOMAIN_S, id, 0);
        set_enabled(DOMAIN_S, id, 0);
        undelegate(id);
        write_sithreshold(0);
    }

    test("sithreshold_suppresses_lower_priority_source") {
        skip_unless(have_ssnip());
        write_sithreshold(5);         // gate everything with iprio >= 5
        int id = 2;
        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_iprio(DOMAIN_S, id, 7);
        set_enabled(DOMAIN_S, id, 1);
        set_pending(DOMAIN_S, id, 1);
        xlreg_t v = read_stopsi();
        fail_if((intptr_t)v >> 16 != 0);   // suppressed
        fail_if((v & 0x1FF) != 5);         // reports threshold
        set_pending(DOMAIN_S, id, 0);
        set_enabled(DOMAIN_S, id, 0);
        undelegate(id);
        write_sithreshold(0);
    }

    return 0;
}
