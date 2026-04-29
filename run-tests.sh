#!/usr/bin/env bash

for DIR in [0-9]*; do
    printf "%-20s" "$DIR"

    if make -s -C $DIR run > /dev/null 2>&1; then
        printf "\x1b[32mPASS\x1b[0m\n"
    else
        printf "\x1b[31mFAIL\x1b[0m\n"
    fi
done
