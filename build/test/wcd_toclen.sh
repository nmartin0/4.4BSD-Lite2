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
# build/test/wcd_toclen.c (docs/provenance/smaller-fixes.md), which
# reproduces sys/i386/isa/wcd.c's CDIOREADTOCENTRYS `len' computation
# and bound check standalone -- see that file's own note for why
# wcd.c itself can't be built or run on the host, and
# build/kernel.sh GENERIC.i386 for the real regression proof. First
# greps the real wcd.c to confirm the `len > sizeof(toc->tab)' check
# is still there, so this standalone copy can't quietly drift from
# it.
#
#   sh build/test/wcd_toclen.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/sys/i386/isa/wcd.c

grep -qE 'if \(len > sizeof\(toc->tab\)\)' "$src" || {
	echo "wcd_toclen.sh: $src doesn't check 'len > sizeof(toc->tab)' -- the fix may be stale" >&2
	exit 1
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -g -o "$w/wcd_toclen" build/test/wcd_toclen.c
"$w/wcd_toclen" > "$w/got"
cat "$w/got"
diff -u build/test/wcd_toclen.want "$w/got"
