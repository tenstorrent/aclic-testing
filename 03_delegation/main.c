#include "common/csr.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"
#include "common/mode.h"
#include "common/htif.h"
#include <assert.h>

void
smode()
{
    int id = 1;
    int domain = DOMAIN_S;
    set_source_mode(domain, id, SM_EDGE1);
    set_iprio(domain, id, 1);
    set_enabled(domain, id, 1);
    // TODO: make sure that the interrupt was SEI, not MEI
    expect_interrupt(id) {
        set_pending(domain, id, 1);
    }
    exit(0);
}

int main()
{
    delegate(1);

    // Sources cannot be delegated below S-mode domain
    // Attempting to write D = 1 causes the entire register to be set to zero instead
    {
        uint64_t mask = 0xffffull << 16;
        uint64_t d = (1ull << 10) << 16;
        uint64_t value = read_iregn_select(DOMAIN_S, 2, 0x1000);
        value = (value & ~mask) | d;
        write_iregn(DOMAIN_S, 2, value);
        value = read_iregn(DOMAIN_S, 2);
        assert(value == 0);
    }

    // 0 is the only legal value for child index (is child index WLRL?)
    {
        uint64_t mask = 0xffffull << 16;
        uint64_t d = (1ull << 10) << 16;
        uint64_t value = read_iregn_select(DOMAIN_M, 2, 0x1000);
        uint64_t child_index = 1;
        value = (value & ~mask) | d | child_index;
        write_iregn(DOMAIN_M, 2, value);
        value = read_iregn(DOMAIN_M, 2);
        assert(!(value & 0x3ff));
    }

    // Clearing delegated bit in M-mode domain makes source appear unimplemented in S-mode domain
    {
        int id = 1;
        delegate(id);
        int sourcecfg = get_sourcecfg(DOMAIN_M, id);
        assert(sourcecfg == 0x400);

        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        int sm = get_source_mode(DOMAIN_S, id);
        assert(sm == SM_EDGE1);

        undelegate(id);
        sm = get_source_mode(DOMAIN_S, id);
        assert(sm == SM_INACTIVE);

        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);
        set_source_mode(DOMAIN_M, id, SM_EDGE1); // clears D
        sm = get_source_mode(DOMAIN_S, id);
        assert(sm == SM_INACTIVE);
    }

    // An interrupt source is inactive in the interrupt domain if the source is delegated
    // Whenever interrupt source i is inactive in an interrupt domain, the
    // corresponding interrupt-pending and interrupt-enable bits within the
    // domain are read-only zeros, and register target[i] is also read-only
    // zero.
    {
        int id = 1;
        delegate(id);
        set_source_mode(DOMAIN_S, id, SM_EDGE1);

        set_iprio(DOMAIN_M, id, 2);
        assert(get_iprio(DOMAIN_M, id) == 0);
        set_enabled(DOMAIN_M, id, 1);
        assert(get_enabled(DOMAIN_M, id) == 0);
        set_pending(DOMAIN_M, id, 1);
        assert(get_pending(DOMAIN_M, id) == 0);
    }

    // Test delegation of interrupt to S-mode domain
    delegate(1);
    go_to_smode(smode); // never returns

    return 0;
}
