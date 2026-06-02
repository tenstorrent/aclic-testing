// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 18_stopsi — supervisor-domain stopsi semantics (Ssnip/Ssidctrl), the S-mode
// counterpart of 06_xtopsi.  A source is delegated to the S-domain and
// configured there; stopsi (read from M-mode) reports it.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"

static inline uint64_t read_stopsi(void)         { xlreg_t v; csr_read_num(STOPSI, v); return v; }
static inline void write_sithreshold(uint64_t v) { csr_write_num(SITHRESHOLD, v); }
static inline uint64_t read_sithreshold(void)    { xlreg_t v; csr_read_num(SITHRESHOLD, v); return v; }

static void s_source(int id, int prio)
{
    delegate(id);                          // sourcecfg.D=1 -> source moves to S-domain
    set_source_mode(DOMAIN_S, id, SM_EDGE1);
    set_iprio(DOMAIN_S, id, prio);
    set_enabled(DOMAIN_S, id, 1);
    set_pending(DOMAIN_S, id, 1);
}

static void s_clear(int id)
{
    set_pending(DOMAIN_S, id, 0);
    set_enabled(DOMAIN_S, id, 0);
    undelegate(id);
}

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");   // MIE = 0, observe without delivery
    csr_set(mideleg, 1u << 9);    // delegate S-external to S so stopsi/sip see it

    int id1 = 1, id2 = 5;

    test("idle_returns_threshold") {
        skip_unless(have_ssnip());
        write_sithreshold(0x42);
        xlreg_t v = read_stopsi();
        fail_if((intptr_t)v >> 16 != 0);
        fail_if((v & 0x1FF) != 0x42);
        write_sithreshold(0);
    }

    test("external_positive_siid") {
        skip_unless(have_ssnip());
        write_sithreshold(0);
        s_source(id1, 7);
        xlreg_t v = read_stopsi();
        fail_if((intptr_t)v >> 16 != id1);
        fail_if((v & 0x1FF) != 7);
        s_clear(id1);
    }

    test("highest_priority_wins") {
        skip_unless(have_ssnip());
        write_sithreshold(0);
        s_source(id1, 8);
        s_source(id2, 3);                  // higher priority (lower value)
        xlreg_t v = read_stopsi();
        fail_if((intptr_t)v >> 16 != id2);
        fail_if((v & 0x1FF) != 3);
        s_clear(id1);
        s_clear(id2);
    }

    return 0;
}
