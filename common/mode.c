#include "mode.h"
#include "handlers.h"

#include <stdint.h>

void
go_to_umode(void (*fp)())
{
    asm volatile("csrc mstatus, %0" :: "r"(0x1800) : "memory");
    asm volatile("csrw mepc, %0"    :: "r"(fp)    : "memory");
    asm volatile("mret" ::: "memory");
}

void
go_to_smode(void (*fp)())
{
    asm volatile("csrc mstatus, %0" :: "r"(0x1800) : "memory");
    asm volatile("csrs mstatus, %0" :: "r"(0x800)  : "memory");
    asm volatile("csrw mepc, %0"    :: "r"(fp)    : "memory");
    asm volatile("mret" ::: "memory");
}

void
go_to_mmode(void (*fp)())
{
    ecall_dest = (uint64_t) fp;
    asm volatile("ecall" ::: "memory");
}
