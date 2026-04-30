// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include <assert.h>

int main()
{
    int domain = DOMAIN_M;

    // Unimplemented sources have read-only-zero pending, enabled, iprio, and sourcecfg
    {
        int id = NUM_SOURCES+1;

        set_source_mode(domain, id, SM_EDGE1);
        int sourcecfg = get_sourcecfg(domain, id);
        assert(sourcecfg == 0);

        set_iprio(domain, id, 1);
        int iprio = get_iprio(domain, id);
        assert(iprio == 0);

        set_pending(domain, id, 1);
        int pending = get_pending(domain, id);
        assert(!pending);

        set_enabled(domain, id, 1);
        int enabled = get_enabled(domain, id);
        assert(!enabled);
    }

    // Inactive sources have read-only-zero pending, enabled, and iprio
    {
        int id = 1;

        set_source_mode(domain, id, SM_INACTIVE);

        set_iprio(domain, id, 1);
        int iprio = get_iprio(domain, id);
        assert(iprio == 0);

        set_pending(domain, id, 1);
        int pending = get_pending(domain, id);
        assert(!pending);

        set_enabled(domain, id, 1);
        int enabled = get_enabled(domain, id);
        assert(!enabled);
    }

    // First byte of acliciprio0 is read-only zero
    // First two bytes of aclicsourcecfg0 are read-only zero
    {
        uint64_t acliciprio0 = 1;
        write_iregn_select(DOMAIN_M, 1, acliciprio0, 0x1000);
        acliciprio0 = read_iregn(DOMAIN_M, 1);
        assert((acliciprio0 & 0xff) == 0);

        uint64_t aclicsourcecfg0 = SM_EDGE1;
        write_iregn(DOMAIN_M, 2, aclicsourcecfg0);
        aclicsourcecfg0 = read_iregn(DOMAIN_M, 2);
        assert((aclicsourcecfg0 & 0xffff) == 0);
    }

    // Bit 0 of eip0, eie0 are read-only zero
    {
        uint64_t eip0 = 1, eie0 = 1;
        write_iregn_select(DOMAIN_M, 1, eip0, EIP0);
        write_iregn_select(DOMAIN_M, 1, eie0, EIE0);
    }

    return 0;
}
