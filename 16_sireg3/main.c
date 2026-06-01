// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 16_sireg3 — sireg3 (supervisor sourcecfg "packed3") must be accessible.
//
// Sources whose (id % 8) >= 4 keep their packed sourcecfg in xireg3 rather
// than xireg2, so configuring such a source in the S-domain reads/writes
// sireg3.  This is a regression test for SIREG3 being dropped from the
// Sscsrind enable list in CsRegs::enableAia(): when sireg3 is left
// unimplemented the csr access raises an illegal instruction and the trap
// handler aborts the run.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/test.h"

int main(void)
{
    // First source whose packed sourcecfg lives in xireg3 (the high half) at
    // selector 0x1000.  SCFG_FIELDS is 4 on RV64 (id 4) and 2 on RV32 (id 2),
    // so this routes through sireg3 on either XLEN.
    int id = SCFG_FIELDS;

    test("sireg3_sourcecfg_roundtrip") {
        // Delegate the source to the S-domain, then drive its sourcecfg there.
        // set_source_mode/get_source_mode for this id go through sireg3.
        delegate(id);

        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        fail_if(get_source_mode(DOMAIN_S, id) != SM_EDGE1);

        set_source_mode(DOMAIN_S, id, SM_LEVEL1);
        fail_if(get_source_mode(DOMAIN_S, id) != SM_LEVEL1);

        // Cleanup.
        undelegate(id);
    }

    return 0;
}
