/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own tests is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * Nothing here is copied from another system. AI-ONLY NOTE: a host
 * test for sys/kern/exec_elf.c's ELF_ROUND/ELF_TRUNC split
 * (docs/provenance/smaller-fixes.md). Unlike the kernel files this
 * patch also touches, the macros themselves are pure integer
 * arithmetic with no kernel dependency at all, so this test
 * #includes the real file directly and exercises the macros as
 * themselves -- not a copy that can drift -- rather than reproducing
 * them standalone. elf_load_psection() the function is not called
 * (it needs a real struct proc/vnode/Elf32_Phdr and VM calls); only
 * the macros above it are used.
 */
#include <stdio.h>

/* Only the two macros are wanted; nothing past them is safe to
 * compile on the host (struct proc, vnode, VM calls). sed, not
 * #include, pulls just the `#define ELF_ROUND ...'/`#define
 * ELF_TRUNC ...' lines out of the real file -- see the .sh wrapper,
 * which also greps the real file to confirm these two lines are
 * still there unchanged, so this extraction can't quietly drift
 * from the source it's lifted from.
 */
#include "elf_round_macros.h"

static int fails;

static void
check(const char *label, unsigned long got, unsigned long want)
{
	printf("T %s: got %lu, want %lu -- %s\n", label, got, want,
	    got == want ? "ok" : "FAIL");
	if (got != want)
		fails = 1;
}

int
main(void)
{
	/* ELF_TRUNC: round down. An already-aligned value is unchanged. */
	check("ELF_TRUNC(0x1000, 0x1000) unchanged", ELF_TRUNC(0x1000, 0x1000), 0x1000);
	check("ELF_TRUNC(0x1001, 0x1000) truncates", ELF_TRUNC(0x1001, 0x1000), 0x1000);
	check("ELF_TRUNC(0x1fff, 0x1000) truncates", ELF_TRUNC(0x1fff, 0x1000), 0x1000);
	check("ELF_TRUNC(0, 0x1000) stays 0", ELF_TRUNC(0, 0x1000), 0);

	/*
	 * ELF_ROUND: round up. This is the actual fix: the pre-fix
	 * code used ELF_ALIGN(a + b, b), identical in body to
	 * ELF_TRUNC(a, b), on a value it meant to round UP -- which
	 * overshoots by a full `b' even when `a' was already aligned.
	 * ELF_ROUND must leave an aligned value unchanged, unlike that
	 * old expression.
	 */
	check("ELF_ROUND(0x1000, 0x1000) unchanged (the actual bug)", ELF_ROUND(0x1000, 0x1000), 0x1000);
	check("ELF_ROUND(0x1001, 0x1000) rounds up", ELF_ROUND(0x1001, 0x1000), 0x2000);
	check("ELF_ROUND(0x1fff, 0x1000) rounds up", ELF_ROUND(0x1fff, 0x1000), 0x2000);
	check("ELF_ROUND(0, 0x1000) stays 0", ELF_ROUND(0, 0x1000), 0);

	/*
	 * The pre-fix expression, spelled out directly (not via a
	 * macro, since the old ELF_ALIGN is gone from the real file
	 * now): confirms it really did overshoot an aligned address by
	 * a full page, which is what this patch's AI-ONLY note claims.
	 */
	{
		unsigned long a = 0x1000, b = 0x1000;
		unsigned long old_result = (a + b) & ~(b - 1);
		check("old ELF_ALIGN(a+b,b) overshoots an aligned a",
		    old_result, 0x2000);
		check("ELF_ROUND gives the address actually wanted",
		    ELF_ROUND(a, b), a);
	}

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
