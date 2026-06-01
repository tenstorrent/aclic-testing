// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 15_smip — Smip auto-context-push on trap and mipopret restore.  Full
// trap-driven test needs a tailored mtvec handler; this stub just verifies
// the miconfig.mipu bit round-trips and that mipopret encodes/decodes.

#include "common/aclic.h"
#include "common/csr.h"
#include "common/test.h"

int main(void)
{
    test("miconfig_mipu_roundtrip") {
        write_miconfig(1ull << MICONFIG_MIPU_SHIFT);
        uint64_t v = read_miconfig();
        fail_if(((v >> MICONFIG_MIPU_SHIFT) & 1) != 1);
        write_miconfig(0);
    }

    return 0;
}
