#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include "common/htif.h"
#include <assert.h>

int main()
{
    // target[i].iprio is WARL: between 1 and (2^IPRIOLEN)-1
    {
        int id = 1;
        set_source_mode(DOMAIN_M, id, SM_EDGE1);

        int illegal_iprio = 0;
        set_iprio(DOMAIN_M, id, illegal_iprio);
        int iprio = get_iprio(DOMAIN_M, id);
        assert(iprio != illegal_iprio);

        illegal_iprio = 1 << IPRIOLEN;
        set_iprio(DOMAIN_M, id, illegal_iprio);
        iprio = get_iprio(DOMAIN_M, id);
        assert(iprio != illegal_iprio);
    }

    // Set/clear multiple pending/endabled bits for sources with different
    // priorities in a single write a write
    {
        set_source_mode(DOMAIN_M, 1, SM_EDGE1);
        set_source_mode(DOMAIN_M, 2, SM_EDGE1);
        set_source_mode(DOMAIN_M, 3, SM_EDGE1);
        set_iprio(DOMAIN_M, 1, 3);
        set_iprio(DOMAIN_M, 2, 1);
        set_iprio(DOMAIN_M, 3, 2);
        set_enabled(DOMAIN_M, 1, 1);
        set_enabled(DOMAIN_M, 2, 1);
        set_enabled(DOMAIN_M, 3, 1);
        write_iselect(DOMAIN_M, EIP0);
        expect_interrupt(2) {
            expect_interrupt(3) {
                expect_interrupt(1) {
                    write_iregn(DOMAIN_M, 1, 0xe);
                }
            }
        }
    }

    // Change priority of pending interrupt
#if 0
    {
        // TODO: set threshold?
        set_iprio(DOMAIN_M, id, 2);
    }
#endif

    exit(0);
}
