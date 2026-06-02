// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 14_smcsps — mspcs/sspcs round-trip; mcspspush conditionally swaps based on
// mistatus.sppush, latches psppush; mcspspop is the reverse.

#include "common/csr.h"
#include "common/test.h"
#include "common/xlen.h"
#include "common/probe.h"

// mspcs/mistatus are MXLEN-wide; read/write them with an XLEN-wide value so the
// csr access is a single register on both RV32 and RV64.
static inline xlreg_t read_mistatus(void)        { xlreg_t v; csr_read_num(MISTATUS, v); return v; }
static inline void    write_mistatus(xlreg_t v)  { csr_write_num(MISTATUS, v); }
static inline xlreg_t read_mspcs(void)           { xlreg_t v; csr_read_num(MSPCS, v); return v; }
static inline void    write_mspcs(xlreg_t v)     { csr_write_num(MSPCS, v); }

// XLEN-wide test patterns (full register width on either XLEN).
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
    test("mspcs_full_xlen_roundtrip") {
        skip_unless(have_smcsps());
        xlreg_t v = PAT_A;
        write_mspcs(v);
        fail_if(read_mspcs() != v);
    }

    test("mcspspush_no_swap_when_sppush_zero") {
        skip_unless(have_smcsps() && have_smnip() && have_mcspspush());
        // mistatus.sppush = 0 -> no swap; psppush gets latched to 0.
        write_mistatus(0);
        write_mspcs(PAT_1);
        xlreg_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x30910173"); /* mcspspush x2, x2 */
        xlreg_t new_sp, mspcs = read_mspcs();
        asm volatile("mv %0, sp" : "=r"(new_sp));
        fail_if(new_sp != saved_sp);          // sp unchanged
        fail_if(mspcs != PAT_1);              // mspcs unchanged
        xlreg_t mis = read_mistatus();
        fail_if(((mis >> 6) & 1) != 0);       // sppush still 0
        fail_if(((mis >> 7) & 1) != 0);       // psppush latched to 0
    }

    test("mcspspush_swaps_when_sppush_one") {
        skip_unless(have_smcsps() && have_smnip() && have_mcspspush());
        write_mistatus((xlreg_t)1 << 6);      // sppush = 1
        write_mspcs(PAT_2);
        xlreg_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x30910173"); /* mcspspush x2, x2 */
        xlreg_t new_sp;
        asm volatile("mv %0, sp" : "=r"(new_sp));
        // sp <- old mspcs; mspcs <- old sp.
        fail_if(new_sp != PAT_2);
        fail_if(read_mspcs() != saved_sp);
        xlreg_t mis = read_mistatus();
        fail_if(((mis >> 6) & 1) != 0);       // sppush cleared
        fail_if(((mis >> 7) & 1) != 1);       // psppush latched to old sppush=1

        // Restore sp to a valid stack address.
        asm volatile("mv sp, %0" :: "r"(saved_sp));
        // Reset state.
        write_mistatus(0);
        write_mspcs(0);
    }

    return 0;
}
