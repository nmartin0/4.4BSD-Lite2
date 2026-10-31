#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: checks
# the two man-page corrections from docs/provenance/smaller-fixes.md
# (bestfit-sweep.tsv rows #370 and #567): that the wrong wording is
# gone and the corrected wording is present, and that both pages still
# parse as mdoc (groff -mdoc -Tascii -ww, treating warnings as
# failures) -- a man page has no behavior to run, so this is the
# lightest check that actually exercises the page rather than just
# grepping it.
#
#   sh build/test/manpage_fixes.sh
set -e
cd "$(dirname "$0")/../.."

m4=usr/src/usr.bin/m4/m4.1
deroff=usr/src/usr.bin/deroff/deroff.1

echo "T checking $m4"
grep -q "loosing their original meaning" "$m4" && {
	echo "manpage_fixes.sh: $m4 still says 'loosing' -- fix missing" >&2
	exit 1
}
grep -q "losing their original meaning" "$m4" || {
	echo "manpage_fixes.sh: $m4 doesn't say 'losing' either -- wording changed under us" >&2
	exit 1
}
echo "T  'loosing' -> 'losing': ok"

echo "T checking $deroff"
grep -A2 '^\.It m$' "$deroff" | grep -q 'Xr mm 7' || {
	echo "manpage_fixes.sh: $deroff's -m m entry doesn't cite mm(7) -- fix missing" >&2
	exit 1
}
grep -A2 '^\.It s$' "$deroff" | grep -q 'Xr ms 7' || {
	echo "manpage_fixes.sh: $deroff's -m s entry doesn't cite ms(7) -- fix missing" >&2
	exit 1
}
grep -A1 '^\.It l$' "$deroff" | grep -q 'remove list constructs' || {
	echo "manpage_fixes.sh: $deroff's -m l entry doesn't say 'remove list constructs' -- fix missing" >&2
	exit 1
}
echo "T  -m m -> mm(7), -m s -> ms(7), -m l reworded: ok"

echo "T checking both pages still parse as mdoc"
for f in "$m4" "$deroff"; do
	out=$(groff -mdoc -Tascii -ww "$f" 2>&1 >/dev/null) || {
		echo "manpage_fixes.sh: groff failed on $f:" >&2
		echo "$out" >&2
		exit 1
	}
	if [ -n "$out" ]; then
		echo "manpage_fixes.sh: groff warned on $f:" >&2
		echo "$out" >&2
		exit 1
	fi
	echo "T  $f: groff -mdoc clean"
done

echo "T all checks passed"
