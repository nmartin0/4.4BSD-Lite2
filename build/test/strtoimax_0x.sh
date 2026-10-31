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
# build/test/strtoimax_0x.c (docs/provenance/smaller-fixes.md), which
# #includes the real lib/libc/stdlib/strtoimax.c and strtoumax.c
# directly. First greps both real files to confirm the hex-digit
# check after "0x"/"0X" is still there, so the test can't quietly
# drift from them.
#
#   sh build/test/strtoimax_0x.sh
set -e
cd "$(dirname "$0")/../.."

for f in usr/src/lib/libc/stdlib/strtoimax.c usr/src/lib/libc/stdlib/strtoumax.c; do
	grep -qE "\(s\[1\] >= '0' && s\[1\] <= '9'\)" "$f" || {
		echo "strtoimax_0x.sh: $f doesn't check s[1] is a hex digit -- the fix may be stale" >&2
		exit 1
	}
done

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -g -o "$w/strtoimax_0x" build/test/strtoimax_0x.c
"$w/strtoimax_0x" > "$w/got"
cat "$w/got"
diff -u build/test/strtoimax_0x.want "$w/got"
