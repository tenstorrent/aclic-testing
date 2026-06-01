// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 07_xtopei — mtopei reports highest-prio external (per spec PR #831).
// Coexists with xtopsi (Phase 3 verified xtopsi separately).

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"

static inline uint64_t read_mtopei(void)
{ uint64_t v; csr_read(mtopei, v); return v; }

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");

    test("mtopei_idle_is_zero") {
        uint64_t v = read_mtopei();
        fail_if(v != 0);
    }

    test("mtopei_reports_external_id_and_prio") {
        int id = 4;
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_iprio(DOMAIN_M, id, 9);
        set_enabled(DOMAIN_M, id, 1);
        set_pending(DOMAIN_M, id, 1);
        uint64_t v = read_mtopei();
        // mtopei layout: bits[25:16] = IID, bits[7:0] = IPRIO.
        fail_if(((v >> 16) & 0x3FF) != (uint64_t)id);
        fail_if((v & 0xFF) != 9);
        set_pending(DOMAIN_M, id, 0);
        set_enabled(DOMAIN_M, id, 0);
        set_source_mode(DOMAIN_M, id, SM_INACTIVE);
    }

    return 0;
}
