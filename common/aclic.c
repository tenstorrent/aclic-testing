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

// ----------------------------------------------------------------------------
// ACLIC packed indirect-register layout (XLEN-portable).
//
// These mirror whisper's Aclic.cpp: a sourcecfg register holds SCFG_FIELDS
// 16-bit fields (xireg2 = low half, xireg3 = high half of a 2*SCFG_FIELDS block
// of sources); an iprio register holds IPRIO_FIELDS 8-bit fields.  The field
// counts are compile-time constants derived from XLEN, so there is no run-time
// cost and no separate rv32/rv64 code paths.  Source `id` (1..NUM_SOURCES) maps
// to whisper's selector `k` via the same `k*4` source stride it uses internally.
// ----------------------------------------------------------------------------

static int scfg_select(int id) {
    return 0x1000 + (id / (2 * SCFG_FIELDS)) * (SCFG_FIELDS / 2);
}
static int scfg_regn(int id) {          // xireg2 (low half) or xireg3 (high half)
    return ((id % (2 * SCFG_FIELDS)) < SCFG_FIELDS) ? 2 : 3;
}
static int scfg_shift(int id) {         // bit offset of this source's 16-bit field
    return ((id % (2 * SCFG_FIELDS)) % SCFG_FIELDS) * 16;
}
static int iprio_select(int id) {
    return 0x1000 + (id / IPRIO_FIELDS) * (IPRIO_FIELDS / 4);
}
static int iprio_shift(int id) {        // bit offset of this source's 8-bit field
    return (id % IPRIO_FIELDS) * 8;
}

void
set_source_mode(int domain, int id, int sm)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    int n = scfg_regn(id), shamt = scfg_shift(id);
    xlreg_t mask = (xlreg_t)0xffff << shamt;
    write_iselect(domain, scfg_select(id));
    xlreg_t value = read_iregn(domain, n);
    value = (value & ~mask) | ((xlreg_t)sm << shamt);
    write_iregn(domain, n, value);
}

int
get_source_mode(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int n = scfg_regn(id), shamt = scfg_shift(id);
    write_iselect(domain, scfg_select(id));
    xlreg_t value = read_iregn(domain, n);
    return (value >> shamt) & 0x7;
}

int
get_sourcecfg(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int n = scfg_regn(id), shamt = scfg_shift(id);
    write_iselect(domain, scfg_select(id));
    xlreg_t value = read_iregn(domain, n);
    return (value >> shamt) & 0xffff;
}

void
set_iprio(int domain, int id, int iprio)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    iprio &= 0xff;
    int shamt = iprio_shift(id);
    xlreg_t mask = (xlreg_t)0xff << shamt;
    xlreg_t value = read_iregn_select(domain, 1, iprio_select(id));
    value = (value & ~mask) | ((xlreg_t)iprio << shamt);
    write_iregn(domain, 1, value);
}

int
get_iprio(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    int shamt = iprio_shift(id);
    xlreg_t value = read_iregn_select(domain, 1, iprio_select(id));
    return (value >> shamt) & 0xff;
}

void
delegate(int id)
{
    int n = scfg_regn(id), shamt = scfg_shift(id);
    xlreg_t mask = (xlreg_t)0xffff << shamt;
    xlreg_t d = (xlreg_t)(1u << 10) << shamt;   // sourcecfg.D
    xlreg_t value = read_iregn_select(DOMAIN_M, n, scfg_select(id));
    value = (value & ~mask) | d;
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
    write_iselect(domain, EIE0 + id / XLEN);
    xlreg_t bits = (xlreg_t)1 << (id % XLEN);
    set_or_clear_ireg_bits(domain, bits, enabled);
}

int
get_enabled(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    xlreg_t bits = read_iregn_select(domain, 1, EIE0 + id / XLEN);
    return (bits >> (id % XLEN)) & 1;
}

void
set_pending(int domain, int id, int pending)
{
    if (id == 0 || id > NUM_SOURCES)
        return;
    write_iselect(domain, EIP0 + id / XLEN);
    xlreg_t bits = (xlreg_t)1 << (id % XLEN);
    set_or_clear_ireg_bits(domain, bits, pending);
}

int
get_pending(int domain, int id)
{
    if (id == 0 || id > NUM_SOURCES)
        return 0;
    xlreg_t bits = read_iregn_select(domain, 1, EIP0 + id / XLEN);
    return (bits >> (id % XLEN)) & 1;
}

uint64_t
read_miconfig(void)
{
    return read_iregn_select(DOMAIN_M, 4, 0x1000);
}

void
write_miconfig(uint64_t v)
{
    write_iregn_select(DOMAIN_M, 4, v, 0x1000);
}

uint64_t
read_siconfig(void)
{
    return read_iregn_select(DOMAIN_S, 4, 0x1000);
}

void
write_siconfig(uint64_t v)
{
    write_iregn_select(DOMAIN_S, 4, v, 0x1000);
}
