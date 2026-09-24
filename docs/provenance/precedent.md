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
   Lite2: NetBSD 1.0 and 1.1, FreeBSD 2.0.5, and OpenBSD from 1996.
   OpenBSD forked from NetBSD in October 1995, four months after Lite2,
   and keeps 4.4BSD structure longest of the three.
4. **Later BSDs in historical order**, stopping at the earliest release
   that has the thing.
5. **XNU and Rhapsody: read-only.** See below.
6. **Our own code, last**, and only where the four above have nothing.

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
