// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

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

    // When interrupt sources have equal priority number, the source with the
    // lowest identity number has the highest priority
    {
        set_source_mode(DOMAIN_M, 1, SM_EDGE1);
        set_source_mode(DOMAIN_M, 2, SM_EDGE1);
        set_iprio(DOMAIN_M, 1, 1);
        set_iprio(DOMAIN_M, 2, 1);
        set_enabled(DOMAIN_M, 1, 1);
        set_enabled(DOMAIN_M, 2, 1);
        expect_interrupt(1) {
            expect_interrupt(2) {
                write_iregn_select(DOMAIN_M, 1, 0x6, EIP0);
            }
        }
    }

    // TODO: Sources with iprio >= xithreshold & NIPPRIO_MASK are suppressed
    {
        set_source_mode(DOMAIN_M, 1, SM_EDGE1);
        set_source_mode(DOMAIN_M, 2, SM_EDGE1);
        set_iprio(DOMAIN_M, 1, 1);
        set_iprio(DOMAIN_M, 2, 2);
        set_enabled(DOMAIN_M, 1, 1);
        set_enabled(DOMAIN_M, 2, 1);

        uint64_t mithreshold = 4;
        csr_write(MITHRESHOLD, mithreshold);

        // TODO: re-enable interrupts during handler
        expect_interrupt(1) {
            expect_interrupt(2) {
                // pithreshold set to 4
                // mithreshold set to 1 during handler
                write_iregn_select(DOMAIN_M, 1, 0x6, EIP0);
            }
        }
        csr_read(MITHRESHOLD, mithreshold);
        assert(mithreshold == 4);

        uint64_t mpistatus;
        csr_read(MPISTATUS, mpistatus);
        uint64_t pithreshold = mpistatus & 0xff;
        assert(pithreshold == 4);
    }

    // Set/clear multiple pending/enabled bits for sources with different
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

    // TODO: Change priority of pending interrupt
#if 0
    {
        // TODO: set threshold?
        set_iprio(DOMAIN_M, id, 2);
    }
#endif

  // TODO: While handling an interrupt with iprio N, interrupt sources with iprio >= N
  // are suppressed until handling is finished

  // TODO: xithreshold is saved/restore to/from xpistatus.pithreshold

    exit(0);
}
