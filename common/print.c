// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#include "print.h"

volatile char __whisper_console_io;

void print_str(const char * s)
{
    char c;
    while ((c = *s++))
        __whisper_console_io = c;
}

void print_int(int x)
{
    char s[32];
    char * p = s + sizeof(s) - 1;
    *p = 0;
    int neg = x < 0;
    if (neg)
        x *= -1;
    if (x == 0)
        *--p = '0';
    while (x) {
        *--p = (x % 10) + '0';
        x /= 10;
    }
    if (neg)
        *--p = '-';
    print_str(p);
}
