// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 08_miconfig — miconfig via mireg4 at xiselect=0x1000.  Spec §Smnip.

#include "common/aclic.h"
#include "common/ireg.h"
#include "common/test.h"

int main(void)
{
    test("miconfig_roundtrip") {
        // mnipen=1, mnipbits=4, mipu=1, snipen=1, snipbits=2, sipu=0
        uint64_t v = (1ull << MICONFIG_MNIPEN_SHIFT)
                   | (4ull << MICONFIG_MNIPBITS_SHIFT)
                   | (1ull << MICONFIG_MIPU_SHIFT)
                   | (1ull << MICONFIG_SNIPEN_SHIFT)
                   | (2ull << MICONFIG_SNIPBITS_SHIFT);
        write_miconfig(v);
        uint64_t got = read_miconfig();
        fail_if(((got >> MICONFIG_MNIPEN_SHIFT) & 1) != 1);
        fail_if(((got >> MICONFIG_MNIPBITS_SHIFT) & 0xF) != 4);
        fail_if(((got >> MICONFIG_MIPU_SHIFT) & 1) != 1);
        fail_if(((got >> MICONFIG_SNIPEN_SHIFT) & 1) != 1);
        fail_if(((got >> MICONFIG_SNIPBITS_SHIFT) & 0xF) != 2);
    }

    test("siconfig_aliases_subset_of_miconfig") {
        // siconfig owns: snipen[3], sipu[5], snipbits[19:16].
        // Writing siconfig=0x60003 sets snipen=0, snipbits=6, sipu=1, snipen=0.
        // Use snipbits=6 to validate (4-bit field).
        write_miconfig(0);
        uint64_t sval = (6ull << MICONFIG_SNIPBITS_SHIFT)
                      | (1ull << MICONFIG_SIPU_SHIFT)
                      | (1ull << MICONFIG_SNIPEN_SHIFT);
        write_siconfig(sval);
        // siconfig read should reflect the subset.
        uint64_t sgot = read_siconfig();
        fail_if(((sgot >> MICONFIG_SNIPBITS_SHIFT) & 0xF) != 6);
        fail_if(((sgot >> MICONFIG_SIPU_SHIFT) & 1) != 1);
        fail_if(((sgot >> MICONFIG_SNIPEN_SHIFT) & 1) != 1);
        // And miconfig must show the same S-mode fields (alias semantics).
        uint64_t mgot = read_miconfig();
        fail_if(((mgot >> MICONFIG_SNIPBITS_SHIFT) & 0xF) != 6);
        fail_if(((mgot >> MICONFIG_SIPU_SHIFT) & 1) != 1);
        fail_if(((mgot >> MICONFIG_SNIPEN_SHIFT) & 1) != 1);
    }

    test("siconfig_cannot_write_mmode_only_bits") {
        // siconfig has read-only-zero for mnipen/mnipbits/mipu.  Writing them
        // through siconfig must NOT affect miconfig's M-mode fields.
        write_miconfig(0);
        uint64_t bogus = (1ull << MICONFIG_MNIPEN_SHIFT)
                      | (8ull << MICONFIG_MNIPBITS_SHIFT)
                      | (1ull << MICONFIG_MIPU_SHIFT);
        write_siconfig(bogus);
        uint64_t mgot = read_miconfig();
        fail_if(((mgot >> MICONFIG_MNIPEN_SHIFT) & 1) != 0);
        fail_if(((mgot >> MICONFIG_MNIPBITS_SHIFT) & 0xF) != 0);
        fail_if(((mgot >> MICONFIG_MIPU_SHIFT) & 1) != 0);
    }

    return 0;
}
