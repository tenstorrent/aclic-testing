// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 02_delegation — sourcecfg.D bit, S-domain visibility after delegation,
// M↔S transitions clear stale state.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/test.h"

int main(void)
{
    int id = 3;

    test("delegate_moves_source_to_s_domain") {
        // Configure source in M with non-zero state.
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_iprio(DOMAIN_M, id, 5);
        set_enabled(DOMAIN_M, id, 1);
        fail_if(get_source_mode(DOMAIN_M, id) != SM_EDGE1);
        // Delegate: D bit set in M-domain sourcecfg.
        delegate(id);
        // Verify D bit visible in M-domain sourcecfg packed read.
        int cfg = get_sourcecfg(DOMAIN_M, id);
        fail_if(((cfg >> 10) & 1) != 1);
        // S-domain SM should now report whatever sourcecfg's SM is in S-domain.
        // M-domain's view of pending/enabled is cleared on delegate transition.
        fail_if(get_enabled(DOMAIN_M, id) != 0);
        fail_if(get_pending(DOMAIN_M, id) != 0);
    }

    test("undelegate_clears_s_domain_state") {
        // Configure delegated source in S, then undelegate.
        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_iprio(DOMAIN_S, id, 7);
        set_enabled(DOMAIN_S, id, 1);
        // Undelegate (set M sourcecfg = 0).
        undelegate(id);
        // S-domain state for this source should be cleared.
        fail_if(get_enabled(DOMAIN_S, id) != 0);
        fail_if(get_pending(DOMAIN_S, id) != 0);
        fail_if(get_iprio(DOMAIN_S, id) != 0);
    }

    return 0;
}
