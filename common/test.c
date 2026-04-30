// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "test.h"
#include "print.h"

int test_num;
const char * test_name;
int passed;

void
start_test(const char * name)
{
    test_num++;
    test_name = name;
    passed = 1;
}

void
end_test()
{
    print_str("Test ");
    if (*test_name)
        print_str(test_name);
    else
        print_int(test_num);
    if (passed)
        print_str(": \x1b[32mPASS\x1b[0m.\n");
    else
        print_str(": \x1b[31mFAIL\x1b[0m.\n");
}
