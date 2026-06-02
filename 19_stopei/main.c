// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 19_stopei — stopei reports the highest-priority pending+enabled S-domain
// external source (Ssaia), the S-mode counterpart of 07_xtopei.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"

static inline uint64_t read_stopei(void) { xlreg_t v; csr_read_num(STOPEI, v); return v; }

int main(void)
{
    asm volatile("csrci mstatus, 8" ::: "memory");
    csr_set(mideleg, 1u << 9);    // delegate S-external to S

    test("stopei_idle_is_zero") {
        skip_unless(have_stopei());
        fail_if(read_stopei() != 0);
    }

    test("stopei_reports_external_id_and_prio") {
        skip_unless(have_stopei());
        int id = 4;
        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_iprio(DOMAIN_S, id, 9);
        set_enabled(DOMAIN_S, id, 1);
        set_pending(DOMAIN_S, id, 1);
        uint64_t v = read_stopei();
        // stopei layout: bits[25:16] = IID, bits[7:0] = IPRIO.
        fail_if(((v >> 16) & 0x3FF) != (uint64_t)id);
        fail_if((v & 0xFF) != 9);
        set_pending(DOMAIN_S, id, 0);
        set_enabled(DOMAIN_S, id, 0);
        undelegate(id);
    }

    return 0;
}
