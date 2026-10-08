# Precedent, attribution, and what in this tree is ours

How donors are chosen for changes to 4.4BSD-Lite2, what has been
written here rather than taken, and corrections to statements already
committed. Facts, not legal advice.

## Where to look, in order

1. **4.4BSD-Lite2 itself**, including its other ports: a spelling this
   tree already uses beats one borrowed from elsewhere.
2. **4.4BSD-Lite**, the settlement baseline. Its kernel differs from
   Lite2's in about one file in eight, so it also shows whether
   something is a Lite2 change or older.
3. **The contemporaries**, all released within about eighteen months of
   Lite2: NetBSD 1.0 and 1.1, FreeBSD 2.0.5, OpenBSD from 1996, and
   Lites 1.1.u3 of March 1996. OpenBSD forked from NetBSD in October
   1995, four months after Lite2, and keeps 4.4BSD structure longest
   of the three. Lites is a 4.4BSD-Lite server for Mach that ran, so
   it had to fill bodies the Lite cut emptied; what it has and what it
   does not is in `lites.md`, and it has no pmap at all.
4. **Later BSDs in historical order**, stopping at the earliest release
   that has the thing.
5. **XNU and Rhapsody: read-only.** See below.
6. **Our own code, last**, and only where the four above have nothing.

## The VM system is Mach's, and that changes the order for it

The search order above is for Berkeley's own code. `sys/vm` is not
Berkeley's own code: it is Mach's, and every one of the seventeen
files in that directory carries a Carnegie Mellon notice. So do the
`pmap.c` of each port, by way of Utah:

	hp300/hp300/pmap.c   "contributed to Berkeley by the Systems
	                      Programming Group of the University of
	                      Utah Computer Science Department"
	i386/i386/pmap.c     the same, "and William Jolitz of UUNET
	                      Technologies Inc."

The i386 pmap is therefore not a separate design that resembles
hp300's. It is hp300's lineage -- Utah's Mach work, which McKusick's
commit of 6 December 1990 describes as "adopted from Mach 2.5" --
forked by Jolitz for the 386 and left unfinished. That is why the two
have the same tests in the same positions with different bodies.

Three consequences for anything under `sys/vm` or in a `pmap.c`.

**The other ports are closer kin than usual.** hp300's pmap is Utah's
own, complete, and predates the i386 port's VM work by eleven months.
It is the first place to look and often the last.

**Mach itself is a legitimate source, not a foreign tree.** Where this
tree and the descendants all derive from Mach 2.5, Mach is the common
ancestor rather than an outside system, and reading it settles
questions that reading the forks only muddies.

**And the descendants are kin here too, for longer than elsewhere.**
NetBSD and OpenBSD kept the Mach VM until they moved to UVM around
1999; FreeBSD keeps it still. So a NetBSD 1.0 or FreeBSD 2.0.5
`pmap.c` is the same Mach codebase under different maintenance, not a
rewrite -- which makes it better corroboration for VM work than it
would be for, say, a device driver, where those trees really did start
again.

This was got wrong before it was written down. `docs/provenance/
audit.md` records citing NetBSD and FreeBSD for VM-adjacent work as
though they were outside trees, and reading OpenBSD's comment about
`pt_map` as "nobody solved this" when it was describing hp300's
solution in this tree.

## XNU and Rhapsody are read-only, and never for the stubs

`github.com/apple-oss-distributions/xnu` (28 branches, `rel/xnu-124`
through `rel/xnu-12377` and `main`) and
`github.com/calmsacibis995/xnu-rhapsody-53` descend from 4.4BSD and
are a quarter-century of annotated evolution of the same code. They
may be read to answer *what* a function must do and *why* something
changed. Nothing is ever copied from them, and nothing written here
may be derived from reading them: a BSD-licensed precedent that could
have been copied is required for anything that lands in this tree.

They carry Apple's Public Source License. XNU's `bsd/vfs/vfs_bio.c`,
measured at the oldest branch (`rel/xnu-124`), stacks four notices:
Apple 2000 under the APSL, Christopher G. Demetriou 1994, the Regents
1982/1986/1989/1993, and

	(c) UNIX System Laboratories, Inc.
	All or some portions of this file are derived from material
	licensed to the University of California by American Telephone
	and Telegraph Co. ...

That last one matters more than the first. `vfs_bio.c` is one of the
eight files whose function bodies the USL settlement removed from
4.4BSD-Lite and Lite2; Apple kept the material because NeXT held a
UNIX licence. So for the 35 deleted bodies -- the place where these
trees look most useful -- they are the worst place to look, on two
independent grounds. That work goes to NetBSD 1.0, which is
BSD-licensed and may be copied.

## Every claim names a tree and a release, and is read before it is made

Two failures, one of which nearly reached the tree, produced this rule.

A symlink shadow tree of sys/ was proposed as the way to build a kernel
without writing into the source tree, and called "the traditional BSD
answer". It is not. lndir is an X11 idiom and is in none of the trees
here; every BSD of the era writes the kernel build into the source
tree, and FreeBSD 2.0.5 even ships sys/compile. The proposal was
rejected before it was written, but it was put forward wearing the
vocabulary of precedent.

usr.bin/lorder/lorder.sh's note said its form was "taken from
OpenBSD's lorder", with no release. OpenBSD 1996 reads `nm -go $*';
the form actually taken -- pairs written directly, "$@" quoted, NM
overridable -- is OpenBSD's current tree. The code is right and the
citation pointed at the wrong decade. It is corrected in the file.

So, without exception:

1. A citation names the tree AND the release or branch. Not "OpenBSD's
   lorder" but "OpenBSD's current tree, usr.bin/lorder/lorder.sh".
   An undated citation in this tree reads as a contemporary, and the
   contemporaries are 4.4BSD-Lite, NetBSD 1.0 and 1.1, FreeBSD 2.0.5
   and OpenBSD 1996.

2. Nothing outside the BSD trees on disk is precedent. If an idea
   comes from X11, from GNU, from Linux, from a model's training or
   from its own reasoning, it is this project's own invention: it is
   labelled as such, the alternatives are given, and the maintainer
   decides. That no BSD does a thing is itself the finding, and is
   reported rather than covered over with a plausible-sounding
   source.

3. A claim is written only after the file it describes has been read
   in the session that writes it -- not from memory of what a BSD
   "usually does". The sweep described below is how that reading is
   done; if it was not run, the claim is not made.

## The CSRG history is read-only, like XNU

github.com/weiss/original-bsd carries the Computer Systems Research
Group's own history, every release from 1BSD to 4.4BSD-Lite2, with the
commits that made them. It answers questions no other tree can: when a
line in this tree was written, what it replaced, and whether Berkeley
corrected it somewhere else.

It is read-only, on the same terms as XNU and Rhapsody, and for a
sharper reason: most of what it holds is the encumbered 4.4BSD that
the USL settlement removed from the Lite releases. Nothing is copied
from it. What may be taken is what the history says -- dates, commit
messages, which files changed together, and the fact that a given
line exists elsewhere in Berkeley's own tree.

**This rule was broken, and the breach is recorded here so the shape
of it is recognisable.** Twelve function bodies were written into
kern/vfs_bio.c and kern/tty_subr.c by reading the encumbered files and
reproducing them. The commit messages stated a policy -- "a
reconstructed body is 4.4BSD's exact text, changed only where this
tree demonstrably changed something" -- which contradicts the sentence
above in the same repository. Measured afterwards, four of the twelve
were byte-identical to their originals and the rest differed only by
the sys/queue.h conversion.

Two lessons, both operational.

**The test.** Any body that is not taken verbatim from a named
permissive donor is diffed against the encumbered original before it
is committed, and the result goes in the commit message. Not to write
it -- to prove it is not a copy. If it comes out line-for-line, it is
a copy whatever the intention was, and it does not go in. That diff
was available every day the twelve bodies were written and was never
run until someone asked for it.

**No original works.** This project no longer writes function bodies
at all. Where a body is missing, the answer is an import: skim the
releases of NetBSD, OpenBSD and FreeBSD -- earlier than Lite2 as well
as later, since each needed time to absorb the Lite code and build on
it -- pick the best fit by measurement, and shrink-wrap it, cutting
what this tree's interfaces do not want rather than reshaping this
tree to suit the donor. Every such import is labelled a transplant
with its author, its licence and what was cut.

What remains permitted is a correction to code that already exists
here: a wrong constant, a missing flag, a guard the contemporaries all
have. Those cite a donor or another port in this tree and stay at the
scale of a line or two.

## Era-appropriate is not the same as correct

The donor search in this file is written around the contemporaries,
because that is where the licences are clean and the structures still
match. That bias has a failure mode, and it has already bitten once.

This project's aim is an ancient system that stays minimal and keeps
its shape, built and run to **current** conventions: a modern C
toolchain, current POSIX, and whatever the linker emits today. A donor
from 1995 is era-appropriate and may still be wrong, because it
encodes 1995's conventions.

The worked example is the ELF loader. Every BSD of the era --
NetBSD 1.1 through 1.6, OpenBSD 1996, and the rest -- caps a binary at
two loadable segments, each carrying the same `XXX Can handle only 2
sections' comment. Nothing then produced more. Modern `ld` does:
binutils made `-z separate-code' the default in 2018, and this tree's
own `/sbin/init', linked with binutils 2.42, has four `PT_LOAD'
segments -- headers, text, rodata, data. Every era donor rejects it.

So when a donor is needed, the question is not only which release fits
this tree's structures, but which handles what this tree's toolchain
actually emits. Sometimes no release satisfies both, and that is worth
knowing before the import rather than after.

The reverse also holds: a working-around is not a fix. Linking
`/sbin/init' with `-z noseparate-code' would have made every era donor
work, at the cost of pinning this system's output to 1995 link
conventions for good. That was proposed here and was wrong.

Where it does settle a question, the finding is what gets recorded,
and any line adopted must also exist in a tree that may be copied. The
console device in i386/conf/LINK.i386 is the first use: the history
showed that the driver was called cn until 1992, that the
reorganisation renamed it to pc, that GENERIC.i386 and CIRCE were
never followed through while ARGO and BLITZ were, and that the
corrected line is Berkeley's own. The Lite releases ship only
GENERIC.i386, which is why the corrected form is not in this tree.

## Proving an absence

A found donor proves itself: the line is shown. An absence does not,
and this tree has already recorded one wrong claim because three trees
were searched and eight were not.

So: no proposal may state that no donor exists unless every tree above
has been searched, and the commit message names them. A sweep tool
that searches all of them in this order, and prints a line per tree
whether or not it found anything, is kept outside the tree with the
maintainer's working copies; it is not part of building this system.

## Corrections to committed messages
- **`i386: locore.s must pass main a frame pointer`**. The message
  credits NetBSD 1.0's `locore.s:572` with the two instructions. That
  is true, but **hp300 does the same thing in this tree** and should
  have been cited first under rule 1. Its `locore.s:1126` reserves the
  frame and passes its address before calling main:

	lea	sp@(-64),sp	| construct space for D0-D7/A0-A7
	pea	sp@		| addr of space for D0
	jbsr	_main		| main(firstaddr, r0)

  The change itself is unaffected -- the i386 needs `movl %esp,%eax;
  pushl %eax` because its arguments go on the stack, not in a register
  -- but a reader was sent outside the tree for something inside it.
  The following commit, `i386: the return to user must use the frame`,
  does cite hp300 for the reading half.
- **`execve, written: the kernel execs`**, for its `cpu_fork` hunk. The
  message credits NetBSD 1.0's `cpu_fork` with passing
  `VM_PROT_READ | VM_PROT_WRITE` when mapping the child's u-area.
  **Five ports in this tree pass exactly that**, and none was cited:

	hp300, luna68k, pmax, news3400 and sparc, each in its own
	arch/vm_machdep.c, all `VM_PROT_READ|VM_PROT_WRITE`

  vax and tahoe have no `pmap_enter` in `cpu_fork` at all, so the
  i386 was the only port of seven doing something different. The
  change is unaffected; the attribution was six references to NetBSD
  where one line of `grep` across this tree would have settled it.
- **Checked and not misses**, recorded so the same ground is not
  covered twice. `i386: wd reads the bad-sector table only when the
  label says to` cites FreeBSD 2.0.5, and that is right: `isbad()` is
  defined in `hp300/hp300/dkbad.c`, `vax/vax/dkbad.c` and
  `i386/i386/dkbad.c` and called by nothing, so no in-tree driver
  gates the read on `D_BADSECT` and there was no rule 1 answer.
  `execve`'s `USRSTACK` hunk cites NetBSD 1.0's `init_main.c:347`,
  which is right because `kern/init_main.c` is shared code with one
  copy and no per-port variant to prefer.

- **`routed, XNSrouted: declare iftraceinit at file scope`**
  (`e7c4c061`). The message says FreeBSD and OpenBSD "both replaced
  routed with a later program that has no iftraceinit at all". That is
  true of OpenBSD. It is **false of FreeBSD 2.0.5**, which has both
  programs under `usr.sbin/` rather than `sbin/`, each with the same
  block-scope declaration, unfixed. The search that produced the claim
  looked only in `sbin/`. The change itself is unaffected: FreeBSD's
  copy is not a better donor, and the line taken is NetBSD 1.1's.
  NetBSD 1.2 also carries the fixed form, which the same search
  missed.

- **`ftp, systat: declare the command table after its struct`**
  (`b4b737d6`). The message credits NetBSD 1.6 with keeping the
  declaration in ftp_var.h. **NetBSD 1.5 is the earliest** that does
  (`ftp_var.h:310`), and 1.6 and 10 follow it; **OpenBSD's current tree
  arrived at the same arrangement independently**, which the message
  does not mention. The change itself is unaffected, and a later sweep
  of all fourteen trees strengthened it: Lite1, Lite2, NetBSD 1.0
  through 1.4, FreeBSD 2.0.5 and OpenBSD 1996 declare it in extern.h,
  the four later trees in ftp_var.h, and **not one tree of the fourteen
  reorders the include** -- NetBSD 1.5 and after moved the struct
  nearer the include rather than the include past the struct. The
  alternative considered at the time, moving `#include "extern.h"'
  below the struct, therefore has no precedent anywhere; it compiles,
  and that is all that can be said for it.

## Two decisions checked after the fact

Both were committed without being put to the maintainer first, which
the rule above now forbids, and both were re-researched afterwards
across every tree. Both stand:

- ftp and systat's command table, above.
- `mail, window: drop -R` (`6d2429a6`). -R told the older compilers to
  put initialized data into read-only text; GCC has never had it.
  NetBSD 1.1 and FreeBSD 2.0.5 drop it and keep the rest of the line;
  NetBSD 1.2 and after, and OpenBSD 1996, have no CFLAGS line in this
  Makefile at all. Lite2's line held -R and nothing else -- it had
  already lost the -DUSE_OLD_TTY that Lite1 and NetBSD 1.0 carry -- so
  dropping the flag and dropping the line are the same act here, and
  the result is what NetBSD 1.2 and OpenBSD 1996 have. window keeps
  -DVMIN_BUG: it guards live code in wwrint.c, `#if defined(OLD_TTY)
  || defined(VMIN_BUG)', and no tree drops it while still building
  window, so dropping it would have changed which branch compiles on
  nobody's authority.

## What in this tree is ours

No function body, algorithm or data structure in 4.4BSD-Lite2 is ours,
and nothing under `usr/src` carries our copyright except the one file
noted below. Everything written here:

| file or change | size | what it is |
|---|---|---|
| `build/make.sh` | 92 code lines | ours; shaped after NetBSD's nbmake wrapper and OpenBSD's Makefile.cross, no line copied |
| `build/sysroot.sh` | 55 code lines | ours; staging follows NetBSD's build.sh and OpenBSD's cross-tools |
| the generated specs file | 9 lines | recipe is GCC's NetBSD ELF target definition; the absolute paths are ours |
| `lib/csu/i386_elf/Makefile` | 27 code lines | ours, modelled on this tree's own `lib/csu/i386/Makefile`; the only file under `usr/src` carrying our copyright line |
| `lib/libl/Makefile` | 4 code lines | NetBSD 1.0's, with one `.PATH` changed |
| `_C_LABEL` in `DEFS.h`, `SYS.h` | ~20 lines | macro block is NetBSD 1.5's; both BSDs keep it in `machine/asm.h`, which this tree lacks, so the placement is ours |
| `Ovfork.s`, `lib/Makefile`, `include/Makefile`, `sbin` include paths | ~15 lines | a donor's form applied to this tree's file, where no donor line existed to copy |

Everything else changed under `usr/src` is a donor's own line:
`${DESTDIR}` prefixes, `${HOSTCC}` rules, `__P` prototypes,
`ENTRY(...)`, `_C_LABEL(...)`, `pax`, `cp = (char *)cp + cc`, and the
verbatim import of NetBSD 1.6's startup code (751 of the 1147 lines
added under `usr/src`).

No licence is asserted for the files that are ours. They carry an
SPDX-FileCopyrightText line and deliberately no licence identifier;
4.4BSD-Lite2's Berkeley terms, in `./COPYRIGHT`, cover the Berkeley
material and not those files.
