// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef UTIL_H
#define UTIL_H

#define countof(X)      (sizeof(X)/sizeof((X)[0]))
#define UNUSED(X)       (void)(X)

#define concat(A, B) A ## B
#define concat2(A, B) concat(A, B)
#define macro_var(name) concat2(name, __LINE__)
#define defer(start, end) for (             \
        int macro_var(_i_) = (start, 0);    \
        !macro_var(_i_);                    \
        (macro_var(_i_) += 1), end)


#endif
