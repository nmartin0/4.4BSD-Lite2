/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)tty_subr.c	8.2 (Berkeley) 9/5/93
 */

/*
 * AI-ONLY NOTE: the bodies below, lost in the AT&T settlement.
 *
 * This is deliberately an interim state, and is meant to be replaced
 * function by function. docs/provenance/clist.md records the whole
 * search: no post-settlement BSD restored CSRG's clist, every one
 * wrote its own, so taking any of them is a transplant -- kind D in
 * docs/provenance/imports.md's terms -- and not a restoration. The
 * canonical answer for a settlement body is to write it, which is how
 * execve and ten of vfs_bio.c's fourteen were done.
 *
 * It is here first because it works, and because a working clist is
 * the only way to check a written one. With these bodies the kernel
 * reaches a shell and the shell's own output arrives on the console:
 *
 *	/etc/rc: Can't open /etc/rc
 *
 * which is sh failing to find its startup file and saying so. Nothing
 * in this tree could print that before. Each body written to replace
 * one of these can now be checked against a kernel that works, rather
 * than only read.
 *
 * What is here, and from where:
 *
 *	getc q_to_b ndflush putc b_to_q nextc unputc catq
 *	cblock_alloc cblock_free	FreeBSD 2.0, David Greenman
 *	clist_init			4.4BSD's shape: this port
 *					allocates its cblocks statically,
 *					i386/machdep.c:216, so they are
 *					threaded onto cfreelist rather
 *					than malloc'd
 *	ndqb				written to 4.4BSD's shape, which
 *					FreeBSD 2.0 does not have
 *
 * And what was dropped from FreeBSD's file: cblock_alloc_cblocks and
 * cblock_free_cblocks, which grow and shrink a malloc'd pool and are
 * dead against a static one, and with them the only uses of M_TTYS, a
 * malloc type this tree does not have; INITIAL_CBLOCKS and
 * <sys/malloc.h> with them; and print_nblocks under an unconditional
 * `#define MBUF_DIAG', a debugging printf nothing calls.
 *
 * One cast was needed: FreeBSD's clist.h declares c_info as unsigned
 * char where 4.4BSD's declares it char, so a pointer subtraction in
 * unputc takes a cast rather than this tree's header being changed.
 *
 * Parameter names are FreeBSD's, as kern/exec_elf.c keeps NetBSD's,
 * so that imported text reads as imported.
 */

/*
 * Copyright (C) 1994, David Greenman. This software may be used, modified,
 *   copied, distributed, and sold, in both source and binary form provided
 *   that the above copyright and these terms are retained. Under no
 *   circumstances is the author responsible for the proper functioning
 *   of this software, nor does the author assume any responsibility
 *   for damages incurred with its use.
 *
 * $Id: tty_subr.c,v 1.7 1994/09/25 19:33:50 phk Exp $
 */

#include <sys/param.h>
#include <sys/systm.h>		/* min(), from libkern */
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/clist.h>

char cwaiting;
struct cblock *cfree, *cfreelist;
int cfreecount, nclist;

cblock_alloc()
{
	struct cblock *cblockp;

	cblockp = cfreelist;
	if (!cblockp) {
		/* XXX should syslog a message that we're out! */
		return (0);
	}
	cfreelist = cblockp->c_next;
	cblockp->c_next = NULL;
	cfreecount -= CBSIZE;
	return (cblockp);
}

cblock_free(cblockp)
	struct cblock *cblockp;
{
	cblockp->c_next = cfreelist;
	cfreelist = cblockp;
	cfreecount += CBSIZE;
	return;
}

/*
 * Called from init_main.c
 *
 * AI-ONLY NOTE: written from <sys/clist.h> and the allocation this
 * port already does, replacing a version transcribed from 4.4BSD's
 * encumbered cinit. That one was identical to its original but for
 * the function name and one space, which is not a reconstruction.
 *
 * Everything needed is declared: clist.h gives struct cblock with its
 * c_next link, and cfree, cfreelist, cfreecount and nclist;
 * i386/machdep.c:216 does valloc(cfree, struct cblock, nclist) and
 * conf/param.c:83 sets nclist. So the array exists at boot and the
 * only work is to put every element on the free list, counting the
 * bytes each carries. CROUND aligns the start, because valloc hands
 * back whatever address the running allocation reached.
 */
void
clist_init()
{
	struct cblock *cp, *limit;

	cp = (struct cblock *)(((int)cfree + CROUND) & ~CROUND);
	limit = cfree + nclist;

	cfreelist = NULL;
	cfreecount = 0;
	while (cp < limit) {
		cp->c_next = cfreelist;
		cfreelist = cp;
		cfreecount += CBSIZE;
		cp++;
	}
}

/*
 * Get a character from the head of a clist.
 */
int
getc(clistp)
	struct clist *clistp;
{
	int chr = -1;
	int s;
	struct cblock *cblockp;

	s = spltty();

	/* If there are characters in the list, get one */
	if (clistp->c_cc) {
		cblockp = (struct cblock *)((long)clistp->c_cf & ~CROUND);
		chr = (u_char)*clistp->c_cf;

		/*
		 * If this char is quoted, set the flag.
		 */
		if (isset(cblockp->c_quote, clistp->c_cf - (char *)cblockp->c_info))
			chr |= TTY_QUOTE;

		/*
		 * Advance to next character.
		 */
		clistp->c_cf++;
		clistp->c_cc--;
		/*
		 * If we have advanced the 'first' character pointer
		 * past the end of this cblock, advance to the next one.
		 * If there are no more characters, set the first and
		 * last pointers to NULL. In either case, free the
		 * current cblock.
		 */
		if ((clistp->c_cf >= (char *)(cblockp+1)) || (clistp->c_cc == 0)) {
			if (clistp->c_cc > 0) {
				clistp->c_cf = cblockp->c_next->c_info;
			} else {
				clistp->c_cf = clistp->c_cl = NULL;
			}
			cblock_free(cblockp);
		}
	}

	splx(s);
	return (chr);
}

/*
 * Copy 'amount' of chars, beginning at head of clist 'clistp' to
 * destination linear buffer 'dest'. Return number of characters
 * actually copied.
 */
int
q_to_b(clistp, dest, amount)
	struct clist *clistp;
	char *dest;
	int amount;
{
	struct cblock *cblockp;
	struct cblock *cblockn;
	char *dest_orig = dest;
	int numc;
	int s;

	s = spltty();

	while (clistp && amount && (clistp->c_cc > 0)) {
		cblockp = (struct cblock *)((long)clistp->c_cf & ~CROUND);
		cblockn = cblockp + 1; /* pointer arithmetic! */
		numc = min(amount, (char *)cblockn - clistp->c_cf);
		numc = min(numc, clistp->c_cc);
		bcopy(clistp->c_cf, dest, numc);
		amount -= numc;
		clistp->c_cf += numc;
		clistp->c_cc -= numc;
		dest += numc;
		/*
		 * If this cblock has been emptied, advance to the next
		 * one. If there are no more characters, set the first
		 * and last pointer to NULL. In either case, free the
		 * current cblock.
		 */
		if ((clistp->c_cf >= (char *)cblockn) || (clistp->c_cc == 0)) {
			if (clistp->c_cc > 0) {
				clistp->c_cf = cblockp->c_next->c_info;
			} else {
				clistp->c_cf = clistp->c_cl = NULL;
			}
			cblock_free(cblockp);
		}
	}

	splx(s);
	return (dest - dest_orig);
}

/*
 * AI-ONLY NOTE: stubbed again, deliberately.
 *
 * The body here was transcribed from 4.4BSD's encumbered ndqb --
 * identical but for renaming q to clistp, dropping register, and
 * merging two pairs of statements. FreeBSD 2.0 dropped this function,
 * so there is no permissive donor for it, and writing one honestly
 * means working from <sys/clist.h> and the callers rather than from
 * the original.
 *
 * Nothing on this port calls it: only news3400/bm/bmcons.c and
 * news3400/iop/rs.c do, and neither is in any i386 configuration. So
 * it returns zero until it is written, which is what it did before
 * the transcription and costs this port nothing.
 */
ndqb(clistp, flag)
	struct clist *clistp;
	int flag;
{

	/*
	 * Body deleted.
	 */
	return (0);
}

/*
 * Flush 'amount' of chars, beginning at head of clist 'clistp'.
 */
void
ndflush(clistp, amount)
	struct clist *clistp;
	int amount;
{
	struct cblock *cblockp;
	struct cblock *cblockn;
	int numc;
	int s;

	s = spltty();

	while (amount && (clistp->c_cc > 0)) {
		cblockp = (struct cblock *)((long)clistp->c_cf & ~CROUND);
		cblockn = cblockp + 1; /* pointer arithmetic! */
		numc = min(amount, (char *)cblockn - clistp->c_cf);
		numc = min(numc, clistp->c_cc);
		amount -= numc;
		clistp->c_cf += numc;
		clistp->c_cc -= numc;
		/*
		 * If this cblock has been emptied, advance to the next
		 * one. If there are no more characters, set the first
		 * and last pointer to NULL. In either case, free the
		 * current cblock.
		 */
		if ((clistp->c_cf >= (char *)cblockn) || (clistp->c_cc == 0)) {
			if (clistp->c_cc > 0) {
				clistp->c_cf = cblockp->c_next->c_info;
			} else {
				clistp->c_cf = clistp->c_cl = NULL;
			}
			cblock_free(cblockp);
		}
	}

	splx(s);
	return;
}

/*
 * Add a character to the end of a clist. Return -1 is no
 * more clists, or 0 for success.
 */
int
putc(chr, clistp)
	int chr;
	struct clist *clistp;
{
	struct cblock *cblockp;
	int s;

	s = spltty();

	cblockp = (struct cblock *)((long)clistp->c_cl & ~CROUND);

	if (clistp->c_cl == NULL) {
		cblockp = cblock_alloc();
		if (cblockp) {
			clistp->c_cf = clistp->c_cl = cblockp->c_info;
			clistp->c_cc = 0;
		} else {
			splx(s);
			return (-1);
		}
	} else {
		if (((long)clistp->c_cl & CROUND) == 0) {
			struct cblock *prev = (cblockp - 1);
			cblockp = cblock_alloc();
			if (cblockp) {
				prev->c_next = cblockp;
				clistp->c_cl = cblockp->c_info;
			} else {
				splx(s);
				return (-1);
			}
		}
	}

	/*
	 * If this character is quoted, set the quote bit, if not, clear it.
	 */
	if (chr & TTY_QUOTE)
		setbit(cblockp->c_quote, clistp->c_cl - (char *)cblockp->c_info);
	else
		clrbit(cblockp->c_quote, clistp->c_cl - (char *)cblockp->c_info);

	*clistp->c_cl++ = chr;
	clistp->c_cc++;

	splx(s);
	return (0);
}

/*
 * Copy data from linear buffer to clist chain. Return the
 * number of characters not copied.
 */
int
b_to_q(src, amount, clistp)
	char *src;
	int amount;
	struct clist *clistp;
{
	struct cblock *cblockp;
	char *firstbyte, *lastbyte;
	u_char startmask, endmask;
	int startbit, endbit, num_between, numc;
	int s;

	s = spltty();

	/*
	 * If there are no cblocks assigned to this clist yet,
	 * then get one.
	 */
	if (clistp->c_cl == NULL) {
		cblockp = cblock_alloc();
		if (cblockp) {
			clistp->c_cf = clistp->c_cl = cblockp->c_info;
			clistp->c_cc = 0;
		} else {
			splx(s);
			return (amount);
		}
	} else {
		cblockp = (struct cblock *)((long)clistp->c_cl & ~CROUND);
	}

	while (amount) {
		/*
		 * Get another cblock if needed.
		 */
		if (((long)clistp->c_cl & CROUND) == 0) {
			struct cblock *prev = cblockp - 1;
			cblockp = cblock_alloc();
			if (cblockp) {
				prev->c_next = cblockp;
				clistp->c_cl = cblockp->c_info;
			} else {
				splx(s);
				return (amount);
			}
		}

		/*
		 * Copy a chunk of the linear buffer up to the end
		 * of this cblock.
		 */
		numc = min(amount, (char *)(cblockp + 1) - clistp->c_cl);
		bcopy(src, clistp->c_cl, numc);
		
		/*
		 * Clear quote bits. The following could probably be made into
		 * a seperate "bitzero()" routine, but why bother?
		 */
		startbit = clistp->c_cl - (char *)cblockp->c_info;
		endbit = startbit + numc - 1;

		firstbyte = (u_char *)cblockp->c_quote + (startbit / NBBY);
		lastbyte = (u_char *)cblockp->c_quote + (endbit / NBBY);

		/*
		 * Calculate mask of bits to preserve in first and
		 * last bytes.
		 */
		startmask = NBBY - (startbit % NBBY);
		startmask = 0xff >> startmask;
		endmask = (endbit % NBBY);
		endmask = 0xff << (endmask + 1);

		if (firstbyte != lastbyte) {
			*firstbyte &= startmask;
			*lastbyte &= endmask;

			num_between = lastbyte - firstbyte - 1;
			if (num_between)
				bzero(firstbyte + 1, num_between);
		} else {
			*firstbyte &= (startmask | endmask);
		}

		/*
		 * ...and update pointer for the next chunk.
		 */
		src += numc;
		clistp->c_cl += numc;
		clistp->c_cc += numc;
		amount -= numc;
		/*
		 * If we go through the loop again, it's always
		 * for data in the next cblock, so by adding one (cblock),
		 * (which makes the pointer 1 beyond the end of this
		 * cblock) we prepare for the assignment of 'prev'
		 * above.
		 */
		cblockp += 1;

	}

	splx(s);
	return (amount);
}

/*
 * Get the next character in the clist. Store it at dst. Don't
 * advance any clist pointers, but return a pointer to the next
 * character position.
 */
char *
nextc(clistp, cp, dst)
	struct clist *clistp;
	char *cp;
	int *dst;
{
	struct cblock *cblockp;

	++cp;
	/*
	 * See if the next character is beyond the end of
	 * the clist.
	 */
	if (clistp->c_cc && (cp != clistp->c_cl)) {
		/*
		 * If the next character is beyond the end of this
		 * cblock, advance to the next cblock.
		 */
		if (((long)cp & CROUND) == 0)
			cp = ((struct cblock *)cp - 1)->c_next->c_info;
		cblockp = (struct cblock *)((long)cp & ~CROUND);

		/*
		 * Get the character. Set the quote flag if this character
		 * is quoted.
		 */
		*dst = (u_char)*cp | (isset(cblockp->c_quote, cp - (char *)cblockp->c_info) ? TTY_QUOTE : 0);

		return (cp);
	}

	return (NULL);
}

/*
 * "Unput" a character from a clist.
 */
int
unputc(clistp)
	struct clist *clistp;
{
	struct cblock *cblockp = 0, *cbp = 0;
	int s;
	int chr = -1;


	s = spltty();

	if (clistp->c_cc) {
		--clistp->c_cc;
		--clistp->c_cl;

		chr = (u_char)*clistp->c_cl;

		cblockp = (struct cblock *)((long)clistp->c_cl & ~CROUND);

		/*
		 * Set quote flag if this character was quoted.	
		 */
		if (isset(cblockp->c_quote, (char *)clistp->c_cl - cblockp->c_info))
			chr |= TTY_QUOTE;

		/*
		 * If all of the characters have been unput in this
		 * cblock, then find the previous one and free this
		 * one.
		 */
		if (clistp->c_cc && (clistp->c_cl <= (char *)cblockp->c_info)) {
			cbp = (struct cblock *)((long)clistp->c_cf & ~CROUND);

			while (cbp->c_next != cblockp)
				cbp = cbp->c_next;

			/*
			 * When the previous cblock is at the end, the 'last'
			 * pointer always points (invalidly) one past.
			 */
			clistp->c_cl = (char *)(cbp+1);
			cblock_free(cblockp);
			cbp->c_next = NULL;
		}
	}

	/*
	 * If there are no more characters on the list, then
	 * free the last cblock.
	 */
	if ((clistp->c_cc == 0) && clistp->c_cl) {
		cblockp = (struct cblock *)((long)clistp->c_cl & ~CROUND);
		cblock_free(cblockp);
		clistp->c_cf = clistp->c_cl = NULL;
	}

	splx(s);
	return (chr);
}

/*
 * Move characters in source clist to destination clist,
 * preserving quote bits.
 */
void
catq(src_clistp, dest_clistp)
	struct clist *src_clistp, *dest_clistp;
{
	int chr, s;

	s = spltty();
	/*
	 * If the destination clist is empty (has no cblocks atttached),
	 * then we simply assign the current clist to the destination.
	 */
	if (!dest_clistp->c_cf) {
		dest_clistp->c_cf = src_clistp->c_cf;
		dest_clistp->c_cl = src_clistp->c_cl;
		src_clistp->c_cf = src_clistp->c_cl = NULL;

		dest_clistp->c_cc = src_clistp->c_cc;
		src_clistp->c_cc = 0;

		splx(s);
		return;
	}
	splx(s);

	/*
	 * XXX  This should probably be optimized to more than one
	 * character at a time.
	 */
	while ((chr = getc(src_clistp)) != -1)
		putc(chr, dest_clistp);

	return;
}
