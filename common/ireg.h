// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef IREG_H
#define IREG_H

#include <stdint.h>
#include "csr.h"
#include "xlen.h"

enum {
    DOMAIN_M,
    DOMAIN_S,
};

void write_iselect(int domain, int index);
xlreg_t read_iregn(int domain, int n);
void write_iregn(int domain, int n, xlreg_t value);
xlreg_t read_iregn_select(int domain, int n, int index);
void write_iregn_select(int domain, int n, xlreg_t value, int index);
void set_or_clear_ireg_bits(int domain, xlreg_t bits, int set);

#endif
