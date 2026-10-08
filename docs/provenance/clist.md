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
| NetBSD, OpenBSD | seven fields, ring + bitmap | yes, its own way | needs a new `struct clist` |
| FreeBSD 2.0.5 | ten fields, reservation | yes, `c_quote` field | needs a new `struct clist` |
| Lites, CMU | **ours exactly** | **none at all** | would lose `TTY_QUOTE` |
| 4.4BSD encumbered | ours | yes, `isquote` | read-only |

The encumbered file is the only implementation that both fits the
structure and quotes, and it is the one that may be read but not
taken. That is precisely the settlement-body case.

The function set confirms the shape is unchanged apart from Lite's own
editing:

	4.4BSD enc.  cinit getc q_to_b ndqb ndflush putc b_to_q
	             nextc unputc catq getw putw
	this tree    clist_init getc q_to_b ndqb ndflush putc b_to_q
	             nextc unputc catq

Ten of the twelve, with `cinit` renamed `clist_init` and `getw`/`putw`
dropped. Those two changes are Lite's, not a donor's, and the ten that
remain keep their names, their arguments and their order.
