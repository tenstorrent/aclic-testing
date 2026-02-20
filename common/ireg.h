#ifndef IREG_H
#define IREG_H

#include <stdint.h>

#define SIREG  0x151
#define SIREG2 0x152
#define SIREG3 0x153
#define SIREG4 0x155
#define SIREG5 0x156
#define SIREG6 0x157

#define MIREG  0x351
#define MIREG2 0x352
#define MIREG3 0x353
#define MIREG4 0x355
#define MIREG5 0x356
#define MIREG6 0x357

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
