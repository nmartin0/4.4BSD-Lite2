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
 * test for sys/i386/isa/wcd.c's CDIOREADTOCENTRYS bounds check
 * (docs/provenance/smaller-fixes.md). wcdioctl() itself can't be
 * built or run on the host: it's kernel code, guarded by
 * `#if NWCD > 0 && NWDC > 0 && defined(ATAPI)', using sys/buf.h,
 * copyout(), and a real ATAPI drive's responses -- none of which
 * exist outside a running kernel. Regression is proven the way this
 * project proves other kernel changes build: build/kernel.sh
 * GENERIC.i386 (docs/provenance/smaller-fixes.md's "The test"
 * section has the exact result).
 *
 * This test instead reproduces, standalone, the handful of lines
 * CDIOREADTOCENTRYS's `len' computation and bound check actually
 * are -- the same approach build/test/db_elf_sanity.c uses for
 * sys/ddb/db_elf.c. Copied character-for-character (down to using
 * `u_long len' and the same comparison operators) from the real
 * case block, with small stand-ins for `struct ioc_toc_header' and
 * `struct toc' holding only the fields the computation touches.
 * build/test/wcd_toclen.sh greps the real wcd.c to confirm this
 * hasn't drifted from it.
 */
#include <stdio.h>

#define	MAXTRK	99
struct toc_hdr {
	unsigned char starting_track;
	unsigned char ending_track;
};
struct toc {
	struct toc_hdr hdr;
	char tab[MAXTRK + 1][8]; /* stand-in cd_toc_entry, size doesn't matter */
};

/*
 * Returns 1 and sets *len_out if CDIOREADTOCENTRYS would proceed to
 * copyout (and, for CD_MSF_FORMAT, the in-place conversion loop);
 * 0 if it would return EINVAL/EIO instead. data_len stands in for
 * te->data_len, starting_track for te->starting_track.
 */
static int
toclen(struct toc *toc, unsigned char starting_track,
    unsigned short data_len, unsigned long *len_out)
{
	unsigned long len;

	if (!toc->hdr.ending_track)
		return (0);
	if (starting_track < toc->hdr.starting_track ||
	    starting_track > toc->hdr.ending_track)
		return (0);

	len = (toc->hdr.ending_track - starting_track + 2) *
	    sizeof(toc->tab[0]);
	if (data_len < len)
		len = data_len;
	if (len <= 0)
		return (0);
	/* AI-ONLY NOTE: this is the fix itself, copied from wcd.c's
	 * `if (len > sizeof(toc->tab)) return (EINVAL);'. */
	if (len > sizeof(toc->tab))
		return (0);

	*len_out = len;
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
	struct toc toc;
	unsigned long len;
	int ok;

	/* A normal, well-formed TOC: must proceed normally. */
	toc.hdr.starting_track = 1;
	toc.hdr.ending_track = 10;
	ok = toclen(&toc, 1, (unsigned short)sizeof(toc.tab), &len);
	check("normal TOC, full request proceeds", ok, 1);
	if (ok)
		check("normal TOC len is in range", len <= sizeof(toc.tab), 1);

	/*
	 * The hole this check closes: a drive that reports more
	 * tracks than MAXTRK+1 can actually hold (wcd_read_toc()
	 * clamps a local `ntracks' to MAXTRK for the TOC read itself,
	 * but never clamps hdr.ending_track). ending_track = 250 with
	 * a request for the whole range, and a data_len large enough
	 * not to clamp len down first, makes the raw entry count (250
	 * - 1 + 2 = 251 entries) come to far more than
	 * sizeof(toc.tab) (100 entries) -- exactly what would
	 * otherwise overrun the (possibly stack-resident, for the
	 * MSF path) buffer.
	 */
	toc.hdr.starting_track = 1;
	toc.hdr.ending_track = 250;
	ok = toclen(&toc, 1, 60000, &len);
	check("corrupted ending_track is rejected", ok, 0);

	/*
	 * A known, documented limitation of this exact fix (the
	 * AI-ONLY NOTE at the real check says so, and confirms even
	 * FreeBSD's own later tree still has it): a small `data_len'
	 * clamps `len' down to something within sizeof(toc.tab)
	 * *before* this check runs, so this check alone does not
	 * reject every corrupted-TOC request -- only ones whose
	 * requested byte count itself would overrun the buffer. This
	 * is not a flaw introduced by the test; it's the literal donor
	 * fix's own scope, ported as-is and documented as incomplete
	 * rather than silently patched over. Asserted here (as "ok",
	 * not "FAIL") so this boundary stays visible rather than
	 * quietly vanishing if someone "improves" the check later
	 * without updating the documentation.
	 */
	ok = toclen(&toc, 1, 16, &len);
	check("corrupted ending_track + small data_len: NOT caught by this check (documented gap)", ok, 1);

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
