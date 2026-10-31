#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own tests is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: a host
# test for bin/test/test.c's three-argument -a/-o handling
# (docs/provenance/test-3arg.md). test(1) is a plain host program
# (no device, no funopen), so this builds it with the host's own cc,
# not the cross toolchain, and runs it directly -- build/kernel.sh is
# not involved and this is not a QEMU test.
#
#   sh build/test/test_3arg.sh
set -e
cd "$(dirname "$0")/../.."
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -D'__P(x)=x' -o "$w/test" \
    usr/src/bin/test/test.c usr/src/bin/test/operators.c

t() {
	label=$1; shift
	got=0
	"$w/test" "$@" > /dev/null 2>&1 || got=$?
	printf 'T %s: exit=%d\n' "$label" "$got" >> "$w/got"
}

printf 'T begin\n' > "$w/got"
t '""      -a ""     '  ""  -a ""
t '""      -a "x"    '  ""  -a "x"
t '"x"     -a ""     '  "x" -a ""
t '"x"     -a "y"    '  "x" -a "y"
t '""      -o ""     '  ""  -o ""
t '""      -o "x"    '  ""  -o "x"
t '"x"     -o ""     '  "x" -o ""
t '"x"     -o "y"    '  "x" -o "y"
# These three are ordinary binary comparisons, not -a/-o, and must
# still work exactly as before: lookup_op(argv[2], andor_op) rejects
# them (not in andor_op), so posix_binary_op() still runs for these.
t 'abc = abc (unaffected)'       abc = abc
t 'abc = xyz (unaffected)'       abc = xyz
t '1 -eq 1 (unaffected)'         1 -eq 1
printf 'T end\n' >> "$w/got"
cat "$w/got"
diff -u build/test/test_3arg.want "$w/got"
