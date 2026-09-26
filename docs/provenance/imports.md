# What is Berkeley's, what is borrowed, and what is neither

Every change on this branch falls into one of six kinds. The kinds are
about provenance, not about size or difficulty: they say how far a
given patch stands from the code Berkeley shipped, so that a reader
can judge each one's authenticity without re-deriving the argument.

The second half of this file records what was NOT done, and what that
costs, because a fork is shaped as much by its refusals as by its
changes.

## The six kinds

	A  Berkeley's code, corrected to Berkeley's own intent
	B  Berkeley's code, corrected following another BSD's line
	C  Berkeley's code, changed with no donor line: our judgement
	D  Code transplanted that 4.4BSD-Lite2 never had
	E  A reimplementation of something the Lite cut removed
	F  Berkeley's own file, restored from a tree that kept it
	G  Files that are ours, outside Berkeley's tree

A is the most authentic and G the least. D, E and F are the only
kinds that put another system's text into usr/src.

## A -- corrected to Berkeley's own intent

The evidence for these is inside this tree or in Berkeley's own
history; no other system is needed to justify them.

  libc: sys_nerr non-const		Berkeley's 1993 commit on
					errlst.c says "fix def to match
					stdio.h" and did exactly that for
					sys_errlist, missing sys_nerr.
					This applies their stated rule to
					the one they missed.
  i386 pmap: /* static */		hp300 and luna68k comment the
					keyword out six times each in this
					tree, on this same function.
  i386: register_t			copied from this tree's own
					hp300 and sparc types.h, which
					define it identically.
  locore.s: movw and %al		this file already writes movw
					correctly six times; four lines
					had slipped.
  LINK.i386: console is pc0		Berkeley's ARGO and BLITZ
					configurations carry this line
					character for character.
  libcompat, usr.bin: SRCS and SUBDIR	named only what the release
					contains, which is what the file
					lists already imply.
  Makefile.i386: OBJECT_FMT		the conditional's shape is
					sparc/conf/Makefile.sparc's, the
					only one in this tree's kernel
					templates.

## B -- corrected following another BSD's line

The code stays Berkeley's; the line taken exists in a tree that may be
copied, and the commit names it with its release.

  libc: file-scope static declarations, OpenBSD 1996
  lorder: pairs written directly, OpenBSD's current tree
  stdio.h, bsd.prog.mk, bsd.lib.mk, include/Makefile: DESTDIR and
    tool flags, NetBSD 1.0 and OpenBSD 1996
  bin/sh: HOSTCC for the generators, OpenBSD 1996; mknodes's infp,
    NetBSD 1.6
  sbin, libexec: kernel paths inside the tree, FreeBSD 2.0.5 and
    OpenBSD 1996
  ftp, systat: command table after its struct, NetBSD 1.5
  tip, window, routed: static helpers at file scope, NetBSD 1.1 and 1.4
  ps, w: adjacent string literals, NetBSD 1.6
  netstat: ns_nfiles, NetBSD 10
  pstat: the flags this release has, NetBSD 1.4
  vnode.h and vfs_subr.c: void inlines and the DIAGNOSTIC guard,
    NetBSD 1.4 and 1.6
  mem.c: <vm/vm.h>, NetBSD 1.0 through 1.2 and OpenBSD 1996 -- and
    Berkeley's own April 1995 commit moved what it needs into that
    header
  if_ether.c, machdep.c: declarations taken from the headers, NetBSD
    1.0 and FreeBSD 2.0.5; sendsig's u_long, NetBSD 1.1
  newvers.sh: a here document, FreeBSD 4.0
  Makefile.i386: -Ttext, OpenBSD 1996
  kdump: $DESTDIR in mkioctls, FreeBSD 2.0.5; syscalls.c from the
    tree, OpenBSD 1996
  Kerberos made optional: NetBSD 1.0's shape, in eleven Makefiles

## C -- our judgement, no donor line

Each of these is an edit to Berkeley's text that no tree makes. They
are small, and every one says so in its own AI-ONLY NOTE.

  _C_LABEL through the kernel assembly	the macro is NetBSD's and the
					conversion mechanical, but no
					tree converted this tree's files
  mkglue.c emits _C_LABEL		no tree ever made a vector
					generator ELF-safe; they dropped
					generation first
  bad144: dinode.h include		the type is Lite2's own; no
					other tree has the problem
  mklocale, ftpd: -I${.CURDIR}		no tree needs it, their builds
					compiling in the source directory
  ${LIBTERM} and ${LIBRPC} values	the names this tree builds
  config.new: yywrap from -ll		four programs here do it; no
					tree does it in this file

## D -- transplanted, never in 4.4BSD-Lite2

	lib/csu/*		619 lines	NetBSD 1.6
	lib/libkvm/kvm_i386.c	221 lines	NetBSD 1.0, adapted
	vmstat/names.c i386 case  6 lines	NetBSD 1.0
	lib/libl/Makefile	  4 lines	NetBSD 1.0

The csu files are ELF startup code, which did not exist in 1995 and
could not have. The other three fill holes in Berkeley's i386 port:
kvm_i386.c and the names.c case are in no CSRG tree at all, so nothing
removed them -- they were never written.

## E -- a reimplementation of something the Lite cut removed

	lib/libcompat/4.3/regex.c   93 lines	NetBSD 1.0

Berkeley's regex.c is 407 lines of regular expression engine marked
%sccs.include.proprietary.c%. This is not that file: it is a shim that
implements re_comp and re_exec in terms of regcomp and regexec, over
the engine already in this tree. NetBSD wrote it because Berkeley's
could not be redistributed.

## F -- Berkeley's own file, restored

	sbin/savecore/zopen.c	741 lines	NetBSD 1.0

Marked %sccs.include.redist.c% in Berkeley's tree. It is absent from
the Lite releases because usr.bin/compress went whole for the LZW
patent, not for its licence. NetBSD's copy is Berkeley's file, so this
is that code coming back rather than another system's arriving.

## G -- ours

build/make.sh, build/sysroot.sh, the generated specs file,
lib/csu/i386_elf/Makefile, sys/i386/conf/LINK.i386, and everything
under docs/. See precedent.md for the inventory and for what is
claimed over them.

## What was not done, and what it costs

These are refusals, not omissions. Each one gives up something.

  The i386 disk, floppy and tape drivers
	wd.c and fd.c use the 4.3BSD names for the driver queue against
	a struct buf that has not had them since before 4.4BSD-Lite.
	Ten sites, with FreeBSD 2.0.5 -- the same 386BSD lineage -- as
	a model.
	Cost: no disk, floppy or tape. A kernel that cannot mount a
	root filesystem from a local disk.

  The interrupt architecture
	Every other tree moved these vectors out of config into a
	static file with runtime registration, before ELF was a
	factor. Taking that needs the registration table and every
	driver's attach path.
	Cost: interrupts stay wired in at config time, as Berkeley
	had them. Nothing is lost that this tree ever had.

  libcompat's 4.1 routines
	ftime, getpw, gtty, stty, tell, vlimit, vtimes, all marked
	proprietary in Berkeley's tree and removed by the Lite cut.
	Cost: games/trek does not build, wanting gtty.

  The twenty-five programs the SUBDIR lists name
	ed, expr, bc, dc, spell, learn, at, cron, sa, compress and the
	rest, all proprietary-marked or patent-encumbered. Several
	BSDs wrote their own.
	Cost: no editor, no calculator, no scheduler, no compression.

  usr.bin/vacation's libdbm, the fourteen games, the Kerberos chain
	Cost: those programs, and Kerberos authentication.

  vmstat's real disk names
	FreeBSD 2.0.5 has an implementation reading the kernel's
	namelist; NetBSD 1.0's stub is what was taken.
	Cost: vmstat reports drives as ??0 rather than by name.

## The one inversion, stated plainly

741 lines were imported so savecore could compress a crash dump --
a utility no system needs to boot -- while ten sites of adaptation in
wd.c and fd.c, which would give the kernel a disk driver, were
declined as too large.

By functional importance that is backwards, and it was not a
principled choice: zopen.c was one file with a clean donor and the
drivers were a morning's careful work with a model rather than a
line-for-line source. The reason given at the time -- reaching a link
sooner -- was about momentum, not authenticity.

It is recorded here rather than quietly fixed, because the next person
should be able to see where the line was drawn loosely and re-draw it.
