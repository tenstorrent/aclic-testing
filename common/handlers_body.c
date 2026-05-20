// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "handlers.h"
#include "csr.h"
#include "print.h"
#include "htif.h"

#define MCAUSE_SEI          0x8000000000000009ull
#define MCAUSE_ECALL_SMODE  9ull

static void
die(const char * s)
{
    print_str(s);
    exit(1);
}

void
mhandler_body(void)
{
    uint64_t mcause;
    csr_read(mcause, mcause);
    int interrupt = (mcause & (1ull << 63)) != 0;
    if (interrupt) {
        if (num_intr_seen >= num_exp_intr)
            die("Unexpected interrupt\n");
        uint64_t topei;
        if (mcause == MCAUSE_SEI)
            sclaim(topei);
        else
            mclaim(topei);
        uint64_t iid = (topei >> 16) & 0x7ff;
        intr_ids_seen[num_intr_seen++] = iid;
    } else {
        if (mcause == MCAUSE_ECALL_SMODE) {
            csr_write(mepc, ecall_dest);
            csr_set(mstatus, 0x1800);
        } else if (excp_ok) {
            excp_mcause = mcause;
            uint16_t *mepc;
            csr_read(mepc, mepc);
            if ((*mepc & 3) == 3)
                mepc++;
            mepc++;
            csr_write(mepc, mepc);
        } else {
            die("Unexpected exception\n");
        }
    }
}
