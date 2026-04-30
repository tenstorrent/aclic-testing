// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"
#include <stdint.h>

// TODO: expect_interrupt should have major and minor interrupt numbers and iprio
#define expect_interrupt(N) defer(push_intr(N), pop_intr(N))
#define expect_exception(N) defer(set_excp_ok(), check_mcause(N))

void push_intr(uint64_t id);
void pop_intr();

void set_excp_ok();
void check_mcause(uint64_t mcause);

#endif
