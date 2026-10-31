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
# build/test/at_fixes.c (docs/provenance/at-fixes.md). The
# parsetime.c half of that test #includes the real source directly,
# so it can't drift from it by construction. The at.c half
# (writefile()'s eqp/ap mistake) is reproduced standalone instead,
# because writefile() opens real files and reads the real
# environment -- not practical to call directly here -- so this
# script first greps the real at.c to confirm it still reads
# `eqp == NULL', not `ap == NULL', so that half of the test can't
# quietly drift from what at.c actually says either.
#
#   sh build/test/at_fixes.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/usr.bin/at/at.c
if grep -nE 'if \(ap == NULL\)' "$src"; then
	echo "at_fixes.sh: $src still has the pre-fix 'ap == NULL' form -- stopping" >&2
	exit 1
fi
grep -qE 'if \(eqp == NULL\)' "$src" || {
	echo "at_fixes.sh: expected 'eqp == NULL' not found in $src -- the test's copied shape may be stale" >&2
	exit 1
}
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -I usr/src/usr.bin/at -o "$w/at_fixes" build/test/at_fixes.c
"$w/at_fixes" > "$w/got"
cat "$w/got"
diff -u build/test/at_fixes.want "$w/got"
