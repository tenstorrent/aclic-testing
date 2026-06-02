// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef TEST_H
#define TEST_H

#include "util.h"

#define test(name) defer(start_test(name), end_test())
#define fail_if(condition) if (condition) { passed = 0; continue; }

// Skip the current test block when a required extension/feature is absent.
// Reports SKIP (counts as neither pass nor fail).  Place at the top of a
// test() block, before touching the gated CSRs/instructions.
#define skip_unless(condition) if (!(condition)) { skipped = 1; continue; }

void start_test(const char * name);
void end_test();

extern int test_num;
extern int passed;
extern int skipped;
extern int test_failures;
extern int test_skips;
extern const char * test_name;

#endif
