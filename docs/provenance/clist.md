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

## Is Greenman's file encumbered? No, and the reason is checkable

Its header carries no Regents or AT&T notice at all, only his own:

	Copyright (C) 1994, David Greenman. This software may be used,
	modified, copied, distributed, and sold, in both source and
	binary form provided that the above copyright and these terms
	are retained.

	$Id: tty_subr.c,v 1.7 1994/09/25 19:33:50 phk Exp $

and it is written against the published interface -- it includes
`<sys/clist.h>` and defines `cfreelist` and `cfreecount`, the globals
that header declares.

The stronger evidence is that **FreeBSD's own line had no `tty_subr.c`
before 2.0 to derive one from.** `sys/kern/tty_subr.c` is absent from
`release/1.0.0_cvs`, `release/1.1.0_cvs` and `release/1.1.5.1_cvs`.
What those carry instead is `sys/kern/tty_ring.c`: Jolitz's ring
buffer, 215 lines under his TeleMuse copyright, 79 lines evolved from
386BSD 0.1's. Their `sys/clist.h` is there with the same `struct
cblock` and goes unused.

So the Net/2-derived FreeBSD releases used Jolitz's ring, and
Greenman's clist first appears in the 2.0 line, after the rebase onto
4.4BSD-Lite. There is no earlier FreeBSD file for it to be a
derivative of.

## And no earlier FreeBSD fits better

2.0 is the earliest release in that line with a cblock clist at all,
so it is both the cleanest and the first. 1.x is not a candidate on
its merits either: `tty_ring.c` has no `struct clist`, no `q_to_b`,
`b_to_q`, `ndqb` or `ndflush`, and no quoting.

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

## The same survey, for the other settlement bodies

Having found that FreeBSD 2.0 fits for the clist, the obvious question
is whether it fits for the rest. Each of its files was compiled
against this tree's headers and then linked into a kernel, which is
the only way to tell. Results:

| our file | stubs | FreeBSD 2.0 | what it would take |
|---|---|---|---|
| `kern/tty_subr.c` | 10 | Greenman 1994 | **works** -- builds, links, boots, and a user process's write reaches the console |
| `kern/kern_acct.c` | 2 | C. G. Demetriou 1994 | **two lines** -- builds, links, boots |
| `kern/kern_physio.c` | 2 | John S. Dyson 1994 | compiles clean, but will not link |
| `kern/sys_process.c` | 2 | Sean Eric Fagan 1994 | needs a new `struct proc` field |
| `kern/subr_rmap.c` | 3 | absent | FreeBSD dropped resource maps entirely |
| `kern/vfs_bio.c` | 4 | John S. Dyson 1994 | a rewrite; NetBSD 1.0 is the donor there |

None of the five that exist carries a Regents notice; they are all
1994 work by named authors, like Greenman's.

**`kern_acct.c` is two lines from working.** `VOP_UNLOCK(nd.ni_vp)`
needs this tree's three-argument form, `VOP_UNLOCK(nd.ni_vp, 0, p)` --
the VFS locking interface changed between the Lites -- and a
`LEASE_CHECK(vp, p, p->p_ucred, LEASE_WRITE)` call must come out,
NFS leasing being something FreeBSD added and this tree does not have.
With those two the kernel builds and boots. Its function set is ours
plus two helpers: `acct` and `acct_process`, with `encode_comp_t` and
`acctwatch` alongside.

**`kern_physio.c` compiles with no errors at all and then fails to
link**, on `getpbuf` and `relpbuf`. Those are FreeBSD's physical
buffer pool, from their VM, and this tree has nothing like them. A
file that compiles clean is not a file that fits, which is worth
recording: the compile was the encouraging result and the link was the
true one.

**`sys_process.c` has exactly our two functions**, `ptrace` and
`trace_req`, and wants `p_tptr` in `struct proc` -- a trace-parent
pointer FreeBSD added. That is the same kind of obstacle as de
Raadt's seven-field `struct clist`: a structure change reaching
outside the file.

**`subr_rmap.c` has no counterpart at all.** FreeBSD removed resource
maps. So for those three stubs the contemporaries offer nothing and
the method is the written one.

## NetBSD 1.0, surveyed the same way

The FreeBSD survey above was done first and alone, which was a gap:
NetBSD 1.0 has all five files FreeBSD does and one more, and two of
them link into this kernel untouched.

| our file | stubs | NetBSD 1.0 | compile | link |
|---|---|---|---|---|
| `kern/subr_rmap.c` | 3 | Wolfgang Solfrank 1992, 1994 | clean | **links** |
| `kern/vfs_bio.c` | 4 | C. G. Demetriou 1994 | clean | **links** |
| `kern/sys_process.c` | 2 | C. G. Demetriou 1994 | clean | `FIX_SSTEP`, `process_sstep`, `process_set_pc` |
| `kern/kern_acct.c` | 2 | C. G. Demetriou 1994 | `VOP_UNLOCK` arity | -- |
| `kern/kern_physio.c` | 2 | C. G. Demetriou 1994 | `p_holdcnt` in `struct proc` | -- |
| `kern/tty_subr.c` | 10 | Theo de Raadt 1993, 1994 | needs a seven-field `struct clist` | -- |

**`subr_rmap.c` is the one FreeBSD cannot supply at all**, having
removed resource maps, and NetBSD's links with nothing changed.

**`vfs_bio.c` links too**, which is consistent with what
`missing.md` already records: NetBSD 1.0's is Lite's own file with the
holes filled, which is why the ten bodies written into this tree so
far followed it. The remaining four -- `bwrite`, `bdwrite`, `bawrite`,
`breadn` -- can follow the same way, or the file can be taken whole;
that is a decision, not a finding, and taking it whole would discard
ten bodies already written and reviewed.

## Where the two trees land together

Taking the best of each, by file rather than by tree:

| file | stubs | donor | state |
|---|---|---|---|
| `tty_subr.c` | 10 | FreeBSD 2.0 | fitted and tested; a user write reaches the console |
| `vfs_bio.c` | 4 | NetBSD 1.0 | links untouched; ten bodies already written from it |
| `subr_rmap.c` | 3 | NetBSD 1.0 | links untouched |
| `kern_acct.c` | 2 | either | two lines, tested with FreeBSD's |
| `kern_physio.c` | 2 | neither | FreeBSD wants `getpbuf`/`relpbuf`, NetBSD `p_holdcnt` |
| `sys_process.c` | 2 | neither | FreeBSD wants `p_tptr`, NetBSD three MD hooks |

Nineteen of the twenty-three have a donor that builds. The four that
do not are raw device I/O and `ptrace`, and both want a field or a
hook that reaches outside the file -- the same obstacle de Raadt's
`struct clist` presents, and the same answer: write them.

## The earlier trees, and 386BSD in particular

The surveys above stopped at the contemporaries. Going earlier changes
two of the six answers and confirms the method for a third.

**386BSD 0.1 renames what Jolitz rewrote**, with a doubled underscore,
and keeps CSRG's text under the original name where he did not. So the
licence sorts the files for you:

| file | copyright | usable |
|---|---|---|
| `kern_acct.c` | Regents 1982, 1986, 1989 | no -- Net/2 text, encumbered |
| `sys_process.c` | Regents 1982, 1986, 1989 | no -- the same |
| `kern__physio.c` | William F. Jolitz 1989-1992 | clean |
| `vfs__bio.c` | William F. Jolitz 1989-1992 | clean |
| `subr_rlist.c` | William F. Jolitz 1992 | clean |
| `tty_ring.c` | William F. Jolitz 1989-1992 | clean |

The clean ones are all replacements rather than implementations of
what this tree declares. `kern__physio.c` has `rawread` and `rawwrite`
and no `physio` or `minphys` at all; `subr_rlist.c` is resource lists,
not resource maps; `tty_ring.c` is a ring, not a clist. Jolitz
rewrote the interface each time, so none of them fits a tree built on
the 4.4BSD one -- which is the same reason his `kern_execve.c` could
not be used for `execve`.

**And `kern_physio.c` has an answer after all, in this tree.** 4.4BSD's
own `physio` holds the process with a flag --

	p->p_flag |= SPHYSIO;
	vslock(a = bp->b_un.b_addr, requested);
	vmapbuf(bp);

-- and this tree has that flag, renamed: `sys/proc.h:203` is
`#define P_PHYSIO 0x10000 /* Doing physical I/O. */`. NetBSD replaced
it with a counter, `p_holdcnt`, which is why their file will not
compile here; FreeBSD replaced the buffer allocation with a pbuf pool,
which is why theirs will not link. Both changed the mechanism; this
tree still has 4.4BSD's.

The buffer pool is here too, and by the same pattern as the clist:

	clist   i386/machdep.c valloc(cfree, struct cblock, nclist)
	physio  i386/machdep.c valloc(swbuf, struct buf, nswbuf)

4.4BSD's `getswbuf` and `freeswbuf` are static helpers inside its own
`kern_physio.c`, removed with the bodies, which is why they are absent
here rather than being somewhere else. And `rawread` and `rawwrite`
already have bodies in this tree: only `physio` and `minphys` were
cut.

So `kern_physio.c` is the `vfs_bio.c` case, not the `tty_subr.c` case.
The machinery is all present -- the flag, the pool, `vslock`,
`vmapbuf`, which the i386 does have -- and only the code driving it
was removed. It is written, not imported, with the contemporaries
readable for algorithm and neither usable for text.

## Examining this tree, file by file, to see what is actually needed

A file that compiles and links can still be wrong, so each conclusion
above was checked against what this tree declares and what calls it.

**`subr_rmap.c` from NetBSD 1.0 is verified, not merely linking.** The
signatures match ours exactly -- `rminit(mp, size, addr, name, nelem)`,
`rmalloc(mp, size)`, `rmfree(mp, size, addr)` -- and `struct map` and
`struct mapent` have identical fields. The one discrepancy is a
comment: ours says `m_limit` is the "address of last slot in map",
NetBSD's says "first slot beyond map". NetBSD is right and **4.4BSD's
own comment is wrong**, because its `rminit` sets

	mp->m_limit = (struct mapent *)&mp[mapsize];

which is one past the end. NetBSD's writes `(struct mapent *)mp +
nelem`, the same address, since `sizeof(struct map)` and `sizeof(struct
mapent)` are both eight bytes here. And nothing else in this tree reads
`m_limit` at all -- only the removed functions did -- so the convention
is whatever the implementation sets, and the two agree.

**`sys_process.c` is the written case, and this tree says so clearly.**
Our two stubs are `ptrace` and `trace_req`; 4.4BSD's file has `ptrace`
and `procxmt`. They are the same function, renamed by Lite, which the
call site proves:

	4.4BSD   } while (!procxmt(p) && p->p_flag & STRC);
	Lite2    } while (!trace_req(p) && p->p_flag & P_TRACED);

So this tree carries 4.4BSD's interface under Lite's names, and both
contemporaries left it: NetBSD's wants `process_sstep` and
`process_set_pc`, a machine-dependent register interface 4.4BSD does
not have, and FreeBSD's wants `p_tptr` in `struct proc`. Neither is
a rename; both are new mechanisms.

Everything 4.4BSD's file needs is here, which was checked name by
name. Of its callees only `setrun` is missing, and that is a third
Lite rename -- `setrunnable`, at `kern/kern_synch.c:622`. `fuword`,
`suword`, `fuiword`, `suiword`, `useracc`, `vm_map_protect`, `pfind`,
`psignal` and `proc_reparent` are all present.

`FIX_SSTEP` is not an obstacle either: `miscfs/procfs/procfs_ctl.c:56`
already defines it empty when the port provides none, which is this
tree's own convention for that hook.

So the four stubs without a donor -- `physio`, `minphys`, `ptrace`,
`trace_req` -- are all in the same position: this tree has every piece
of machinery they need, under Lite's names, and the contemporaries
cannot supply them because each replaced the mechanism rather than
renaming it.

## Did the other BSDs adopt Lite2's interfaces later?

Every survey above stopped at releases that **predate Lite2**. NetBSD
1.0 is October 1994 and FreeBSD 2.0 November 1994; Lite2 is June 1995.
So the obvious question is whether the later releases merged it.

The ancestry first. Net/2 of 1991 is the common root: 386BSD 0.1 came
from it in 1992, NetBSD and FreeBSD from 386BSD in 1993, and OpenBSD
from NetBSD in October 1995. 4.4BSD-Lite of 1994 and Lite2 of 1995 are
a separate CSRG line that each of the others merged from, at its own
pace.

**They did adopt the names.** `trace_req` is Lite2's rename of
4.4BSD's `procxmt`, and it appears in every later release of both
lines:

| release | `trace_req` | `procxmt` |
|---|---|---|
| NetBSD 1.0 | no | no -- its own naming |
| NetBSD 1.1, 1.2, 1.3, 1.4 | yes | no |
| FreeBSD 2.0 | no | no |
| FreeBSD 2.1, 2.2 | yes | no |
| OpenBSD 1996 | yes | no |

**They did not adopt the mechanism.** Compiled against this tree's
headers, the later releases do not get closer; they get further, apart
from one:

| release | compile errors | what stops it |
|---|---|---|
| NetBSD 1.0 | 0 | links: `process_sstep`, `process_set_pc` |
| NetBSD 1.1, 1.2, 1.3 | 19, 19, 20 | `struct sys_ptrace_args`, `SCARG` |
| OpenBSD 1996 | 19 | the same, inherited |
| FreeBSD 2.0 | 6 | `p_tptr` in `struct proc` |
| FreeBSD 2.1 | 6 | the same |
| **FreeBSD 2.2** | **0** | links: `PHOLD`, `PRELE`, `ptrace_set_pc`, `ptrace_single_step` |

FreeBSD 2.2.0 is the closest any of them comes. It has exactly our two
functions, `ptrace` and `trace_req`, it dropped the `p_tptr` that
blocked 2.0, and with two FreeBSD-only headers removed --
`<sys/sysproto.h>` and `<vm/lock.h>` -- it compiles against this tree
with no errors at all. Then it fails to link, on `PHOLD` and `PRELE`,
which are FreeBSD's process-hold macros, and on `ptrace_set_pc` and
`ptrace_single_step`, which are their machine-dependent hooks.

So the convergence is real and partial: the interface name crossed
over, the implementation did not. Each line kept the
machine-dependent layer it had built -- NetBSD's `process_*`,
FreeBSD's `ptrace_*` -- and 4.4BSD has neither, because its `procxmt`
does the register work itself through `fuword` and `suword`.

That is the canonical answer, and it is the same one for
`kern_physio.c`: the names travelled, the mechanisms did not, and this
tree still has 4.4BSD's.
