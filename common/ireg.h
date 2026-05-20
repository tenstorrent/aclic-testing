// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef IREG_H
#define IREG_H

#include <stdint.h>
#include "csr.h"

enum {
    DOMAIN_M,
    DOMAIN_S,
};

void write_iselect(int domain, int index);
uint64_t read_iregn(int domain, int n);
void write_iregn(int domain, int n, uint64_t value);
uint64_t read_iregn_select(int domain, int n, int index);
void write_iregn_select(int domain, int n, uint64_t value, int index);
void set_or_clear_ireg_bits(int domain, uint64_t bits, int set);

#endif
