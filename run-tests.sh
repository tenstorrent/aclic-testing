#!/bin/bash
# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Runs every test under each requested XLEN.  Override the widths with e.g.
#   XLENS=64 ./run-tests.sh      # only RV64
#   XLENS="32 64" ./run-tests.sh # both (default)

rc=0
XLENS="${XLENS:-64 32}"

for XLEN in $XLENS; do
    printf "===== XLEN=%s =====\n" "$XLEN"
    for DIR in [0-9]*; do
        printf "%-32s" "$DIR"
        if make -s -C "$DIR" run XLEN="$XLEN" > /dev/null 2>&1; then
            printf "\e[32mPASS\e[0m\n"
        else
            printf "\e[31mFAIL\e[0m\n"
            rc=1
        fi
    done
done
exit $rc
