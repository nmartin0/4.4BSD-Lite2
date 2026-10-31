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
# build/test/mkdir_nul.c (docs/provenance/mkdir-nul.md). mkdir(1) is a
# plain host program with no device dependency, so this builds it
# with the host's own cc, not the cross toolchain -- build/kernel.sh
# is not involved.
#
#   sh build/test/mkdir_nul.sh
set -e
cd "$(dirname "$0")/../.."
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -D'__P(x)=x' -o "$w/t" build/test/mkdir_nul.c
"$w/t" > "$w/got"
cat "$w/got"
diff -u build/test/mkdir_nul.want "$w/got"
