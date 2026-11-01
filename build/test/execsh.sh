#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: runs
# build/test/execsh.c through ktest.sh with this tree's own /bin/sh and
# ls, from the target root build/sysroot.sh made, on the disk beside it.
#
#   sh build/test/execsh.sh [kernel]
: "${L2_BUILD:=${HOME}/.cache/lite2}"
R=$L2_BUILD/root
[ -x $R/bin/sh ] && [ -x $R/bin/ls ] || { echo "no $R/bin/sh or ls -- run build/sysroot.sh" >&2; exit 1; }
KTEST_FILES="bin $R/bin/sh x $R/bin/ls" exec sh build/test/ktest.sh execsh "$@"
