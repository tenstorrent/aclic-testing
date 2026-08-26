// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 13_smehv — placeholder.  Smehv synchronous-exception vectoring requires
// a synchronous trap with mtvec.mode=11 + xijt.EHV != 0; verifying the
// resulting PC computation needs a custom handler.  Left as follow-up.

#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"
#include <stdint.h>

int main(void)
{
#if 0
    // We nee Smehv (not smijt) but we don't know how to test for Smehv
    test("xijt_ehv_can_be_programmed_to_nonzero") {
        skip_unless(have_smijt());
        csr_write_num(MIJT, 0x80004000ull | (1ull << 2));
        xlreg_t got;
        csr_read_num(MIJT, got);
        fail_if(((got >> 2) & 0x3) == 0);
    }
#endif

    return 0;
}
