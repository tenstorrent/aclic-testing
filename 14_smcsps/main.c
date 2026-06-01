// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0
//
// 14_smcsps — mspcs/sspcs round-trip; mcspspush conditionally swaps based on
// mistatus.sppush, latches psppush; mcspspop is the reverse.

#include "common/csr.h"
#include "common/test.h"
#include <stdint.h>

static inline uint64_t read_mistatus(void)        { uint64_t v; csr_read_num(MISTATUS, v); return v; }
static inline void     write_mistatus(uint64_t v) { csr_write_num(MISTATUS, v); }
static inline uint64_t read_mspcs(void)           { uint64_t v; csr_read_num(MSPCS, v); return v; }
static inline void     write_mspcs(uint64_t v)    { csr_write_num(MSPCS, v); }

int main(void)
{
    test("mspcs_full_xlen_roundtrip") {
        uint64_t v = 0xdeadbeefcafebabeull;
        write_mspcs(v);
        fail_if(read_mspcs() != v);
    }

    test("mcspspush_no_swap_when_sppush_zero") {
        // mistatus.sppush = 0 -> no swap; psppush gets latched to 0.
        write_mistatus(0);
        write_mspcs(0x1111111111111111ull);
        uint64_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x30910173"); /* mcspspush x2, x2 */
        uint64_t new_sp, mspcs = read_mspcs();
        asm volatile("mv %0, sp" : "=r"(new_sp));
        fail_if(new_sp != saved_sp);          // sp unchanged
        fail_if(mspcs != 0x1111111111111111ull); // mspcs unchanged
        uint64_t mis = read_mistatus();
        fail_if(((mis >> 6) & 1) != 0);       // sppush still 0
        fail_if(((mis >> 7) & 1) != 0);       // psppush latched to 0
    }

    test("mcspspush_swaps_when_sppush_one") {
        write_mistatus(1ull << 6);            // sppush = 1
        write_mspcs(0x2222222222222222ull);
        uint64_t saved_sp;
        asm volatile("mv %0, sp" : "=r"(saved_sp));
        asm volatile(".4byte 0x30910173"); /* mcspspush x2, x2 */
        uint64_t new_sp;
        asm volatile("mv %0, sp" : "=r"(new_sp));
        // sp <- old mspcs; mspcs <- old sp.
        fail_if(new_sp != 0x2222222222222222ull);
        fail_if(read_mspcs() != saved_sp);
        uint64_t mis = read_mistatus();
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
