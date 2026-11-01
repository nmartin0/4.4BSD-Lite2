# The A2 files: sys_process.c and its kin, measured

Stage 3, item 3 of `plan-4.5.md`; `roadmap-audit.md`'s A2, "the text
that may have been copied from the encumbered files." Written
2026-11-01 (patch 265). `execve` is `exec-stack.md`; `vfs_bio.c`'s
corrections are `bufcache.md`; this record takes the rest: `kern_acct.c`,
`subr_rmap.c`, `kern_physio.c`, `sys_process.c` and
`i386/procfs_machdep.c`, and `vfs_bio.c` too, which A2 names and which
the clearance ledger had no row for.

## What the test is, and what it cannot see

`build/lineage.py`, as `disk-wd.md` used it: the distinct lines of a
file with twelve or more letters or digits, whitespace folded, and how
many are in each reference text. It reads only what it is given, and
never the CSRG tree. The references, all lawful to read:

- **Lite2's own file**, which for all six of these is the settlement's
  stub -- `Body deleted`, the Regents' notice, declarations. A line a
  candidate shares with it is Berkeley's own cleared text.
- **386BSD 0.0 and 0.1** (`/home/claude/386bsd`, tags `r00`, `r01`),
  which carry Net/2's text. Where their copy of a file is a full body
  under the Regents' notice (`sys_process.c`, 0.1's: "from:
  @(#)sys_process.c 7.22 (Berkeley) 5/11/91") it is Net/2's own, the
  best reference there is. Where it is a stub it is no reference. Where
  it is a full body under Jolitz's notice (`kern__physio.c`,
  `vfs__bio.c`) it is Berkeley-derived text reworked by him, and how
  much is Berkeley's is not established; a line in common with it is
  still a line in common with Net/2-era text.

It is a lower bound: a line renamed or reflowed counts as new. And it
can only find Net/2 text that some readable copy still has. **Where
every readable copy is a stub, "0 lines in common" means "nothing
could be compared", not "nothing was copied", and the table says so.**

## The measurements

| file | what the tree has | readable Net/2 text | lines | in Lite2's stub | in the Net/2 text, not Lite2's |
|---|---|---|---|---|---|
| `kern/sys_process.c` | FreeBSD 2.2, rev 1.24 (Fagan's 1994 `ptrace`) | 386BSD 0.1's `sys_process.c`, a full body (179 lines); 0.0's is a 110-line partial | 182 | 21 | **16 (9%)** |
| `i386/i386/procfs_machdep.c` | Pendry's `procfs_i386.c` as NetBSD had it, Jan 1994 | none needed (below) | 150 | 68 of 150 are in Lite2's `hp300/procfs_machdep.c` (45%) | -- |
| `kern/subr_rmap.c` | NetBSD 1.2, rev 1.11 (Solfrank, TooLs) | none: neither 386BSD has the file | 130 | 11 | none comparable |
| `kern/kern_acct.c` | FreeBSD 2.0 (Demetriou), two lines adapted | none: 386BSD 0.0's and 0.1's copies are stubs | 192 | 62 | none comparable |
| `kern/kern_physio.c` | NetBSD 1.0 (Demetriou), three mechanisms adapted | 386BSD's `kern__physio.c`, a full body under Jolitz's notice | 223 | 44 | **1** (`return (EFAULT);`) |
| `kern/vfs_bio.c` | NetBSD 1.0, with 1.1's corrections (patch 248) | 386BSD 0.0's `vfs__bio.c`, a full body under Jolitz's notice | 428 | 106 | **35 (8%)**, 17 after this patch (428 distinct lines before, 422 after) |

**`sys_process.c`.** The 16 lines are `ptrace`'s signature and its
`struct proc *curp;` declaration, the notice's `All rights reserved.`,
eight `case PT_...:` labels (the request names are Lite2's own
`<sys/ptrace.h>`'s) with the `#ifdef PT_GETREGS` that guards two of them,
`if (p->p_stat == SSTOP) {`, `return EFAULT;`, `return EINVAL;` and
`p->p_addr->u_kproc.kp_proc = *p;`. (An earlier draft of this record said
13, 7%, and described them wrongly: it counted only the lines first seen
in 0.1, missing three that 0.0's partial copy also has. Found by the
patch's reviewer and re-run.) Nothing there is anything but the interface. The same
test over the line of FreeBSD releases shows the Net/2 body going out
as Sean Eric Fagan's 1994 rewrite replaced it:

| | lines | in the Net/2 text, not Lite2's |
|---|---|---|
| FreeBSD 1.0 | 260 | 133 (51%) |
| FreeBSD 1.1.5.1 | 210 | 64 (30%) |
| FreeBSD 2.0, 2.1 | 166 | 31 (18%) |
| FreeBSD 2.2.0, 2.2.8 | 219, 221 | 18 (8%) |
| **this tree's** (2.2, rev 1.24, adapted) | 182 | **16 (9%)** |
| NetBSD 1.0; OpenBSD's 1995 import | 205; 185 | 10 (4%); 8 (4%) |

Clear on this test, for what it can see. (The first fit under 4.5BSD's rule is NetBSD 1.0's,
not FreeBSD 2.2's -- `plan-4.5.md`, stage 4, item 1, #380 -- and it
measures as clear as this one does; that replay is stage 4's.)

**`procfs_machdep.c`.** Jan-Simon Pendry wrote `/proc` for 4.4BSD
(1992-3, Imperial College); the Regents' copy of this file is in Lite2
twice, for `hp300` and `pmax`, under a Regents-and-Pendry notice like
this file's, and says it is Pendry's. 386BSD, which carries Net/2's
text, has no `/proc` at all. So this is Lite2's own sibling for a third machine, not a settlement
body, and what the line count shows is the structure it shares with
those two (45% of this file's lines are in `hp300`'s; the rest are this
port's register frame and FreeBSD 2.1.0's floating-point copy). Likely
clear: a sibling of Lite2's own files, with no comparable Net/2 text
anywhere readable (the settlement's list was not consulted). FreeBSD's three releases of it are 74% to 89% in
`hp300`'s, for the same reason.

**`subr_rmap.c`.** Net/2's resource-map code is not in any readable
tree: Lite2's file is three stubs and 386BSD has no copy. What the tree
has is Wolfgang Solfrank's, 1992 and 1994, written, by its own header,
"from how the swap code uses the interface and from sys/map.h's
comments". Shared with Lite2's stub: 11 lines (8%), its notice and
declarations. **This cannot be checked further.** The finding is the
author's statement, the file's independence from every text that can be
read, and `clist.md`'s survey, which found FreeBSD dropped resource
maps and every other BSD that has the file has this one.

**`kern_acct.c`.** `acct` and `acct_process` were withheld in Net/2
itself, in 1991 (386BSD 0.0 and 0.1 have the same `Body deleted` text),
so no Net/2 body for them was ever published. Christopher Demetriou
filled them for NetBSD in 1994, and FreeBSD took his file. **Nothing
readable to compare with** -- 62 of its 192 lines are in Lite2's stub
(the notice, the includes, the comment), and the other 130 have no
readable counterpart. NetBSD 1.0, 1.1, OpenBSD's 1995 import and
FreeBSD 2.0 and 2.1 measure the same (36%, 0% beyond the stub). The
finding is the history: no Net/2 body for it is readable anywhere, so
there is nothing to have copied *from* that can be shown; this is the
author's statement, not a measurement.

**`kern_physio.c`.** 386BSD's `kern__physio.c` is a working `physio`
under Jolitz's notice, the nearest readable thing to the withheld
body. This tree's has one line in common with it, `return (EFAULT);`.
Clear on what can be compared; the Net/2 original itself cannot be.
(NetBSD 1.0's own: 1 line; FreeBSD 2.0's: 4.)

**`vfs_bio.c`.** The one with something to find. 35 of its 428 lines
are also in 386BSD 0.0's `vfs__bio.c` and not in Lite2's stub. They are
the file's inheritance from NetBSD 1.0, which has the same 35 (of its
388), and are of two kinds:

- **Comment text**: eighteen lines, in four blocks (on `bdwrite`,
  `getblk`, `geteblk` and `getnewbuf`) and two one-line comments, word
  for word what 386BSD 0.0's `vfs__bio.c` says. (The `bdwrite` block's
  last line, `Described in Leffler, et al. (pp. 208-213).`, a citation
  of the Berkeley book, is not in 386BSD's copy and was kept.)
  Comments are text.
- **Interface lines**: `bread`'s and `allocbuf`'s signatures and their
  parameter declarations, `VOP_STRATEGY(bp);`, `VOP_STRATEGY(rabp);`,
  `reassignbuf(bp, bp->b_vp);`, `binshash(bp, bh);`, `bh =
  BUFHASH(vp, blkno);`, and four one-line assignments to `struct buf`'s
  fields. Fixed by Lite2's own `<sys/buf.h>` and `<sys/vnode.h>`; each is
  the one way to say the thing.

**Done in this patch:** the comment text reworded, in this tree's own
words (the same remedy as patch 261's `getxfile` comment), 18 lines
out; no code touched. Checked by compiling the kernel before and after:
**`vfs_bio.o` is byte for byte the same**, and the same check fails on a
one-constant change to `getnewbuf` (`PRIBIO+1` to `PRIBIO+2`), so it
sees code. After: 17 lines in common, all of them the second kind.

**Not decided here, for the maintainer:** whether the second kind --
interface lines, in NetBSD 1.0's file and so in every BSD's -- needs
more. The remedy would be to write them differently, which is
rewriting a function signature or a one-line call, and the rule that
this project does not write bodies (`precedent.md`) and 4.5BSD's first
fit both point the other way. Left as NetBSD 1.0 has them.

## The ledger had no rows for four of these

`clearance.md` was built from `imports.md`, and `imports.md` never
listed `kern/kern_acct.c`, `kern/kern_physio.c` or `kern/vfs_bio.c`
(nor `tty_subr.c`, which `clist.md` covers at length): the same hole as
`exec_elf.c`, found by patch 262. Found now by listing every kernel file
with a donor's RCS tag that is not Lite2's own and is named in neither
record; five files turned up, and one was a false alarm: `procfs_mem.c`
carries Pendry's `$Id` in Lite2's own copy too, and is a Lite2 file with
a fifteen-line change. Rows for the other four are in `clearance.md` and
`imports.md`.

`tty_subr.c` is FreeBSD 2.0's `getc`/`putc`/`b_to_q` family (David
Greenman) with its `ndqb` written here to 4.4BSD's shape (FreeBSD 2.0 has none), per `clist.md`. It was not measured
here: neither 386BSD has a `tty_subr.c` (they use Jolitz's ring), so
there is nothing readable to compare it with. Its note called it "deliberately an interim state ... to be replaced
function by function" and said "the canonical answer for a settlement
body is to write it", which was the rule before patch 262 and is not
now. **Decided by the maintainer, 2026-11-01 (patch 266):** this is the
state 4.5BSD ships, and the note is rewritten to say so. The record
had said `ndqb` was "stubbed"; the file says it is written, to 4.4BSD's
shape, since FreeBSD 2.0 has no such function. That one function is the
port's own writing, and a donor's replacing it is left to 4.6BSD.

## ptrace on a program you may run but not read

`ptrace` lets a process attach to a program it can execute but not read,
and so read its memory. Every BSD examined does the same (`sys_process.c`
is Fagan's, as NetBSD 1.0's and OpenBSD's are). **Decided by the
maintainer, 2026-11-01 (patch 266): not changed in 4.5BSD**, which is
the first faithful fit; listed as a known issue and left to 4.6BSD
(`plan-4.6.md`). Untested here whether any BSD refuses it under some
option; none was found.

## What was not established

- **Net/2's bodies for `kern_acct.c`, `subr_rmap.c`, and `tty_subr.c`
  cannot be compared with anything**, as above. The findings for them
  are about authorship, not measurement.
- **The test is a lower bound.** Text copied and then lightly reworded
  would not show.
- **The settlement's own list of files was not consulted** -- it is not
  available here. "Clear" means: no readable Net/2 text found beyond
  interface lines, a licence that permits use, and an author who says
  he wrote it. It is not a legal opinion.
- **The 18 comment lines reworded in `vfs_bio.c` were found by the same
  test**, so comment text that matches Net/2 but not 386BSD 0.0 would
  not have shown.
