// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 01_basic — sanity test that source-mode / iprio / enable / pending round-trip
// through xireg/xireg2/xireg3 still works after the spec rework.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/test.h"
#include <assert.h>

int main(void)
{
    int id = 1;

    test("source_mode_roundtrip") {
        // Iterate over the spec-defined SM values; SM=2 and SM=3 are reserved
        // and have implementation-specific WARL behavior — skip them here.
        int valid[] = { SM_INACTIVE, SM_DETACHED, SM_EDGE1, SM_EDGE0,
                        SM_LEVEL1,   SM_LEVEL0 };
        for (unsigned i = 0; i < sizeof(valid)/sizeof(valid[0]); i++) {
            set_source_mode(DOMAIN_M, id, valid[i]);
            fail_if(get_source_mode(DOMAIN_M, id) != valid[i]);
        }
    }

    test("iprio_roundtrip") {
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        for (int p = 1; p <= 255; p += 17) {
            set_iprio(DOMAIN_M, id, p);
            fail_if(get_iprio(DOMAIN_M, id) != p);
        }
    }

    test("enable_roundtrip") {
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_enabled(DOMAIN_M, id, 1);
        fail_if(!get_enabled(DOMAIN_M, id));
        set_enabled(DOMAIN_M, id, 0);
        fail_if(get_enabled(DOMAIN_M, id));
    }

    test("pending_edge_roundtrip") {
        set_source_mode(DOMAIN_M, id, SM_EDGE1);
        set_pending(DOMAIN_M, id, 1);
        fail_if(!get_pending(DOMAIN_M, id));
        set_pending(DOMAIN_M, id, 0);
        fail_if(get_pending(DOMAIN_M, id));
    }

    return 0;
}
