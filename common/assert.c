// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "util.h"
#include "print.h"
#include "htif.h"
#include <stdint.h>

void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
#if ASSERT_PRINT
    print_str(file);
    print_str(":");
    print_int(line);
    print_str(": ");
    print_str(func);
    print_str(": Assertion `");
    print_str(failedexpr);
    print_str("' failed.\n");
#else
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    UNUSED(failedexpr);
#endif
    tohost = 3;
}
