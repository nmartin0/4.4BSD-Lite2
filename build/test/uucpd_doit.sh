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
# build/test/uucpd_doit.c (docs/provenance/uucpd-doit.md), which
# reproduces libexec/uucpd/uucpd.c's doit() ordering mistake
# standalone (doit() itself can't be called in a test: it reads a
# login from fd 0 and ends by really exec'ing uucico). This script
# first greps the real uucpd.c to confirm setuid() still runs before
# chdir() and setlogin() is still called, so the test can't quietly
# drift from what doit() actually does; then, if run as root (the
# test needs real root to show real root's permission bypass -- see
# the AI-ONLY NOTE in uucpd_doit.c), builds and runs the harness.
#
#   sh build/test/uucpd_doit.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/libexec/uucpd/uucpd.c

setuid_line=$(grep -n 'setuid(pw->pw_uid);' "$src" | head -1 | cut -d: -f1)
chdir_line=$(grep -n 'chdir(pw->pw_dir);' "$src" | head -1 | cut -d: -f1)
[ -n "$setuid_line" ] && [ -n "$chdir_line" ] || {
	echo "uucpd_doit.sh: couldn't find both setuid() and chdir() calls in $src -- the test's copied shape may be stale" >&2
	exit 1
}
if [ "$setuid_line" -gt "$chdir_line" ]; then
	echo "uucpd_doit.sh: $src calls chdir() (line $chdir_line) before setuid() (line $setuid_line) -- the pre-fix order -- stopping" >&2
	exit 1
fi

grep -qE 'setlogin\(user\);' "$src" || {
	echo "uucpd_doit.sh: setlogin(user) not found in $src -- the test's copied shape may be stale" >&2
	exit 1
}

if [ "$(id -u)" != "0" ]; then
	echo "uucpd_doit.sh: not root -- skipping (root's chdir() permission bypass, the actual bug, can't be shown otherwise)"
	exit 0
fi

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -o "$w/uucpd_doit" build/test/uucpd_doit.c
"$w/uucpd_doit" > "$w/got"
cat "$w/got"
diff -u build/test/uucpd_doit.want "$w/got"
