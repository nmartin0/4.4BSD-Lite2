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
 * test for sys/ddb/db_elf.c's X_db_sym_init() sanity checks
 * (docs/provenance/smaller-fixes.md). X_db_sym_init() itself can't
 * be built or run on the host: it's kernel code (sys/systm.h,
 * sys/proc.h, machine/db_machdep.h, db_printf(), db_add_symbol_table()
 * -- none of which exist, or mean the same thing, outside a running
 * i386 kernel. Regression is instead proven the way this project
 * proves other kernel changes build: build/kernel.sh GENERIC.i386
 * (docs/provenance/smaller-fixes.md's own "The test" section has the
 * exact result).
 *
 * This test instead reproduces, standalone, the exact few lines of
 * the symbol/string-table-finding loop this patch changes -- the
 * same approach build/test/sa_argv.c and the eqp/ap half of
 * build/test/at_fixes.c use for code that can't be pulled in by
 * #include. The loop body below is copied character-for-character
 * from X_db_sym_init() (down to the variable names and lack of
 * braces around single-statement `if's, matching this tree's own
 * K&R style), with a tiny stand-in `struct shdr' in place of the
 * real cross-compiled `Elf32_Shdr' -- only the four fields the loop
 * actually touches (sh_type, sh_offset, sh_size, sh_link), since the
 * bug and its fix are about those four fields' values, not about
 * ELF's own on-disk layout. build/test/db_elf_sanity.sh greps the
 * real db_elf.c to confirm this copy hasn't drifted from it.
 */
#include <stdio.h>
#include <string.h>

#define	SHT_SYMTAB	2	/* stand-in for this tree's Elf_sht_symtab */
#define	SHT_NULL	0

struct shdr {
	int sh_type;
	long sh_offset;
	long sh_size;
	int sh_link;
};

/*
 * Returns 1 if a valid symtab_start/strtab_start pair was found (the
 * real function's "badheader" is never reached in that case), 0 if
 * none was found (the real function's loop falls through with both
 * still NULL, which its own sanity check just below -- unchanged by
 * this patch -- turns into "goto badheader").
 *
 * e_shoff_zero lets the caller choose whether e_shoff is itself zero,
 * exercising this patch's first added check directly.
 */
static int
find_symtab(struct shdr *shp, int shnum, int e_shoff_zero,
    long *symtab_off_out, long *strtab_off_out)
{
	long symtab_off = -1, strtab_off = -1;
	int i, j;

	/*
	 * AI-ONLY NOTE: this `if' is the fix itself, copied from
	 * db_elf.c's `if (elf->e_shoff == 0) goto badheader;' -- here,
	 * "goto badheader" is simply "report not found", since this
	 * standalone copy has no header/symbol table to link.
	 */
	if (e_shoff_zero)
		return (0);

	/* Copied verbatim in shape from X_db_sym_init()'s own loop. */
	for (i = 0; i < shnum; i++) {
		if (shp[i].sh_type == SHT_SYMTAB) {
			if (shp[i].sh_offset == 0)
				continue;
			/* Got the symbol table. */
			symtab_off = shp[i].sh_offset;
			/* Find the string table to go with it. */
			j = shp[i].sh_link;
			if (shp[j].sh_offset == 0)
				continue;
			strtab_off = shp[j].sh_offset;
			/* There should only be one symbol table. */
			break;
		}
	}

	if (symtab_off == -1 || strtab_off == -1)
		return (0);
	*symtab_off_out = symtab_off;
	*strtab_off_out = strtab_off;
	return (1);
}

static int fails;

static void
check(const char *label, int got, int want)
{
	printf("T %s: got %d, want %d -- %s\n", label, got, want,
	    got == want ? "ok" : "FAIL");
	if (got != want)
		fails = 1;
}

int
main(void)
{
	long symoff, stroff;
	int found;

	/* A normal, fully-loaded table: must be found. */
	{
		struct shdr shp[2];
		shp[0].sh_type = SHT_SYMTAB;
		shp[0].sh_offset = 0x1000;
		shp[0].sh_size = 0x40;
		shp[0].sh_link = 1;
		shp[1].sh_type = SHT_NULL; /* stand-in strtab section */
		shp[1].sh_offset = 0x1040;
		shp[1].sh_size = 0x80;
		shp[1].sh_link = 0;

		found = find_symtab(shp, 2, 0, &symoff, &stroff);
		check("normal table found", found, 1);
		if (found) {
			check("normal table symtab offset", (int)symoff, 0x1000);
			check("normal table strtab offset", (int)stroff, 0x1040);
		}
	}

	/* Hole 1: e_shoff itself is zero -- the bootloader never
	 * loaded any section headers at all. */
	{
		struct shdr shp[2];
		memset(shp, 0, sizeof(shp));
		found = find_symtab(shp, 2, /* e_shoff_zero */ 1, &symoff, &stroff);
		check("e_shoff == 0 is rejected", found, 0);
	}

	/* Hole 2a: a real SHT_SYMTAB section header is present, but its
	 * own sh_offset is zero -- the bootloader claims the section
	 * exists but never actually loaded its data. Pre-fix, this was
	 * read as "symtab starts at symtab+0", i.e. the ELF header
	 * itself; post-fix, it's skipped. */
	{
		struct shdr shp[2];
		shp[0].sh_type = SHT_SYMTAB;
		shp[0].sh_offset = 0;		/* the bug */
		shp[0].sh_size = 0x40;
		shp[0].sh_link = 1;
		shp[1].sh_type = SHT_NULL;
		shp[1].sh_offset = 0x1040;
		shp[1].sh_size = 0x80;
		shp[1].sh_link = 0;

		found = find_symtab(shp, 2, 0, &symoff, &stroff);
		check("symtab sh_offset == 0 is rejected", found, 0);
	}

	/* Hole 2b: the symbol table itself is fine, but the string
	 * table its sh_link names has sh_offset zero. */
	{
		struct shdr shp[2];
		shp[0].sh_type = SHT_SYMTAB;
		shp[0].sh_offset = 0x1000;
		shp[0].sh_size = 0x40;
		shp[0].sh_link = 1;
		shp[1].sh_type = SHT_NULL;
		shp[1].sh_offset = 0;		/* the bug */
		shp[1].sh_size = 0x80;
		shp[1].sh_link = 0;

		found = find_symtab(shp, 2, 0, &symoff, &stroff);
		check("strtab sh_offset == 0 is rejected", found, 0);
	}

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
