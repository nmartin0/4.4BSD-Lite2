#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: confirms
# usr/src/sys/kern/kern_exec.c no longer quotes 4.4BSD's encumbered
# getxfile() verbatim inside a comment (docs/provenance/exec-stack.md)
# and that the replacement reasoning is still present. Fails if either
# the quoted fragment reappears or the replacement comment is removed
# without putting something else in its place.
#
#   sh build/test/exec_getxfile_quote.sh
set -e
cd "$(dirname "$0")/../.."

F=usr/src/sys/kern/kern_exec.c

if grep -q 'trunc_page(USRSTACK - size) - NBPG;.*XXX' "$F"; then
	echo "exec_getxfile_quote.sh: $F quotes the encumbered getxfile() fragment verbatim -- regression" >&2
	exit 1
fi

if grep -qE '^\s*\*\s*(size = round_page|#ifdef\s+i386|addr = trunc_page)' "$F"; then
	echo "exec_getxfile_quote.sh: $F's comments contain a quoted C fragment -- check it isn't quoting getxfile() again" >&2
	exit 1
fi

grep -q 'AI-ONLY NOTE (2026-10-31; docs/provenance/exec-stack.md)' "$F" || {
	echo "exec_getxfile_quote.sh: $F's replacement reasoning is missing -- stale" >&2
	exit 1
}

grep -q 'trunc_page(USRSTACK - size) - NBPG' "$F" || {
	echo "exec_getxfile_quote.sh: $F no longer computes the stack address this way at all -- check the fix is still in place" >&2
	exit 1
}

echo "all checks passed: no quoted getxfile() text; the fix's own reasoning is in its place"
