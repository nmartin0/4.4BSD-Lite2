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
# Changed (2026-10-31, the next patch): kern_exec.c is now NetBSD 1.0's
# and computes no stack address of its own -- the stack is NetBSD's
# exec_aout_setup_stack, in kern/exec_subr.c -- so the last check asks
# for that instead of for the old body's stack arithmetic.
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

if grep -q 'trunc_page(USRSTACK - size)' "$F"; then
	echo "exec_getxfile_quote.sh: $F computes the stack as the old body did -- stale" >&2
	exit 1
fi
grep -q '^exec_aout_setup_stack(p, epp)' usr/src/sys/kern/exec_subr.c || {
	echo "exec_getxfile_quote.sh: the stack is not set up by exec_subr.c's exec_aout_setup_stack -- check" >&2
	exit 1
}

echo "all checks passed: no quoted getxfile() text; NetBSD's framework sets up the stack"
