#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdint.h>

extern volatile uint64_t ecall_dest;

extern volatile uint64_t exp_intr_ids[8];
extern volatile uint64_t num_exp_intr;
extern volatile uint64_t intr_ids_seen[8];
extern volatile uint64_t num_intr_seen;

extern volatile uint8_t excp_ok;
extern volatile uint64_t excp_mcause;

#endif
