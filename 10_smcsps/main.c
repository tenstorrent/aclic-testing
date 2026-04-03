#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include "common/htif.h"
#include <assert.h>

#if 0
#define mcspspush(rd, rs)   asm volatile (".insn i 0x73, 0, %0, %1, 0x309" : "=r"(rd) : "r"(rs)  : "memory")
#define mcspspop(rd, rs)    asm volatile (".insn i 0x73, 0, %0, %1, 0x30c" : "=r"(rd) : "r"(rs)  : "memory")
#define scspspush(rd, rs)   asm volatile (".insn i 0x73, 0, %0, %1, 0x109" : "=r"(rd) : "r"(rs)  : "memory")
#define scspspop(rd, rs)    asm volatile (".insn i 0x73, 0, %0, %1, 0x10c" : "=r"(rd) : "r"(rs)  : "memory")
#else
#define mcspspush   asm volatile (".insn i 0x73, 0, sp, sp, 0x309" ::: "memory")
#define mcspspop    asm volatile (".insn i 0x73, 0, sp, sp, 0x30c" ::: "memory")
#define scspspush   asm volatile (".insn i 0x73, 0, sp, sp, 0x109" ::: "memory")
#define scspspop    asm volatile (".insn i 0x73, 0, sp, sp, 0x10c" ::: "memory")
#endif

int main()
{
    //register uint64_t rd asm("sp");
    //register uint64_t rs asm("sp");
    uint64_t mspcs;
    uint64_t sspcs;

    // PPUSH is saved unconditionally, PUSH is restored unconditionally
    {
        csr_set(MSPCS, 1); // PPUSH = 1
        csr_set(SSPCS, 1); // PPUSH = 1

        mcspspush;
        csr_read(MSPCS, mspcs);
        assert(mspcs == 0);

        scspspush;
        csr_read(SSPCS, sspcs);
        assert(sspcs == 0);

        csr_set(MSPCS, 2); // PUSH = 1
        csr_set(SSPCS, 2); // PUSH = 1

        mcspspop;
        csr_read(MSPCS, mspcs);
        assert(mspcs == 0);

        scspspop;
        csr_read(SSPCS, sspcs);
        assert(sspcs == 0);
    }

    // PUSH is cleared on push and PPUSH is cleared on pop
    {
        csr_set(MSPCS, 2); // PUSH = 1

        mcspspush;
        csr_read(MSPCS, mspcs);
        assert((mspcs & 3) == 1); // PPUSH == 1

        mcspspop;
        csr_read(MSPCS, mspcs);
        assert((mspcs & 3) == 2); // PUSH == 1

        csr_set(SSPCS, 2); // PUSH = 1

        scspspush;
        csr_read(SSPCS, sspcs);
        assert((sspcs & 3) == 1); // PPUSH == 1

        scspspop;
        csr_read(SSPCS, sspcs);
        assert((sspcs & 3) == 2); // PUSH == 1
    }
}
