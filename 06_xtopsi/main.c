// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 06_xtopsi — new Smidctrl mtopsi/stopsi semantics:
//   * SIID encoding (positive for external, two's-complement-negative for major)
//   * Idle reads return SIID=0, IPRIO=xithreshold.iprio
//   * CSRRW xtopsi raises threshold + clears pending of reported interrupt
//
// All checks are done from M-mode without enabling external interrupt delivery
// to the hart (no MIE.MEIE / mtvec setup beyond the default).  Disable interrupt
// delivery to the hart so the trap doesn't fire while we observe xtopsi.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"

static inline uint64_t read_mtopsi(void) {
    xlreg_t v; csr_read_num(MTOPSI, v); return v;
}

static inline void write_mithreshold(uint64_t v) { csr_write_num(MITHRESHOLD, v); }
static inline uint64_t read_mithreshold(void)    { xlreg_t v; csr_read_num(MITHRESHOLD, v); return v; }

int main(void)
{
    // Disable hart-level interrupt delivery so the trap handler doesn't run
    // while we set things up.
    asm volatile("csrci mstatus, 8" ::: "memory");  // MIE = 0

    int id1 = 1, id2 = 5;

    test("idle_returns_threshold") {
        skip_unless(have_smnip());
        // No pending+enabled source — mtopsi returns SIID=0, IPRIO=mithreshold.
        // Use a recognizable threshold value 0x42 (66 dec).
        write_mithreshold(0x42);
        xlreg_t v = read_mtopsi();
        fail_if((intptr_t)v >> 16 != 0);
        fail_if((v & 0x1FF) != 0x42);
    }

    test("external_positive_siid") {
        skip_unless(have_smnip());
        write_mithreshold(0);     // disable threshold gating
        set_source_mode(DOMAIN_M, id1, SM_EDGE1);
        set_iprio(DOMAIN_M, id1, 7);
        set_enabled(DOMAIN_M, id1, 1);
        set_pending(DOMAIN_M, id1, 1);

        xlreg_t v = read_mtopsi();
        // SIID = +id1 (positive), IPRIO = 7
        fail_if((intptr_t)v >> 16 != id1);
        fail_if((v & 0x1FF) != 7);

        set_pending(DOMAIN_M, id1, 0);
        set_enabled(DOMAIN_M, id1, 0);
        set_source_mode(DOMAIN_M, id1, SM_INACTIVE);
    }

    test("highest_priority_wins") {
        skip_unless(have_smnip());
        write_mithreshold(0);
        set_source_mode(DOMAIN_M, id1, SM_EDGE1);
        set_iprio(DOMAIN_M, id1, 8);
        set_enabled(DOMAIN_M, id1, 1);
        set_pending(DOMAIN_M, id1, 1);

        set_source_mode(DOMAIN_M, id2, SM_EDGE1);
        set_iprio(DOMAIN_M, id2, 3);   // higher priority (lower value)
        set_enabled(DOMAIN_M, id2, 1);
        set_pending(DOMAIN_M, id2, 1);

        xlreg_t v = read_mtopsi();
        fail_if((intptr_t)v >> 16 != id2);
        fail_if((v & 0x1FF) != 3);

        set_pending(DOMAIN_M, id1, 0);
        set_pending(DOMAIN_M, id2, 0);
        set_enabled(DOMAIN_M, id1, 0);
        set_enabled(DOMAIN_M, id2, 0);
        set_source_mode(DOMAIN_M, id1, SM_INACTIVE);
        set_source_mode(DOMAIN_M, id2, SM_INACTIVE);
    }

    test("claim_raises_threshold_and_clears_pending") {
        skip_unless(have_smnip());
        // Set up a pending external interrupt at IPRIO=3.
        write_mithreshold(0);
        set_source_mode(DOMAIN_M, id1, SM_EDGE1);
        set_iprio(DOMAIN_M, id1, 3);
        set_enabled(DOMAIN_M, id1, 1);
        set_pending(DOMAIN_M, id1, 1);

        // CSRRW: write 0 (no extra raise) and capture pre-side-effect snapshot.
        xlreg_t pre = 0;
        mclaim_topsi(pre, 0);
        // Pre-snapshot reports the interrupt we just claimed.
        fail_if((intptr_t)pre >> 16 != id1);
        fail_if((pre & 0x1FF) != 3);
        // After claim: pending cleared (edge source) and threshold raised to IPRIO=3.
        fail_if(get_pending(DOMAIN_M, id1));
        fail_if(read_mithreshold() != 3);

        // Clean up.
        write_mithreshold(0);
        set_enabled(DOMAIN_M, id1, 0);
        set_source_mode(DOMAIN_M, id1, SM_INACTIVE);
    }

    return 0;
}
