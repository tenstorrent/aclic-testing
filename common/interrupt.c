// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "interrupt.h"
#include "handlers.h"
#include <assert.h>

void push_intr(uint64_t id)
{
    exp_intr_ids[num_exp_intr++] = id;
}

void pop_intr()
{
    assert(num_intr_seen > 0);
    num_intr_seen--;
    num_exp_intr--;
    assert(exp_intr_ids[num_exp_intr] == intr_ids_seen[num_intr_seen]);
}

void set_excp_ok()
{
    excp_ok = 1;
    excp_mcause = 0;
}

void check_mcause(uint64_t mcause)
{
    assert(excp_mcause == mcause);
    excp_ok = 0;
}
