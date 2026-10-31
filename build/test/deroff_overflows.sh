#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: unlike
# this project's other build/test/*.sh scripts, there is no separate
# build/test/deroff_overflows.c harness here: all three of deroff's
# holes (docs/provenance/deroff-overflows.md) are in code paths that
# span several interacting functions and file-scope state (the
# growing `line' buffer, `inpic()''s own state machine), not one
# function that can be #included or called standalone the way
# parsetime.c's assign_date() could for at_fixes.c. So this script
# builds the real usr.bin/deroff/deroff.c itself, with the host's own
# cc, and drives the resulting binary directly against three crafted
# inputs, one per hole. `__dead' is Lite2's own macro (sys/cdefs.h,
# via the cross toolchain's own headers) and isn't defined by the
# host's; deroff.c uses it on two declarations, so the host build
# defines it away.
#
# This script's own checks print one "T ..." line per step to a
# transcript, in the same style the other tests' .c harnesses print
# themselves, and that transcript is diffed against
# build/test/deroff_overflows.want -- so a silent change in behaviour
# (not just a crash) is caught too.
#
# This script proves the FIXED binary behaves correctly; it is not
# itself how the PRE-fix crash was shown (AddressSanitizer against a
# saved pre-fix copy of the file, done once by hand -- see the
# "Test methodology and results" section of
# docs/provenance/deroff-overflows.md for the exact commands and
# crash sites). The drift-check greps below also mean this script
# can't be run against that pre-fix copy at all: it refuses before
# reaching the crash, which is correct -- its job from here on is to
# guard the fix, not to re-demonstrate the bug every time.
#
#   sh build/test/deroff_overflows.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/usr.bin/deroff/deroff.c

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
got="$w/got"

note() { echo "T $*" | tee -a "$got"; }

grep -qE 'char[[:space:]]+chars\[256\]' "$src" || {
	echo "deroff_overflows.sh: chars[] is not sized 256 in $src -- the pre-fix size, or the test's expectation, may be stale" >&2
	exit 1
}
grep -qE '^char \*line;' "$src" || {
	echo "deroff_overflows.sh: line is not a plain 'char *' in $src -- the pre-fix fixed array, or the test's expectation, may be stale" >&2
	exit 1
}
grep -qE '\.PS length exceeds limit' "$src" || {
	echo "deroff_overflows.sh: the .PS length check's message is missing from $src" >&2
	exit 1
}
note "begin"

if command -v gcc >/dev/null 2>&1 && gcc -fsanitize=address -x c -o /dev/null -c - </dev/null >/dev/null 2>&1; then
	cc="gcc -fsanitize=address"
	note "built with AddressSanitizer"
else
	cc="${CC:-cc}"
	note "built without AddressSanitizer (none found on host)"
fi
$cc -std=gnu89 -w -g -D'__dead=' -o "$w/deroff" "$src" 2>"$w/cc.err" || {
	cat "$w/cc.err" >&2
	echo "deroff_overflows.sh: build of $src failed" >&2
	exit 1
}

run() {
	# run LABEL EXPECT_EXIT FILE [ARGS...]
	label=$1; expect=$2; file=$3; shift 3
	set +e
	"$w/deroff" "$@" "$file" >"$w/out.$label" 2>"$w/err.$label"
	rv=$?
	set -e
	case "$rv" in
	134|139)
		note "$label: crashed (exit $rv)"
		cat "$w/err.$label" >&2
		exit 1
		;;
	esac
	if [ "$rv" != "$expect" ]; then
		note "$label: exit $rv (wanted $expect)"
		exit 1
	fi
	note "$label: exit $rv"
}

# Hole 1: regline()'s line buffer, long line with no embedded newline
# (OpenBSD millert, 2023-09-27).
printf '%s\n' "$(printf 'a%.0s' $(seq 1 5000))" >"$w/longline.txt"
run long_line 0 "$w/longline.txt"
if cmp -s "$w/longline.txt" "$w/out.long_line"; then
	note "long_line: output matches input"
else
	note "long_line: output does not match input"
	exit 1
fi

# Hole 2: inpic()'s .PS/.PE quoted-text copy, oversized quoted string
# (OpenBSD millert, 2014-12-03). The dummy line after .PS is needed
# because inpic() always skips the rest of the .PS control line's own
# line first (ordinary pic syntax: that line is normally a sizing
# spec, not content).
{
	echo '.PS'
	echo 'dummy'
	printf '"'
	printf 'x%.0s' $(seq 1 5000)
	printf '"\n'
	echo '.PE'
} >"$w/pslong.txt"
run ps_long 1 "$w/pslong.txt"
if grep -q '\.PS length exceeds limit' "$w/err.ps_long"; then
	note "ps_long: reported the length error"
else
	note "ps_long: did not report the length error"
	exit 1
fi

# A short, legitimate .PS block must still pass through untouched.
{
	echo '.PS'
	echo 'dummy'
	printf '"short"\n'
	echo '.PE'
} >"$w/psshort.txt"
run ps_short 0 "$w/psshort.txt"

# Hole 3: chars[]'s size, a high-bit byte under -w (OpenBSD deraadt /
# Renaud Allard, 2026-04-01).
printf 'hello \200 world\n' >"$w/highbyte.txt"
run high_byte 0 "$w/highbyte.txt" -w
if [ "$(cat "$w/out.high_byte")" = "$(printf 'hello\nworld')" ]; then
	note "high_byte: output as expected"
else
	note "high_byte: output not as expected"
	cat "$w/out.high_byte" >&2
	exit 1
fi

note "all checks passed"
note "end"

diff -u build/test/deroff_overflows.want "$got"
