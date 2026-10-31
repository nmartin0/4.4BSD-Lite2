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
# build/test/db_elf_sanity.c (docs/provenance/smaller-fixes.md),
# which reproduces sys/ddb/db_elf.c's X_db_sym_init() symtab/strtab
# search loop standalone (that function is kernel code and can't be
# built or run on the host -- see db_elf_sanity.c's own note; the
# real regression proof is build/kernel.sh GENERIC.i386, recorded in
# the provenance doc). First greps the real db_elf.c to confirm the
# three added checks (e_shoff == 0, and each sh_offset == 0) are
# still there, so this standalone copy can't quietly drift from it.
#
#   sh build/test/db_elf_sanity.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/sys/ddb/db_elf.c

grep -qE 'if \(elf->e_shoff == 0\)' "$src" || {
	echo "db_elf_sanity.sh: $src doesn't check e_shoff == 0 -- the fix may be stale" >&2
	exit 1
}
n=$(grep -cE 'if \(shp\[[ij]\]\.sh_offset == 0\)' "$src")
[ "$n" = 2 ] || {
	echo "db_elf_sanity.sh: $src doesn't have exactly two 'sh_offset == 0' checks (found $n) -- the fix may be stale" >&2
	exit 1
}

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -std=gnu89 -w -g -o "$w/db_elf_sanity" build/test/db_elf_sanity.c
"$w/db_elf_sanity" > "$w/got"
cat "$w/got"
diff -u build/test/db_elf_sanity.want "$w/got"
