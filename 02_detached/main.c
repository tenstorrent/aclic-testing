#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include <assert.h>

int main()
{
    int domain = DOMAIN_M;

#if ACLIC_HINTS
    // Wire input is ignored for detached sources
    {
        int id = 1;
        set_source_mode(domain, id, SM_DETACHED);
        set_source_state(id, 1);
    }
#endif

    return 0;
}
