// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 11_smeihv — placeholder smoke test for xtvec.mode=10 (Smeihv).  A complete
// hardware-vectoring test requires custom mtvec/handler trampolines that
// validate the trap PC; left as follow-up.  This stub just checks that mtvec
// accepts mode=10 when Smeihv is enabled (bit 1 of mtvec writable).

#include "common/csr.h"
#include "common/test.h"
#include <stdint.h>

int main(void)
{
    test("mtvec_mode_10_writable_with_smeihv") {
        uint64_t saved;
        csr_read(mtvec, saved);
        uint64_t base = saved & ~0x3ull;
        csr_write(mtvec, base | 2);       // mode = 10
        uint64_t got;
        csr_read(mtvec, got);
        fail_if((got & 0x3) != 2);
        csr_write(mtvec, saved);          // restore
    }

    return 0;
}
