#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include "common/htif.h"
#include <assert.h>

void
umode()
{
    int id = 1;
    expect_interrupt(id) {
        set_source_state(id, 1);
    }
    exit(0);
}

int main()
{
    csr_set(mstatus, 1u << 25); // set MIPU (NOTE: bit position will change!)

    int id = 1;
    set_source_mode(DOMAIN_M, id, SM_EDGE1);
    set_enabled(DOMAIN_M, id, 1);
    // TODO: configure page table so that sp points to invalid page when
    // interrupt is taken
    go_to_umode(umode);
}
