// SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
// SPDX-License-Identifier: Apache-2.0

#ifndef MODE_H
#define MODE_H

void go_to_umode(void (*fp)());

void go_to_smode(void (*fp)());

void go_to_mmode(void (*fp)());

#endif
