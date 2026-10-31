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
# build/test/sa_argv.c (docs/provenance/sa-argv.md), which reproduces
# usr.sbin/sa/main.c's two warn("...%s", *argv) calls in a standalone
# host program rather than including main.c itself (main.c's real
# calls only run from a live, privileged accounting-file failure,
# which isn't practical to stage here -- see the AI-ONLY NOTE in
# sa_argv.c for why). Because of that, this script first checks by
# grep that the real source still has both fixed calls, so the test
# can't quietly drift from what main.c actually says.
#
#   sh build/test/sa_argv.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/usr.sbin/sa/main.c
if grep -nE 'warn\("couldn.t truncate %s", *argv\)' "$src" | grep -v '\*argv'; then
	echo "sa_argv.sh: $src's truncate warn() no longer uses *argv -- stopping" >&2
	exit 1
fi
for pat in 'warn\("couldn.t truncate %s", \*argv\)' 'warn\("close %s", \*argv\)'; do
	grep -qE "$pat" "$src" || {
		echo "sa_argv.sh: expected pattern not found in $src -- the test's copied shape may be stale" >&2
		exit 1
	}
done
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -o "$w/sa_argv" build/test/sa_argv.c
"$w/sa_argv" > "$w/got" 2>"$w/err"
cat "$w/got"
cat "$w/err" >&2
diff -u build/test/sa_argv.want "$w/got"
