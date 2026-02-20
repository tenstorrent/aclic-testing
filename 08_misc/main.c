#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include <assert.h>

void exit(int);

int main()
{
    // Pending bit is read-only for level-sensitive sources (?)
    {
        int id = 1;
        set_enabled(DOMAIN_M, id, 0);
        set_source_mode(DOMAIN_M, id, SM_LEVEL1);
        set_pending(DOMAIN_M, id, 1);
        int pending = get_pending(DOMAIN_M, id);
        assert(!pending);
    }

    // Transition between different source modes for different source states
    {
        // TODO
    }

    exit(0);
}
