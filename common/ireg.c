// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "ireg.h"
#include "csr.h"

void
write_iselect(int domain, int index)
{
    if (domain == DOMAIN_M)
        csr_write(miselect, index);
    else
        csr_write(siselect, index);
}

uint64_t
read_iregn(int domain, int n)
{
    uint64_t value = 0;
    if (domain == DOMAIN_M) {
        switch (n) {
            case 1: csr_read(MIREG,  value); break;
            case 2: csr_read(MIREG2, value); break;
            case 3: csr_read(MIREG3, value); break;
            case 4: csr_read(MIREG4, value); break;
            case 5: csr_read(MIREG5, value); break;
            case 6: csr_read(MIREG6, value); break;
        }
    } else {
        switch (n) {
            case 1: csr_read(SIREG,  value); break;
            case 2: csr_read(SIREG2, value); break;
            case 3: csr_read(SIREG3, value); break;
            case 4: csr_read(SIREG4, value); break;
            case 5: csr_read(SIREG5, value); break;
            case 6: csr_read(SIREG6, value); break;
        }
    }
    return value;
}

void
write_iregn(int domain, int n, uint64_t value)
{
    if (domain == DOMAIN_M) {
        switch (n) {
            case 1: csr_write(MIREG,  value); break;
            case 2: csr_write(MIREG2, value); break;
            case 3: csr_write(MIREG3, value); break;
            case 4: csr_write(MIREG4, value); break;
            case 5: csr_write(MIREG5, value); break;
            case 6: csr_write(MIREG6, value); break;
        }
    } else {
        switch (n) {
            case 1: csr_write(SIREG,  value); break;
            case 2: csr_write(SIREG2, value); break;
            case 3: csr_write(SIREG3, value); break;
            case 4: csr_write(SIREG4, value); break;
            case 5: csr_write(SIREG5, value); break;
            case 6: csr_write(SIREG6, value); break;
        }
    }
}

uint64_t
read_iregn_select(int domain, int n, int index)
{
    write_iselect(domain, index);
    return read_iregn(domain, n);
}

void
write_iregn_select(int domain, int n, uint64_t value, int index)
{
    write_iselect(domain, index);
    write_iregn(domain, n, value);
}

void
set_or_clear_ireg_bits(int domain, uint64_t bits, int set)
{
    if (domain == DOMAIN_M) {
        if (set)
            csr_set(mireg, bits);
        else
            csr_clear(mireg, bits);
    } else {
        if (set)
            csr_set(sireg, bits);
        else
            csr_clear(sireg, bits);
    }
}
