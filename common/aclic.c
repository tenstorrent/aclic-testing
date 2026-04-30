// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "aclic.h"
#include "ireg.h"

void
set_source_state(int id, int state)
{
    register long x22_reg asm("x22") = id;
    if (state)
        asm volatile ("addi x0, %0, 1" :: "r"(x22_reg) : "memory");
    else
        asm volatile ("addi x0, %0, 0" :: "r"(x22_reg) : "memory");
}

void
set_source_mode(int domain, int id, int sm)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    int j = id % 8;
    int shamt = (id % 4)*16;
    uint64_t mask = 0xffffull << shamt;
    write_iselect(domain, 0x1000 + id/8);
    int n = (j < 4) ? 2 : 3;
    uint64_t value = read_iregn(domain, n);
    value &= ~mask;
    value |= (uint64_t)sm << shamt;
    write_iregn(domain, n, value);
}

int
get_source_mode(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int j = id % 8;
    int shamt = (id % 4)*16;
    write_iselect(domain, 0x1000 + id/8);
    int n = (j < 4) ? 2 : 3;
    uint64_t value = read_iregn(domain, n);
    int sm = (value >> shamt) & 0x7;
    return sm;
}

int
get_sourcecfg(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int j = id % 8;
    int shamt = (id % 4)*16;
    write_iselect(domain, 0x1000 + id/8);
    int n = (j < 4) ? 2 : 3;
    uint64_t value = read_iregn(domain, n);
    int sourcecfg = (value >> shamt) & 0xffff;
    return sourcecfg;
}

void
set_iprio(int domain, int id, int iprio)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    iprio &= 0xff;
    int shamt = (id % 8)*8;
    uint64_t mask = 0xffull << shamt;
    uint64_t value = read_iregn_select(domain, 1, 0x1000 + id/8);
    value &= ~mask;
    value |= (uint64_t)iprio << shamt;
    write_iregn(domain, 1, value);
}

int
get_iprio(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int shamt = (id % 8)*8;
    uint64_t value = read_iregn_select(domain, 1, 0x1000 + id/8);
    int iprio = (value >> shamt) & 0xff;
    return iprio;
}

void
delegate(int id)
{
    int j = id % 8;
    uint64_t mask = 0xffffull << j*16;
    uint64_t d = (1ull << 10) << j*16;
    int index = 0x1000 + id/8;
    int n = (j < 4) ? 2 : 3;
    uint64_t value = read_iregn_select(DOMAIN_M, n, index);
    value &= ~mask;
    value |= d;
    write_iregn(DOMAIN_M, n, value);
}

void
undelegate(int id)
{
    set_source_mode(DOMAIN_M, id, SM_INACTIVE);
}

void
set_enabled(int domain, int id, int enabled)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    write_iselect(domain, EIE0 + id/64);
    uint64_t bits = 1u << (id % 64);
    set_or_clear_ireg_bits(domain, bits, enabled);
}

int
get_enabled(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    uint64_t bits = read_iregn_select(domain, 1, EIE0 + id/64);
    int enabled = (bits >> id % 64) & 1;
    return enabled;
}

void
set_pending(int domain, int id, int pending)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    write_iselect(domain, EIP0 + id/64);
    uint64_t bits = 1u << (id % 64);
    set_or_clear_ireg_bits(domain, bits, pending);
}

int
get_pending(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    uint64_t bits = read_iregn_select(domain, 1, EIP0 + id/64);
    int pending = (bits >> id % 64) & 1;
    return pending;
}
