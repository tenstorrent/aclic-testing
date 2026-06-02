#!/bin/bash
# SPDX-FileCopyrightText: © 2026 Tenstorrent USA, Inc.
# SPDX-License-Identifier: Apache-2.0
#
# Builds every test once per XLEN and runs them under every valid combination
# of the toggleable ACLIC extensions, printing only failing runs.  See
# run-combos.py for details and environment variables (WHISPER, XLENS).

exec python3 "$(dirname "$0")/run-combos.py" "$@"
