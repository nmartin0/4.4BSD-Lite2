# The clist routines: every precedent, read

`kern/tty_subr.c` is ten of the thirty-five settlement bodies, and
they are the queue the whole terminal driver runs on. Nothing a user
process writes can reach a port until they exist:

	sh writes fd 2
	  -> cnwrite -> comwrite -> ttwrite
	      ttwrite:   i = b_to_q(cp, ce, &tp->t_outq);
	  -> ttstart -> comstart
	      comstart:  c = getc(&tp->t_outq);

Both ends are `Body deleted'. This records the search for a donor, in
`precedent.md`'s order, done before writing anything.

## Step 1 — this tree's own ports

Nothing. `tty_subr.c` is machine-independent, so there are no port
variants, and the one other clist-shaped file in the tree --
`hp/dev/hil_subr.c`, Utah's `hilq_to_b` for the HIL queue -- is itself
a single stub.

## Step 2 — 4.4BSD-Lite

Identical to Lite2: 159 lines, the same ten stubs. So this is the
settlement baseline and not a Lite2 change.

## Step 3 — the contemporaries

All four have a complete file. **Not one of them restored CSRG's.**

| tree | lines | copyright |
|---|---|---|
| NetBSD 1.0 | 547 | Theo de Raadt, 1993, 1994 |
| NetBSD 1.1 | 549 | Theo de Raadt, 1993, 1994 |
| OpenBSD 1996 | 578 | Theo de Raadt, 1993, 1994 |
| FreeBSD 2.0.5 | 635 | David Greenman, 1994 |
| Lites 1.1.u3 | 353 | Carnegie Mellon, 1992 |

This is not the `vfs_bio.c` situation, where NetBSD 1.0's file was
Lite's own with the holes filled and could be used line for line. Here
every descendant wrote a new one.

## Lites fits the structure and still cannot be used

CMU's is the one worth examining closely, because unlike the others it
is written against **this tree's own three fields**:

	CMU fields           c_cc c_cf c_cl
	cblock references    24

and its `clist.h` is byte for byte ours -- the same `struct cblock`
with `c_next`, `c_quote[CBQSIZE]` and `c_info[CBSIZE]`, the same
`cfree`, `cfreelist`, `cfreecount` and `nclist`. That follows: Lites
is a 4.4BSD-Lite derivative and kept Lite's header. Its `getc` walks
the chain the way ours must:

	c = *(clp->c_cf++) & 0xff;
	clp->c_cc--;
	clcheck(clp);

**But it never touches `c_quote`.** The whole file has zero references
to quoting, so the array Lite's header reserves sits unused. 4.4BSD's
`getc` is

	if (isquote(p->c_cf))
		c |= TTY_QUOTE;

and `kern/tty.c` in this tree uses `TTY_QUOTE` five times -- for the
literal-next character, and for parity marking in the canonical line
discipline. Taking CMU's file would compile, link, and quietly lose
the quote bit.

So it is not a candidate after all. An earlier version of this file
called it the one candidate found, on its licence and its function
names, before its handling of quoting had been read.

## And the other rewrites bring their own data structure

That is what rules them out as donors rather than merely making them
less convenient:

| tree | fields of `struct clist` |
|---|---|
| 4.4BSD, Lite, Lite2 | `c_cc c_cf c_cl` |
| NetBSD, OpenBSD | `c_cc c_ce c_cf c_cl c_cn c_cq c_cs` |
| FreeBSD 2.0.5 | the three, plus `c_quote c_cbcount c_cblocks c_cbmax c_cbreserved c_clistp` |

de Raadt's is a ring buffer with a separate quote bitmap; Greenman's
keeps cblock reservation accounting. Taking either means replacing the
structure in `sys/tty.h`, and **twenty-two files in this tree touch
`c_cf`, `c_cl` or `c_cc` directly** -- `kern/tty.c`, `tty_pty.c`,
`tty_compat.c`, `net/if_sl.c`, and the serial and console drivers of
every port: hp300's `dca.c` and `dcm.c`, luna68k's `sio.c`, `kbd.c`
and `bmc.c`, pmax's `dc.c` and `scc.c`, sparc's `zs.c` and `cons.c`,
news3400's `rs.c` and `bmcons.c`, tahoe's `vx.c` and `mp.c`, and the
i386's `com.c` and `pccons.c`.

No two contemporaries even agree on what to replace it with.

## What this tree still has

Everything except the ten functions. `sys/clist.h` is intact:

	struct cblock {
		struct cblock *c_next;		/* next cblock in queue */
		char c_quote[CBQSIZE];		/* quoted characters */
		char c_info[CBSIZE];		/* characters */
	};
	extern	struct cblock *cfree, *cfreelist;
	extern	int cfreecount, nclist;

and `sys/tty.h` has the three-field `struct clist` the whole tree is
written against. The cblock chain, the free list, the counts and the
quote array are all present. Only the code that walks them was cut.

## So the method is the one already proven

The same as the ten `vfs_bio.c` bodies: write them against this tree's
own structure, with 4.4BSD's encumbered `kern/tty_subr.c` read for
shape as `precedent.md` allows, and the rewrites readable for
algorithm but not for structure.

Each donor fails for its own reason, and they were each read rather
than ruled out by their headers:

| donor | structure | quoting | verdict |
|---|---|---|---|
| NetBSD, OpenBSD | MALLOC'd flat buffer, separate bitmap | yes, its own way | needs a new `struct clist` |
| 386BSD 0.1 | a ring, no clist at all | none | different everything |
| Lites, CMU | ours exactly | **none at all** | would lose `TTY_QUOTE` |
| **FreeBSD 2.0.5** | **ours, plus accounting** | **yes, our `c_quote`** | **the donor** |
| 4.4BSD encumbered | ours | yes, `isquote` | read-only |

**FreeBSD 2.0.5 is the one to follow**, and the field list misled an
earlier reading of this file into ruling it out. Its extra fields are
additions, not replacements: `c_cc`, `c_cf` and `c_cl` work exactly as
ours do, the cblock pool survives with 116 references, and the quote
bit comes out of our own array:

	cblockp = (struct cblock *)((long)clistp->c_cf & ~CROUND);
	chr = (u_char)*clistp->c_cf;
	if (isset(cblockp->c_quote, clistp->c_cf - (char *)cblockp->c_info))
		chr |= TTY_QUOTE;
	clistp->c_cf++;
	clistp->c_cc--;

What is theirs and must come off is the cblock reservation accounting
-- `c_cbcount`, `c_cbreserved`, `c_cbmax`, `cslushcount`,
`cblock_alloc`, `cblock_free`, `clist_alloc_cblocks` -- which needs
struct fields this tree does not have and a protocol `tty.c` would
have to call at open. Strip that and what remains is the cblock walk
this tree's structure asks for.

It descends from Lite, and no finer than that. An earlier version of
this file said "from Lite2 specifically", on the grounds that its
initialiser is `clist_init` where NetBSD kept `cinit`. That does not
follow. **Lite1 has `clist_init` too** -- both Lites carry the same
file, the same ten stubs and the same `@(#)tty_subr.c 8.2 (Berkeley)
9/5/93` -- and FreeBSD's is dated 1994, so it predates Lite2 and
cannot descend from it. The two Lites are indistinguishable by
anything in this file. The rename is CSRG's own, somewhere between
4.4BSD and the Lite cut.

What the name does show is that FreeBSD started from a Lite rather
than from Net/2, which is what matters: it is on this tree's side of
the settlement.

## FreeBSD 2.0, not 2.0.5

Checking the releases *before* Lite2 as well as after changes which
version to follow. FreeBSD 2.0 of November 1994 has the same
implementation with far less of the accounting on top:

| | lines | accounting touches | untouched functions |
|---|---|---|---|
| FreeBSD 2.0 | 280 | 10 | `nextc`, `catq` |
| FreeBSD 2.0.5 | 298 | 38 | `nextc` |

Its `getc` is 2.0.5's to the character except at the end:

	2.0.5    cblock_free(cblockp);
	         if (--clistp->c_cbcount >= clistp->c_cbreserved)
	                 ++cslushcount;
	2.0      cblock_free(cblockp);

and 2.0 has no `clist_alloc_cblocks`, `clist_free_cblocks` or `cbstat`
at all. The reservation protocol arrived between the two. So 2.0 is
the version to follow and 2.0.5 is the same file with more to strip.

## And the NetBSD line never had it to lose

NetBSD 0.9 of October 1993 -- before 4.4BSD-Lite1 shipped -- already
carries de Raadt's rewrite: 520 lines, zero cblock references, twelve
`TTY_QUOTE`, "Copyright (c) 1993 Theo". So there is no earlier NetBSD
with CSRG's clist in it. The rewrite predates the settlement rather
than responding to it, which also means it is not encumbered; it is
simply built on a different structure.

## How much of it transfers, counted

Not a verbatim import, even from 2.0. Of its 280 lines across the nine
functions this tree needs, 10 touch the cblock helpers, and seven of
the nine have at least one. The 2.0.5 figures, for comparison, are 298
lines and 38 touches:

| function | lines | touching the accounting |
|---|---|---|
| `clist_init` | 5 | 1 |
| `getc` | 32 | 3 |
| `q_to_b` | 36 | 3 |
| `ndflush` | 31 | 3 |
| `putc` | 41 | 9 |
| `b_to_q` | 72 | 9 |
| `nextc` | 18 | 0 |
| `unputc` | 39 | 6 |
| `catq` | 24 | 4 |

So it is adapted rather than taken, which is kind B in `imports.md`'s
terms and the same category as `elf_load_psection`. `nextc` is the
only one that would transfer untouched.

## The later releases, checked

The two families stay divergent, so nothing later changes the answer:

| tree | lines | cblock refs | `TTY_QUOTE` |
|---|---|---|---|
| FreeBSD 2.1 | 663 | 117 | 4 |
| FreeBSD 2.2 | 695 | 120 | 4 |
| NetBSD 1.2 | 554 | 0 | 14 |
| NetBSD 1.3 | 550 | 0 | 14 |

FreeBSD's line keeps Greenman's cblock implementation and NetBSD's
keeps de Raadt's flat buffer. No release of either reverts to CSRG's,
and no later BSD offers a third option.

Nine of the ten map across. The tenth, `ndqb`, FreeBSD dropped. In
this tree it is called only by news3400's `bm/bmcons.c` and
`iop/rs.c`, never by `kern/tty.c` and never on the i386 path, so
nothing here exercises it -- but it is in the file and those ports
call it, so it still needs a body, and for that one the encumbered
file is the shape.

## 386BSD, checked because it rewrote execve

It rewrote this too, and further. There is no `tty_subr.c`: Jolitz
replaced the clist with `kern/tty_ring.c`, 162 lines under his own
copyright --

	Copyright (c) 1989, 1990, 1991, 1992 William F. Jolitz, TeleMuse

-- which is clean, as his `kern_execve.c` is. But it is a ring buffer
with no `struct clist` at all, its functions are `putc getc nextc
ungetc unputc initrb catb` so `q_to_b`, `b_to_q`, `ndqb` and `ndflush`
have no counterpart, and it does not quote. Nothing to take.

The function set confirms the shape is unchanged apart from Lite's own
editing:

	4.4BSD enc.  cinit getc q_to_b ndqb ndflush putc b_to_q
	             nextc unputc catq getw putw
	this tree    clist_init getc q_to_b ndqb ndflush putc b_to_q
	             nextc unputc catq

Ten of the twelve, with `cinit` renamed `clist_init` and `getw`/`putw`
dropped. Those two changes are Lite's, not a donor's, and the ten that
remain keep their names, their arguments and their order.
