#include "common/csr.h"
//#include "common/handlers.h"
#include "common/ireg.h"
#include "common/aclic.h"
#include "common/interrupt.h"

//void
//handler()
//{
//}

//extern void (*mhandler_preemption)();

void install_mhandler_preemption(void);

int main()
{
    //install_handler(DOMAIN_M, handler);
    //csr_write(mtvec, mhandler_preemption);
    install_mhandler_preemption();
    set_source_mode(DOMAIN_M, 1, SM_EDGE1);
    set_source_mode(DOMAIN_M, 2, SM_EDGE1);
    set_iprio(DOMAIN_M, 1, 2);
    set_iprio(DOMAIN_M, 2, 1);
    set_enabled(DOMAIN_M, 1, 1);
    set_enabled(DOMAIN_M, 2, 1);
    //todo[todo_len++] = (Todo) { .addr = 0x1000, .data = 0xdeadbeef };
#if ACLIC_HINTS
    //expect_interrupt(1) {
        set_source_state(1, 1);
    //}
#endif
    return 0;
}
