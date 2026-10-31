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
# build/test/re_comp.c (docs/provenance/smaller-fixes.md), which
# #includes the real lib/libcompat/4.3/regex.c and
# lib/libcompat/regexp/regexp.c directly. First greps the real
# unistd.h to confirm it still prototypes re_comp()/re_exec() as
# `const char *', and regex.c to confirm it still checks for an
# empty string, so the test can't quietly drift from either.
#
# The real usr/src/include/regexp.h is copied (not -I'd directly, see
# re_comp.c's own note) into a scratch include directory alongside a
# copy of regmagic.h, so the host's own cc can build these two BSD
# source files without usr/src/include's own stddef.h/sys/types.h
# shadowing the host's and failing on <machine/ansi.h>.
#
#   sh build/test/re_comp.sh
set -e
cd "$(dirname "$0")/../.."

grep -qE 're_comp[[:space:]]*__P\(\(const char \*\)\);' usr/src/include/unistd.h || {
	echo "re_comp.sh: re_comp()'s prototype in usr/src/include/unistd.h isn't 'const char *' -- the test's expectation may be stale" >&2
	exit 1
}
grep -qE 're_exec[[:space:]]*__P\(\(const char \*\)\);' usr/src/include/unistd.h || {
	echo "re_comp.sh: re_exec()'s prototype in usr/src/include/unistd.h isn't 'const char *' -- the test's expectation may be stale" >&2
	exit 1
}
src=usr/src/lib/libcompat/4.3/regex.c
grep -qE "if \(s == NULL \|\| \*s == '\\\\0'\)" "$src" || {
	echo "re_comp.sh: $src doesn't check for an empty string in re_comp() -- the fix may be stale" >&2
	exit 1
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
cp usr/src/include/regexp.h "$w/"
cp usr/src/lib/libcompat/regexp/regmagic.h "$w/"

gcc -std=gnu89 -w -g -I "$w" -o "$w/re_comp" build/test/re_comp.c
"$w/re_comp" > "$w/got"
cat "$w/got"
diff -u build/test/re_comp.want "$w/got"
