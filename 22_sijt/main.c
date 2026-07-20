// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 22_sijt — sijt accepts BASE (64-byte aligned), SHAMT, and EHV fields
// (Ssijt), the S-mode counterpart of 12_smijt.

#include "common/csr.h"
#include "common/test.h"
#include "common/probe.h"
#include <stdint.h>

int main(void)
{
    test("sijt_base_64byte_aligned") {
        skip_unless(have_ssijt());
        xlreg_t addr = 0x80004000ull;      // aligned to 64
        csr_write_num(SIJT, addr | 0x3F);  // try to set low bits
        xlreg_t got;
        csr_read_num(SIJT, got);
        // Low 6 bits: SHAMT[1:0] and EHV[3:2] are writable; bits[5:4] reserved.
        fail_if((got & ~0x0F) != (addr & ~0x3F));
    }

    test("sijt_shamt_field_writable") {
        skip_unless(have_ssijt());
        xlreg_t base = 0x80004000ull;
        csr_write_num(SIJT, base | 3);     // SHAMT = 3
        xlreg_t got;
        csr_read_num(SIJT, got);
        fail_if((got & 0x3) != 3);
    }

    test("sijt_ehv_field_writable") {
        skip_unless(have_ssijt());
        xlreg_t base = 0x80004000ull;
        csr_write_num(SIJT, base | (1ull << 2));  // EHV[3:2] = 01
        xlreg_t got;
        csr_read_num(SIJT, got);
        fail_if(((got >> 2) & 0x3) != 1);
    }

    return 0;
}
