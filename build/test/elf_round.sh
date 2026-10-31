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
# build/test/elf_round.c (docs/provenance/smaller-fixes.md). Extracts
# just the `#define ELF_ROUND ...'/`#define ELF_TRUNC ...' lines from
# the real sys/kern/exec_elf.c into a scratch header, so the test
# exercises the exact macros as written there rather than a copy
# that can drift -- the rest of that file (struct proc, vnode, VM
# calls) isn't safe to pull onto the host at all. Fails outright, as
# a grep, if either #define isn't found where expected.
#
#   sh build/test/elf_round.sh
set -e
cd "$(dirname "$0")/../.."
src=usr/src/sys/kern/exec_elf.c

w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT

grep -E '^#define[[:space:]]+ELF_ROUND' "$src" > "$w/elf_round_macros.h" || {
	echo "elf_round.sh: ELF_ROUND's #define not found in $src -- the fix may be stale" >&2
	exit 1
}
grep -E '^#define[[:space:]]+ELF_TRUNC' "$src" >> "$w/elf_round_macros.h" || {
	echo "elf_round.sh: ELF_TRUNC's #define not found in $src -- the fix may be stale" >&2
	exit 1
}
grep -qE '^#define ELF_ALIGN' "$src" && {
	echo "elf_round.sh: $src still has the old ELF_ALIGN macro -- the fix may be stale" >&2
	exit 1
}
grep -qE '\*addr = ELF_ROUND\(\*addr, ph->p_align\);' "$src" || {
	echo "elf_round.sh: elf_load_psection() doesn't use ELF_ROUND for *addr -- the fix may be stale" >&2
	exit 1
}

gcc -std=gnu89 -w -g -I "$w" -o "$w/elf_round" build/test/elf_round.c
"$w/elf_round" > "$w/got"
cat "$w/got"
diff -u build/test/elf_round.want "$w/got"
