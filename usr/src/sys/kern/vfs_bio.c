/*-
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	from: @(#)vfs_bio.c	8.6 (Berkeley) 1/11/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/trace.h>
#include <sys/malloc.h>
#include <sys/resourcevar.h>

/*
 * Definitions for the buffer hash lists.
 */
#define	BUFHASH(dvp, lbn)	\
	(&bufhashtbl[((int)(dvp) / sizeof(*(dvp)) + (int)(lbn)) & bufhash])
LIST_HEAD(bufhashhdr, buf) *bufhashtbl, invalhash;
u_long	bufhash;

/*
 * Insq/Remq for the buffer hash lists.
 */
#define	binshash(bp, dp)	LIST_INSERT_HEAD(dp, bp, b_hash)
#define	bremhash(bp)		LIST_REMOVE(bp, b_hash)

/*
 * Definitions for the buffer free lists.
 */
#define	BQUEUES		4		/* number of free buffer queues */

#define	BQ_LOCKED	0		/* super-blocks &c */
#define	BQ_LRU		1		/* lru, useful buffers */
#define	BQ_AGE		2		/* rubbish */
#define	BQ_EMPTY	3		/* buffer headers with no memory */

TAILQ_HEAD(bqueues, buf) bufqueues[BQUEUES];
int needbuffer;

/*
 * Insq/Remq for the buffer free lists.
 */
#define	binsheadfree(bp, dp)	TAILQ_INSERT_HEAD(dp, bp, b_freelist)
#define	binstailfree(bp, dp)	TAILQ_INSERT_TAIL(dp, bp, b_freelist)

void
bremfree(bp)
	struct buf *bp;
{
	struct bqueues *dp = NULL;

	/*
	 * We only calculate the head of the freelist when removing
	 * the last element of the list as that is the only time that
	 * it is needed (e.g. to reset the tail pointer).
	 *
	 * NB: This makes an assumption about how tailq's are implemented.
	 */
	if (bp->b_freelist.tqe_next == NULL) {
		for (dp = bufqueues; dp < &bufqueues[BQUEUES]; dp++)
			if (dp->tqh_last == &bp->b_freelist.tqe_next)
				break;
		if (dp == &bufqueues[BQUEUES])
			panic("bremfree: lost tail");
	}
	TAILQ_REMOVE(dp, bp, b_freelist);
}

/*
 * Initialize buffers and hash links for buffers.
 */
void
bufinit()
{
	register struct buf *bp;
	struct bqueues *dp;
	register int i;
	int base, residual;

	for (dp = bufqueues; dp < &bufqueues[BQUEUES]; dp++)
		TAILQ_INIT(dp);
	bufhashtbl = hashinit(nbuf, M_CACHE, &bufhash);
	base = bufpages / nbuf;
	residual = bufpages % nbuf;
	for (i = 0; i < nbuf; i++) {
		bp = &buf[i];
		bzero((char *)bp, sizeof *bp);
		bp->b_dev = NODEV;
		bp->b_rcred = NOCRED;
		bp->b_wcred = NOCRED;
		bp->b_vnbufs.le_next = NOLIST;
		bp->b_data = buffers + i * MAXBSIZE;
		if (i < residual)
			bp->b_bufsize = (base + 1) * CLBYTES;
		else
			bp->b_bufsize = base * CLBYTES;
		bp->b_flags = B_INVAL;
		dp = bp->b_bufsize ? &bufqueues[BQ_AGE] : &bufqueues[BQ_EMPTY];
		binsheadfree(bp, dp);
		binshash(bp, &invalhash);
	}
}

bread(a1, a2, a3, a4, a5)
	struct vnode *a1;
	daddr_t a2;
	int a3;
	struct ucred *a4;
	struct buf **a5;
{

	/*
	 * Body deleted.
	 */
	return (EIO);
}

breadn(a1, a2, a3, a4, a5, a6, a7, a8)
	struct vnode *a1;
	daddr_t a2; int a3;
	daddr_t a4[]; int a5[];
	int a6;
	struct ucred *a7;
	struct buf **a8;
{

	/*
	 * Body deleted.
	 */
	return (EIO);
}

bwrite(a1)
	struct buf *a1;
{

	/*
	 * Body deleted.
	 */
	return (EIO);
}

int
vn_bwrite(ap)
	struct vop_bwrite_args *ap;
{
	return (bwrite(ap->a_bp));
}

bdwrite(a1)
	struct buf *a1;
{

	/*
	 * Body deleted.
	 */
	return;
}

bawrite(a1)
	struct buf *a1;
{

	/*
	 * Body deleted.
	 */
	return;
}

brelse(a1)
	struct buf *a1;
{

	/*
	 * Body deleted.
	 */
	return;
}

struct buf *
/*
 * AI-ONLY NOTE: written, not restored; see the note on
 * count_lock_queue below for the sources and the rule.
 *
 * The parameters are named here. The settlement left them a1 and a2,
 * which is how the stubs were generated; <sys/buf.h>'s prototype and
 * both donors call them vp and blkno.
 */
incore(vp, blkno)
	struct vnode *vp;
	daddr_t blkno;
{
	register struct buf *bp;

	for (bp = BUFHASH(vp, blkno)->lh_first; bp; bp = bp->b_hash.le_next)
		if (bp->b_lblkno == blkno && bp->b_vp == vp &&
		    (bp->b_flags & B_INVAL) == 0)
			return (bp);
	return (NULL);
}

struct buf *
getblk(a1, a2, a3, a4, a5)
	struct vnode *a1;
	daddr_t a2;
	int a3, a4, a5;
{

	/*
	 * Body deleted.
	 */
	return ((struct buf *)0);
}

struct buf *
/*
 * AI-ONLY NOTE: written, not restored; see count_lock_queue. This one
 * is 4.4BSD's text with nothing changed at all -- it touches neither
 * queue head directly, so there is no <sys/queue.h> conversion to
 * make.
 *
 * NetBSD's drops the MAXBSIZE panic and moves allocbuf above the
 * three zeroings. Neither is taken: the panic is Berkeley's guard on
 * a caller passing nonsense, and the order is his.
 */
geteblk(size)
	int size;
{
	register struct buf *bp;

	if (size > MAXBSIZE)
		panic("geteblk: size too big");
	while ((bp = getnewbuf(0, 0)) == NULL)
		/* void */;
	bp->b_flags |= B_INVAL;
	bremhash(bp);
	binshash(bp, &invalhash);
	bp->b_bcount = 0;
	bp->b_error = 0;
	bp->b_resid = 0;
	allocbuf(bp, size);
	return (bp);
}

/*
 * AI-ONLY NOTE: written, not restored; see count_lock_queue for the
 * rule and the sources. 4.4BSD's text, with the two <sys/queue.h>
 * field names this tree converted: `bufqueues[BQ_EMPTY].qe_next'
 * becomes `.tqh_first'.
 *
 * The parameter is tp, not bp, which is Berkeley's naming here and
 * reads oddly beside the other bodies -- the buffer being grown is
 * `tp' and the ones robbed of space are `bp'. NetBSD renamed it bp
 * and lost the distinction. Kept.
 *
 * pagemove() is machine-dependent and this port has it complete:
 * i386/i386/vm_machdep.c, walking the page tables with kvtopte. It is
 * not one of the settlement stubs.
 */
allocbuf(tp, size)
	register struct buf *tp;
	int size;
{
	register struct buf *bp, *ep;
	int sizealloc, take, s;

	sizealloc = roundup(size, CLBYTES);
	/*
	 * Buffer size does not change
	 */
	if (sizealloc == tp->b_bufsize)
		goto out;
	/*
	 * Buffer size is shrinking.
	 * Place excess space in a buffer header taken from the
	 * BQ_EMPTY buffer list and placed on the "most free" list.
	 * If no extra buffer headers are available, leave the
	 * extra space in the present buffer.
	 */
	if (sizealloc < tp->b_bufsize) {
		if ((ep = bufqueues[BQ_EMPTY].tqh_first) == NULL)
			goto out;
		s = splbio();
		bremfree(ep);
		ep->b_flags |= B_BUSY;
		splx(s);
		pagemove(tp->b_un.b_addr + sizealloc, ep->b_un.b_addr,
		    (int)tp->b_bufsize - sizealloc);
		ep->b_bufsize = tp->b_bufsize - sizealloc;
		tp->b_bufsize = sizealloc;
		ep->b_flags |= B_INVAL;
		ep->b_bcount = 0;
		brelse(ep);
		goto out;
	}
	/*
	 * More buffer space is needed. Get it out of buffers on
	 * the "most free" list, placing the empty headers on the
	 * BQ_EMPTY buffer header list.
	 */
	while (tp->b_bufsize < sizealloc) {
		take = sizealloc - tp->b_bufsize;
		while ((bp = getnewbuf(0, 0)) == NULL)
			/* void */;
		if (take >= bp->b_bufsize)
			take = bp->b_bufsize;
		pagemove(&bp->b_un.b_addr[bp->b_bufsize - take],
		    &tp->b_un.b_addr[tp->b_bufsize], take);
		tp->b_bufsize += take;
		bp->b_bufsize = bp->b_bufsize - take;
		if (bp->b_bcount > bp->b_bufsize)
			bp->b_bcount = bp->b_bufsize;
		if (bp->b_bufsize <= 0) {
			bremhash(bp);
			binshash(bp, &invalhash);
			bp->b_dev = NODEV;
			bp->b_error = 0;
			bp->b_flags |= B_INVAL;
		}
		brelse(bp);
	}
out:
	tp->b_bcount = size;
	return (1);
}

struct buf *
/*
 * AI-ONLY NOTE: written, not restored; see count_lock_queue.
 *
 * The queue scan is Berkeley's, from BQ_AGE downward and stopping
 * before BQ_LOCKED, which is `for (dp = &bufqueues[BQ_AGE]; dp >
 * bufqueues; dp--)' in the encumbered file. NetBSD writes the same
 * two queues out by name. Berkeley's loop is kept because this
 * file's BQUEUES ordering is what gives it meaning: BQ_LOCKED 0,
 * BQ_LRU 1, BQ_AGE 2, BQ_EMPTY 3, declared just above.
 *
 * The trace() call is restored. <sys/trace.h>:46 defines TR_BRELSE
 * and this file calls trace() nowhere, because the bodies that called
 * it were deleted; kern/vfs_cluster.c, ufs/ffs/ffs_inode.c and
 * ufs/ufs/ufs_bmap.c still do, so it is live idiom and the constant
 * exists for this call. NetBSD dropped it. Restoring it is this
 * tree's own intent, as B_EINTR was in biowait.
 *
 * Not taken from NetBSD: they also clear b_dev to NODEV and zero
 * b_blkno, b_lblkno and b_iodone here. Berkeley does not, and the
 * callers -- getblk and geteblk -- set those themselves before the
 * buffer is used. Adding the clearing would be an improvement of
 * theirs, not a restoration of this file.
 */
getnewbuf(slpflag, slptimeo)
	int slpflag, slptimeo;
{
	register struct buf *bp;
	register struct bqueues *dp;
	register struct ucred *cred;
	int s;

loop:
	s = splbio();
	for (dp = &bufqueues[BQ_AGE]; dp > bufqueues; dp--)
		if (dp->tqh_first)
			break;
	if (dp == bufqueues) {		/* no free blocks */
		needbuffer = 1;
		(void) tsleep((caddr_t)&needbuffer, slpflag | (PRIBIO + 1),
			"getnewbuf", slptimeo);
		splx(s);
		return (NULL);
	}
	bp = dp->tqh_first;
	bremfree(bp);
	bp->b_flags |= B_BUSY;
	splx(s);
	if (bp->b_flags & B_DELWRI) {
		(void) bawrite(bp);
		goto loop;
	}
	trace(TR_BRELSE, pack(bp->b_vp, bp->b_bufsize), bp->b_lblkno);
	if (bp->b_vp)
		brelvp(bp);
	if (bp->b_rcred != NOCRED) {
		cred = bp->b_rcred;
		bp->b_rcred = NOCRED;
		crfree(cred);
	}
	if (bp->b_wcred != NOCRED) {
		cred = bp->b_wcred;
		bp->b_wcred = NOCRED;
		crfree(cred);
	}
	bp->b_flags = B_BUSY;
	bp->b_dirtyoff = bp->b_dirtyend = 0;
	bp->b_validoff = bp->b_validend = 0;
	return (bp);
}

/*
 * AI-ONLY NOTE: written, not restored; see count_lock_queue.
 *
 * One choice is settled by this tree rather than by the donors: it
 * sleeps with tsleep, which kern/kern_synch.c:281 defines and which
 * this directory uses twenty-five times against sleep's three.
 * 4.4BSD's own used the bare sleep, the older form, and NetBSD moved
 * to tsleep for the same reason.
 *
 * It does NOT check B_EINTR, and an earlier revision of this body did
 * -- taking NetBSD's shape and calling it a restoration because
 * <sys/buf.h>:101 defines the flag. The flag is defined for bwrite,
 * not for this function. 4.4BSD encumbered's biowait reports B_ERROR
 * and nothing else, and its bwrite checks B_EINTR after calling
 * biowait, clearing it and overriding error with EINTR. Clearing it
 * here would take the flag away before the one function that reads
 * it, and bwrite is still a stub, so the two would have disagreed
 * silently once it was written.
 *
 * NetBSD moved the check inward and clears it here instead. That is
 * coherent in their tree because their bwrite was changed to match.
 * Half of it is not coherent in this one.
 */
biowait(bp)
	register struct buf *bp;
{
	int s;

	s = splbio();
	while ((bp->b_flags & B_DONE) == 0)
		sleep((caddr_t)bp, PRIBIO);
	splx(s);
	if ((bp->b_flags & B_ERROR) == 0)
		return (0);
	if (bp->b_error)
		return (bp->b_error);
	return (EIO);
}

void
biodone(a1)
	struct buf *a1;
{

	/*
	 * Body deleted.
	 */
	return;
}

int
/*
 * AI-ONLY NOTE: the body below is written, not restored. The AT&T
 * settlement removed it from both Lite releases -- see
 * docs/provenance/missing.md -- leaving the signature, the call
 * sites, the structures and the surrounding comments in place, so
 * what follows satisfies an interface this tree fully specifies.
 *
 * Algorithm from NetBSD 1.0's kern/vfs_bio.c, which is the only
 * descendant that kept this design: its header reads `@(#)vfs_bio.c
 * 8.6 (Berkeley) 1/11/94' with a 1994 Demetriou copyright on the new
 * bodies, and it keeps BQ_LOCKED, BQ_LRU, BQ_AGE and BQ_EMPTY with
 * the same four queues and the same hash. FreeBSD 2.0.5 replaced the
 * buffer cache outright and has no BQ_LRU at all. OpenBSD 1996 is
 * NetBSD's with two more years on it. 386BSD 0.1 and anything else
 * predating 4.4BSD-Lite is Net/2-derived and carries the bodies the
 * settlement removed, so it is disqualified by its earliness.
 *
 * Spelling from this file. NetBSD rewrote theirs around SET, CLR and
 * ISSET macros, fifty-one uses; 4.4BSD's own vfs_bio.c uses plain
 * `|=' and `&= ~' twenty-six times and this file's intact functions
 * do the same, so plain operators are used here. struct buf is
 * identical in the two trees, field for field, twenty-six each.
 *
 * 4.4BSD encumbered is read for the shape only, as XNU is -- never
 * copied. Where it differs from what is written here it is because
 * it predates this tree's own conversion to <sys/queue.h>: it walks
 * `bufqueues[BQ_LOCKED].qe_next' where this file declares
 * TAILQ_HEAD and its intact bremfree uses tqe_next and TAILQ_REMOVE.
 *
 * The rule, arrived at after three revisions of getting it wrong.
 * **Lite2 is 4.4BSD minus the bodies.** So the faithful
 * reconstruction of a body is 4.4BSD's exact text, changed only where
 * this tree demonstrably changed something -- and the only such
 * change in this file is the conversion from <sys/queue.h>'s
 * predecessors: `bufqueues[BQ_LOCKED].qe_next' with its cast becomes
 * `.tqh_first', `b_freelist.qe_next' becomes `.tqe_next',
 * `BUFHASH(..)->le_next' becomes `->lh_first', `b_hash.qe_next'
 * becomes `.le_next'. Lite2's intact bremfree proves that conversion;
 * nothing else here is proven.
 *
 * Everything else stays Berkeley's, including what looks like an
 * inconsistency. count_lock_queue writes `++ret' and `return(ret)'
 * where the rest of the file writes `x++' and `return ('. Earlier
 * revisions of this work changed them to the majority, on a rule that
 * "the file wins" -- which was invented to justify spellings already
 * taken from NetBSD, and is the opposite of fidelity. Berkeley wrote
 * that function that way; it is written that way.
 *
 * The same disposes of sleep. biowait sleeps with `sleep((caddr_t)bp,
 * PRIBIO)', which is Berkeley's line. An earlier revision made it
 * tsleep because this directory uses tsleep twenty-five times against
 * sleep's three -- the same invented rule. sleep is defined at
 * kern/kern_synch.c:411, prototyped at <sys/proc.h>:277 and called by
 * kern/vfs_subr.c:130 among others, so there is no reason to prefer
 * tsleep except that NetBSD does.
 */
count_lock_queue()
{
	register struct buf *bp;
	register int ret;

	for (ret = 0, bp = bufqueues[BQ_LOCKED].tqh_first;
	    bp; bp = bp->b_freelist.tqe_next)
		++ret;
	return(ret);
}

#ifdef DIAGNOSTIC
/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "syncprt"
 * in vfs_syscalls.c using sysctl.
 */
void
vfs_bufstats()
{
	int s, i, j, count;
	register struct buf *bp;
	register struct bqueues *dp;
	int counts[MAXBSIZE/CLBYTES+1];
	static char *bname[BQUEUES] = { "LOCKED", "LRU", "AGE", "EMPTY" };

	for (dp = bufqueues, i = 0; dp < &bufqueues[BQUEUES]; dp++, i++) {
		count = 0;
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			counts[j] = 0;
		s = splbio();
		for (bp = dp->tqh_first; bp; bp = bp->b_freelist.tqe_next) {
			counts[bp->b_bufsize/CLBYTES]++;
			count++;
		}
		splx(s);
		printf("%s: total-%d", bname[i], count);
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			if (counts[j] != 0)
				printf(", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}
#endif /* DIAGNOSTIC */
