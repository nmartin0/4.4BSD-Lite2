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
# build/test/uucplock_snprintf.c (docs/provenance/smaller-fixes.md).
# First greps the real usr.bin/tip/uucplock.c for the two snprintf
# calls and the real pathnames.h/sys/dirent.h for the format string
# and MAXNAMLEN this test reproduces, so it can't quietly drift from
# them.
#
#   sh build/test/uucplock_snprintf.sh
set -e
cd "$(dirname "$0")/../.."

grep -qE 'snprintf\(tbuf, sizeof\(tbuf\), _PATH_LOCKDIRNAME, ttyname\);' \
    usr/src/usr.bin/tip/uucplock.c || {
	echo "uucplock_snprintf.sh: uu_lock() no longer snprintf()s tbuf -- the fix may be stale" >&2
	exit 1
}
n=$(grep -cE 'snprintf\(tbuf, sizeof\(tbuf\), _PATH_LOCKDIRNAME, ttyname\);' \
    usr/src/usr.bin/tip/uucplock.c)
test "$n" = 2 || {
	echo "uucplock_snprintf.sh: expected 2 snprintf(tbuf, ...) calls (uu_lock and uu_unlock), found $n" >&2
	exit 1
}
grep -qE '_PATH_LOCKDIRNAME[[:space:]]+"/var/spool/lock/LCK\.\.%s"' \
    usr/src/usr.bin/tip/pathnames.h || {
	echo "uucplock_snprintf.sh: _PATH_LOCKDIRNAME no longer matches what this test reproduces" >&2
	exit 1
}
grep -qE '^#define[[:space:]]+MAXNAMLEN[[:space:]]+255' \
    usr/src/sys/sys/dirent.h || {
	echo "uucplock_snprintf.sh: MAXNAMLEN no longer matches what this test reproduces" >&2
	exit 1
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -g -fno-stack-protector -D_FORTIFY_SOURCE=0 \
    -o "$w/uucplock_snprintf" build/test/uucplock_snprintf.c
"$w/uucplock_snprintf" > "$w/got"
cat "$w/got"
diff -u build/test/uucplock_snprintf.want "$w/got"
