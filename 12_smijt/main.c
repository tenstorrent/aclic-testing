// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 12_smijt — placeholder for Smijt jump-table vectoring.  Verifies that
// xijt accepts BASE (64-byte aligned), SHAMT, and EHV fields per spec
// §Smijt/§Smehv.  Full execution test (mode=11 trap fetching handler from
// the table) is follow-up work.

#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"
#include <stdint.h>

int main(void)
{
    test("mijt_base_64byte_aligned") {
        skip_unless(have_smijt());
        // Write a 64-byte-aligned address; low 6 bits must read back zero
        // (WARL on BASE).
        xlreg_t addr = 0x80004000ull;      // aligned to 64
        csr_write_num(MIJT, addr | 0x3F);  // try to set low bits
        xlreg_t got;
        csr_read_num(MIJT, got);
        // Low 6 bits: SHAMT[1:0] and possibly EHV[2] are writable; bits[5:3] reserved.
        fail_if((got & ~0x07) != (addr & ~0x3F));
    }

    test("mijt_shamt_field_writable") {
        skip_unless(have_smijt());
        xlreg_t base = 0x80004000ull;
        csr_write_num(MIJT, base | 3);     // SHAMT = 3
        xlreg_t got;
        csr_read_num(MIJT, got);
        fail_if((got & 0x3) != 3);
    }

#if 0
    // We nee Smehv (not smijt) but we don't know how to test for Smehv
    test("mijt_ehv_field_writable") {
        skip_unless(have_smijt());
        xlreg_t base = 0x80004000ull;
        csr_write_num(MIJT, base | (1ull << 2));  // EHV[2] = 1
        xlreg_t got;
        csr_read_num(MIJT, got);
        fail_if(((got >> 2) & 0x3) != 1);
    }
#endif

    return 0;
}
