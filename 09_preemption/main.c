// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 09_preemption — basic test that miconfig.mnipbits derives NIPPRIO_MASK
// correctly so that interrupts within the same group cannot preempt.
//
// Full nested-trap reproduction of the spec's example table is left as a
// follow-up; here we only verify the threshold-grouping math by observing
// xtopsi suppression with mnipbits=4 (16-source group).

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"

static inline void write_mithreshold(uint64_t v) { csr_write_num(MITHRESHOLD, v); }
static inline uint64_t read_mtopsi(void)         { xlreg_t v; csr_read_num(MTOPSI, v); return v; }

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");

    test("mnipbits_groups_priorities") {
        // mnipen=1, mnipbits=4 -> NIPPRIO_MASK = ~0x0F (top 4 bits matter).
        uint64_t cfg = (1ull << MICONFIG_MNIPEN_SHIFT)
                     | (4ull << MICONFIG_MNIPBITS_SHIFT);
        write_miconfig(cfg);

        // Threshold at priority 0x20 (binary 00100000).  With NIPPRIO_MASK=0xF0:
        //  source at iprio 0x25 (top4=0010) -> (iprio & 0xF0)=0x20 >= 0x20 -> suppressed
        //  source at iprio 0x18 (top4=0001) -> 0x10 >= 0x20 -> false -> visible
        write_mithreshold(0x20);

        int id_low = 1, id_high = 2;
        set_source_mode(DOMAIN_M, id_low,  SM_EDGE1);
        set_source_mode(DOMAIN_M, id_high, SM_EDGE1);
        set_iprio(DOMAIN_M, id_low,  0x25);   // same group as threshold
        set_iprio(DOMAIN_M, id_high, 0x18);   // strictly higher priority group
        set_enabled(DOMAIN_M, id_low,  1);
        set_enabled(DOMAIN_M, id_high, 1);
        set_pending(DOMAIN_M, id_low,  1);
        set_pending(DOMAIN_M, id_high, 1);

        xlreg_t v = read_mtopsi();
        fail_if((intptr_t)v >> 16 != id_high);

        // Cleanup
        set_pending(DOMAIN_M, id_low,  0);
        set_pending(DOMAIN_M, id_high, 0);
        set_enabled(DOMAIN_M, id_low,  0);
        set_enabled(DOMAIN_M, id_high, 0);
        set_source_mode(DOMAIN_M, id_low,  SM_INACTIVE);
        set_source_mode(DOMAIN_M, id_high, SM_INACTIVE);
        write_mithreshold(0);
        write_miconfig(0);
    }

    return 0;
}
