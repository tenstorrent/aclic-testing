// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 04_illegal — illegal accesses raise illegal-instruction exceptions:
//   * mireg5 / mireg6 in 0x1000-0x10FF range (only mireg/2/3/4 are defined)
//
// Note: odd-xiselect-on-RV64 and mstateen0.ACLIC gating require additional
// trap-handler plumbing not exercised in this initial test.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/csr.h"
#include "common/interrupt.h"
#include "common/test.h"
#include "common/probe.h"

#define EX_ILLEGAL_INSTRUCTION 2

int main(void)
{
    test("mireg5_at_0x1000_illegal") {
        skip_unless(have_aclic_m());
        write_iselect(DOMAIN_M, 0x1000);
        expect_exception(EX_ILLEGAL_INSTRUCTION) {
            uint64_t v;
            csr_read(MIREG5, v);
            (void)v;
        }
    }

    test("mireg6_at_0x1000_illegal") {
        skip_unless(have_aclic_m());
        write_iselect(DOMAIN_M, 0x1000);
        expect_exception(EX_ILLEGAL_INSTRUCTION) {
            uint64_t v;
            csr_read(MIREG6, v);
            (void)v;
        }
    }

    return 0;
}
