// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef ACLIC_H
#define ACLIC_H

#include <stdint.h>

#define NUM_SOURCES 16
#define IPRIOLEN 8

enum {
    EIP0 = 0x80,
    EIE0 = 0xc0,
};

enum {
    SM_INACTIVE,
    SM_DETACHED,
    SM_EDGE1 = 4,
    SM_EDGE0,
    SM_LEVEL1,
    SM_LEVEL0,
};

// miconfig field shifts (spec §Smnip §"New Interrupt Configuration").
#define MICONFIG_MNIPEN_SHIFT   2
#define MICONFIG_SNIPEN_SHIFT   3
#define MICONFIG_MIPU_SHIFT     4
#define MICONFIG_SIPU_SHIFT     5
#define MICONFIG_MNIPBITS_SHIFT 8
#define MICONFIG_SNIPBITS_SHIFT 16

void set_source_state(int id, int state);

void set_source_mode(int domain, int id, int sm);
int get_source_mode(int domain, int id);
int get_sourcecfg(int domain, int id);

void set_iprio(int domain, int id, int iprio);
int get_iprio(int domain, int id);

void delegate(int id);
void undelegate(int id);

void set_enabled(int domain, int id, int enabled);
int get_enabled(int domain, int id);

void set_pending(int domain, int id, int pending);
int get_pending(int domain, int id);

// miconfig/siconfig accessors (via mireg4/sireg4 at xiselect=0x1000).
uint64_t read_miconfig(void);
void     write_miconfig(uint64_t v);
uint64_t read_siconfig(void);
void     write_siconfig(uint64_t v);

#endif
