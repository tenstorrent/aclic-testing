#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include <assert.h>

int main()
{
    // Odd-numbered xiselect between 0x1000 and 0x1100 do not exist for XLEN=64
    {
        write_iselect(DOMAIN_M, 0x1001);
        expect_exception(2) {
            read_iregn(DOMAIN_M, 1);
        }
    }

    // xireg4-6 cause illegal instruction exception
    {
        write_iselect(DOMAIN_M, 0x1000);
        for (int i = 4; i < 7; i++)
            expect_exception(2) { read_iregn(DOMAIN_M, i); }
    }

    // When mstateen0.ACLIC (bit 53) is 0, accessing acliciprio and aclicsourcecfg
    // in S-mode or lower raises illegal instruction exception
    {
        // TODO: go to S-mode and try to access acliciprio and aclicsourcecfg
        csr_clear(MSTATEEN0, 1ull << 53);
    }

    return 0;
}
