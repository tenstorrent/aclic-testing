// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 21_sspcs — sspcs round-trip; scspspush conditionally swaps based on
// sistatus.sppush, latches psppush (Sscsps), the S-mode counterpart of
// 14_smcsps.  scspspush requires priv >= S, which M satisfies.

#include "common/csr.h"
#include "common/test.h"
#include "common/xlen.h"
#include "common/probe.h"

static inline xlreg_t read_sistatus(void)        { xlreg_t v; csr_read_num(SISTATUS, v); return v; }
static inline void    write_sistatus(xlreg_t v)  { csr_write_num(SISTATUS, v); }
static inline xlreg_t read_sspcs(void)           { xlreg_t v; csr_read_num(SSPCS, v); return v; }
static inline void    write_sspcs(xlreg_t v)     { csr_write_num(SSPCS, v); }

#if __riscv_xlen == 64
#define PAT_A 0xdeadbeefcafebabeull
#define PAT_1 0x1111111111111111ull
#define PAT_2 0x2222222222222222ull
#else
#define PAT_A 0xdeadbeefUL
#define PAT_1 0x11111111UL
#define PAT_2 0x22222222UL
#endif

int main(void)
{
    test("sspcs_full_xlen_roundtrip") {
        skip_unless(have_sscsps());
        xlreg_t v = PAT_A;
        write_sspcs(v);
        fail_if(read_sspcs() != v);
    }

    test("scspspush_no_swap_when_sppush_zero") {
        skip_unless(have_sscsps() && have_ssnip() && have_scspspush());
        write_sistatus(0);
        write_sspcs(PAT_1);
        xlreg_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x10910173"); /* scspspush x2, x2 */
        xlreg_t new_sp, sspcs = read_sspcs();
        asm volatile("mv %0, sp" : "=r"(new_sp));
        fail_if(new_sp != saved_sp);          // sp unchanged
        fail_if(sspcs != PAT_1);              // sspcs unchanged
        xlreg_t sis = read_sistatus();
        fail_if(((sis >> 6) & 1) != 0);       // sppush still 0
        fail_if(((sis >> 7) & 1) != 0);       // psppush latched to 0
    }

    test("scspspush_swaps_when_sppush_one") {
        skip_unless(have_sscsps() && have_ssnip() && have_scspspush());
        write_sistatus((xlreg_t)1 << 6);      // sppush = 1
        write_sspcs(PAT_2);
        xlreg_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x10910173"); /* scspspush x2, x2 */
        xlreg_t new_sp;
        asm volatile("mv %0, sp" : "=r"(new_sp));
        // sp <- old sspcs; sspcs <- old sp.
        fail_if(new_sp != PAT_2);
        fail_if(read_sspcs() != saved_sp);
        xlreg_t sis = read_sistatus();
        fail_if(((sis >> 6) & 1) != 0);       // sppush cleared
        fail_if(((sis >> 7) & 1) != 1);       // psppush latched to old sppush=1

        asm volatile("mv sp, %0" :: "r"(saved_sp));   // restore sp
        write_sistatus(0);
        write_sspcs(0);
    }

    return 0;
}
