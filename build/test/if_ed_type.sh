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
# build/test/if_ed_type.c (docs/provenance/if_ed-assign.md) and first
# checks, by grep, that its decide_post() guard line is still copied
# verbatim from the real source and that the real source no longer
# has the pre-fix form anywhere -- so the test can't quietly drift
# from what if_ed.c actually says.
#
#   sh build/test/if_ed_type.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/sys/i386/isa/if_ed.c
if grep -n 'if (sc->type = ED_TYPE_SMC8216C)' "$src"; then
	echo "if_ed_type.sh: $src still has the pre-fix '=' form -- stopping" >&2
	exit 1
fi
n=$(grep -c 'if (sc->type == ED_TYPE_SMC8216C) {' "$src")
if [ "$n" -ne 2 ]; then
	echo "if_ed_type.sh: expected 2 fixed guards in $src, found $n -- the test's copied lines may be stale" >&2
	exit 1
fi
# Both guards share the same condition line, so that count alone
# can't tell the two apart; check each one's own two strings are
# still there too, so a drift in either guard's body is caught.
for s in 'SMC8216/SMC8216C' 'SMC8216T' 'SMC8416C/SMC8416BT' 'SMC8416T'; do
	grep -qF "\"$s\"" "$src" || {
		echo "if_ed_type.sh: string \"$s\" not found in $src -- the test's copied strings may be stale" >&2
		exit 1
	}
done
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -Wall -Wno-parentheses build/test/if_ed_type.c -o "$w/if_ed_type"
"$w/if_ed_type" > "$w/got"
diff -u build/test/if_ed_type.want "$w/got"
cat "$w/got"
