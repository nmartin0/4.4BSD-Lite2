#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: a
# structural test for usr.sbin/pstat/pstat.c's mnt_flags[] table
# (docs/provenance/smaller-fixes.md): the bug this patch fixes is
# itself a static-initializer defect (a stray `{ MNT_DELEXPORT },'
# entry with no m_name, immediately followed by a duplicate
# `{ MNT_UPDATE, "update" },'), not something that depends on
# running the program -- pstat.c itself can't be built or run on the
# host at all (it needs this tree's own kernel headers:
# <sys/mount.h>, <ufs/ufs/quota.h>, <nfs/nfs.h>, and more, none of
# which are meant to compile standalone against a host's libc). So
# this checks the table's own source text directly, extracting it
# from the real file with sed/awk rather than copying it in here,
# so it can't drift from what's actually in the tree:
#
#   1. no entry (other than the `{ 0 }' sentinel) has just one field
#      -- the exact shape of the bug this patch removes, a flag
#      constant with no name, which the table's own print loop
#      (pstat.c, around the `for (i = 0; mnt_flags[i].m_flag; i++)'
#      loop) would reach and pass a null m_name to a %s conversion.
#   2. no MNT_* flag constant appears in the table more than once --
#      the other half of the same bug, MNT_UPDATE appearing twice.
#
# This script's own checks are tried against a deliberately
# re-broken copy first (re-inserting the exact removed line), to
# confirm they actually catch it, before checking the real file.
#
#   sh build/test/pstat_mntflags.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/usr.sbin/pstat/pstat.c

extract() {
	# Prints just the mnt_flags[] table's entry lines (between the
	# opening "mnt_flags[] = {" and its closing "};"), stripping
	# comments, blank lines and the struct declaration itself.
	awk '
		/mnt_flags\[\] = \{/ { in_table = 1; next }
		in_table && /^\};/ { in_table = 0 }
		in_table { print }
	' "$1" | sed -e 's#/\*.*\*/##' -e '/^\s*\*/d' -e '/^\s*\/\*/d' \
	    | grep -E '^\s*\{'
}

check_table() {
	# check_table FILE LABEL
	f=$1; label=$2
	bad_entry=$(extract "$f" | grep -vE '^\s*\{\s*0\s*\}' | grep -vE ',\s*"[^"]*"\s*\},?\s*$' || true)
	if [ -n "$bad_entry" ]; then
		echo "$label: found an entry with no name:"
		echo "$bad_entry" | sed 's/^/  /'
		return 1
	fi
	dup=$(extract "$f" | grep -oE 'MNT_[A-Z]+' | sort | uniq -d || true)
	if [ -n "$dup" ]; then
		echo "$label: found a flag used more than once in the table: $dup"
		return 1
	fi
	echo "$label: no unnamed entry, no duplicate flag"
	return 0
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
got="$w/got"

# First, prove the checks actually catch the bug: reinsert the exact
# line this patch removed into a scratch copy, right where it was.
# Anchored with $ so this only matches the real code line, not the
# same text quoted inside this patch's own AI-ONLY NOTE just below
# it (which ends each quoted line with a closing "'", not a bare
# comma).
sed 's/^\(\t{ MNT_UPDATE, "update" },\)$/\1\n\t{ MNT_DELEXPORT },\n\t{ MNT_UPDATE, "update" },/' \
    "$src" > "$w/broken.c"
if check_table "$w/broken.c" "T re-broken copy" >"$w/broken.out" 2>&1; then
	echo "pstat_mntflags.sh: the checks did NOT catch the re-inserted bug -- stopping" >&2
	cat "$w/broken.out" >&2
	exit 1
fi
cat "$w/broken.out" | tee -a "$got"

check_table "$src" "T real pstat.c" | tee -a "$got"

diff -u build/test/pstat_mntflags.want "$got"
