// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/htif.h"
#include "common/mode.h"
#include <assert.h>

void
smode()
{
    uint64_t dummy;
    expect_exception(2) { set_source_mode(DOMAIN_S, 1, SM_EDGE1); }
    expect_exception(2) { set_iprio(DOMAIN_S, 1, 1); }
    expect_exception(2) { csr_read(SSPCS, dummy); }
    expect_exception(2) { csr_read(SPISTATUS, dummy); }
    expect_exception(2) { csr_read(SITHRESHOLD, dummy); }
    expect_exception(2) { csr_read(SIVT, dummy); }
    expect_exception(2) { csr_read(SEIVT, dummy); }
    // siselect is still accessible if mstateen0.CSRIND == 1
    write_iselect(DOMAIN_S, 0x1000);
    exit(0);
}

int main()
{
    // Odd-numbered xiselect between 0x1000 and 0x1100 do not exist for XLEN=64
    {
        write_iselect(DOMAIN_M, 0x1001);
        expect_exception(2) {
            read_iregn(DOMAIN_M, 1);
        }
        write_iselect(DOMAIN_M, 0xc1);
        expect_exception(2) {
            read_iregn(DOMAIN_M, 1);
        }
        write_iselect(DOMAIN_M, 0x81);
        expect_exception(2) {
            read_iregn(DOMAIN_M, 1);
        }
    }

    // xireg4-6 cause illegal instruction exception
    {
        write_iselect(DOMAIN_M, 0x1000);
        for (int i = 4; i < 7; i++)
            expect_exception(2) { read_iregn(DOMAIN_M, i); }
    }

    // When mstateen0.ACLIC (bit 53) is 0, accessing acliciprio and aclicsourcecfg
    // in S-mode or lower raises illegal instruction exception
    {
        csr_clear(MSTATEEN0, 1ull << 53);
        go_to_smode(smode); // never returns
    }

    return 0;
}
