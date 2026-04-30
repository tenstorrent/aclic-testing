// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef TEST_H
#define TEST_H

#include "common/util.h"

#define test(name) defer(start_test(name), end_test())
#define fail_if(condition) if (condition) { passed = 0; continue; }

void start_test(const char * name);

void end_test();

#endif
