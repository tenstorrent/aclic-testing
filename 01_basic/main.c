#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"

int main()
{
    int id = 1;
    int domain = DOMAIN_M;

    // Enabled, then pending
    set_source_mode(domain, id, SM_EDGE1);
    set_iprio(domain, id, 1);
    set_enabled(domain, id, 1);
    expect_interrupt(id) {
        set_pending(domain, id, 1);
    }

    // Pending, then enabled
    set_enabled(domain, id, 0);
    set_pending(domain, id, 1);
    expect_interrupt(id) {
        set_enabled(domain, id, 1);
    }

#if ACLIC_HINTS
    // Due to source state
    expect_interrupt(id) {
        set_source_state(id, 1);
    }
#endif

    return 0;
}
