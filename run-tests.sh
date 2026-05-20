#!/bin/bash
# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0

rc=0
for DIR in [0-9]*; do
    printf "%-32s" "$DIR"
    if make -s -C "$DIR" run > /dev/null 2>&1; then
        printf "\e[32mPASS\e[0m\n"
    else
        printf "\e[31mFAIL\e[0m\n"
        rc=1
    fi
done
exit $rc
