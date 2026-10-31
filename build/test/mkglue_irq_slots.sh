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
# build/test/mkglue_irq_slots.c (docs/provenance/intr-slots.md), a
# standalone model of the OLD (patch 222) and NEW (patch 258)
# interrupt counter-slot schemes, against GENERIC.i386's real isa
# device list. First greps the real mkglue.c for the anchors the
# model assumes are still true, so the test can't quietly drift from
# the source it is modelling.
#
#   sh build/test/mkglue_irq_slots.sh
set -e
cd "$(dirname "$0")/../.."

M=usr/src/usr.sbin/config/mkglue.c
grep -q 'INTR2(8, _C_LABEL(highmask), 8)' "$M" || {
	echo "mkglue_irq_slots.sh: $M's VEC(rtc) is no longer INTR2(8,...,8) -- stale" >&2
	exit 1
}
grep -q 'claimed\[dp->d_irq\]' "$M" || {
	echo "mkglue_irq_slots.sh: $M no longer tracks claimed[] by irq -- stale" >&2
	exit 1
}
grep -q 'sprintf(namebuf\[dp->d_irq\], "%s%d",' "$M" || {
	echo "mkglue_irq_slots.sh: $M no longer names slots by dp->d_irq -- stale" >&2
	exit 1
}
grep -q '_C_LABEL(isa_intr) + 64' "$M" || {
	echo "mkglue_irq_slots.sh: $M no longer sets eintrcnt to isa_intr+64 -- stale" >&2
	exit 1
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -g -o "$w/mkglue_irq_slots" build/test/mkglue_irq_slots.c
"$w/mkglue_irq_slots" > "$w/got"
cat "$w/got"
diff -u build/test/mkglue_irq_slots.want "$w/got"
