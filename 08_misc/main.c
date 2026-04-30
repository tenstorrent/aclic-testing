// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include "common/htif.h"
#include <assert.h>

int main()
{
    // M- and S-level domains have distinct sourcecfg, iprio, eie, and eip
    {
        int id = 2;
        delegate(id);

        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        int sourcecfg = get_sourcecfg(DOMAIN_M, id);
        assert(sourcecfg == 0x400);

        set_pending(DOMAIN_S, id, 1);
        int pending = get_pending(DOMAIN_M, id);
        assert(!pending);
        set_pending(DOMAIN_S, id, 0);

        set_enabled(DOMAIN_S, id, 1);
        int enabled = get_enabled(DOMAIN_M, id);
        assert(!enabled);

        set_iprio(DOMAIN_S, id, 2);
        int iprio = get_iprio(DOMAIN_M, id);
        assert(iprio == 0);
    }

#if 0
    {
        uint64_t mip = 0;
        int id = 2;
        delegate(id);
        csr_write(mideleg, 1ull << 9); // delegate SEI to S-mode
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_iprio(DOMAIN_S, id, 1);
        set_enabled(DOMAIN_S, id, 1);
        //set_pending(DOMAIN_M, id, 1);
        //assert(get_pending(DOMAIN_M, id) == 0);
        set_source_state(id, 1);
        //set_pending(DOMAIN_S, id, 1);
        csr_read(mip, mip);
        assert(((mip >> 9) & 1) == 1);
    }
#endif

    // Pending bit is read-only for level-sensitive sources
    {
        int id = 1;
        set_source_mode(DOMAIN_M, id, SM_LEVEL1);
        set_pending(DOMAIN_M, id, 1);
        int pending = get_pending(DOMAIN_M, id);
        assert(!pending);
    }

    // Transition between different source modes for different source states
    {
        // TODO
    }

    exit(0);
}
