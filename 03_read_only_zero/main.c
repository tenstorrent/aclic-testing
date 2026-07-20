// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 03_read_only_zero — target[0] / sourcecfg[0] are read-only zero.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/test.h"
#include "common/probe.h"

int main(void)
{
    test("target0_iprio_is_roz") {
        skip_unless(have_aclic_m());
        // acliciprio[0] byte for source 0 is read-only zero.
        // Try writing all-ones to acliciprio[0] via mireg at xiselect=0x1000.
        write_iselect(DOMAIN_M, 0x1000);
        write_iregn(DOMAIN_M, 1, ~0ull);
        uint64_t v = read_iregn(DOMAIN_M, 1);
        // Source 0 occupies bits[7:0]; must be zero.
        fail_if((v & 0xff) != 0);
    }

    test("sourcecfg0_is_roz") {
        skip_unless(have_aclic_m());
        // aclicsourcecfg[0][15:0] is read-only zero.
        write_iselect(DOMAIN_M, 0x1000);
        write_iregn(DOMAIN_M, 2, ~0ull);
        uint64_t v = read_iregn(DOMAIN_M, 2);
        fail_if((v & 0xFFFF) != 0);
    }

    test("source0_pending_and_enabled_are_roz") {
        skip_unless(have_aclic_m());
        // eip0 bit 0 and eie0 bit 0 are read-only zero.
        write_iselect(DOMAIN_M, EIP0);
        csr_set(mireg, 0x1);     // try to set source 0 pending
        uint64_t v = read_iregn(DOMAIN_M, 1);
        fail_if((v & 1) != 0);

        write_iselect(DOMAIN_M, EIE0);
        csr_set(mireg, 0x1);
        v = read_iregn(DOMAIN_M, 1);
        fail_if((v & 1) != 0);
    }

    return 0;
}
