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

## A note on sys/i386, where the ancestry is mixed

The rest of this tree is Berkeley's work with other systems as
donors. i386 is not so simple. On 10 June 1993 Berkeley committed
"update with newer changed from NetBSD" to i386/i386/locore.s and
machdep.c: their own i386 port took changes from a descendant, two
years before this release.

That commit is where two of the faults corrected on this branch came
from. Before it, locore.s had sixteen `movw' loading %ds or %es and
no movl at all, and wrote `movb %al,(%edx)'; after it, six movw
remained, four were movl, and the byte store named %eax. The
corrections here restore what Berkeley's file said before the merge,
which is a better claim than the six surviving movw lines alone
supported.

Counted against the CSRG history rather than from memory, with
`git show 3298e079f31^:usr/src/sys/i386/i386/locore.s' and the same
at that commit. The segment loads come in %ds/%es pairs:

	before   8 pairs, every one movw
	after    5 pairs -- 3 movw, 2 movl

So three pairs are gone, dissolved into the restructuring that merge
also did (the file grew from 1584 lines to 1679), and two pairs were
converted from movw to movl. Four lines changed instruction; six no
longer exist in that form. Nothing was converted the other way.

It also weakens donor evidence in this directory specifically. "NetBSD
1.0 writes it this way" is ordinarily independent confirmation; in
i386/i386/locore.s and machdep.c it may be the same text arriving by
another route, since the two trees share a merge and not merely an
ancestor. Where a claim in this directory rests on NetBSD alone, it
should say so.

Not everything in those files came from the merge: sendsig's `unsigned
code' and the local panicstr declaration in machdep.c both predate it.

The same is true more widely, and the whole of it was measured rather
than sampled. Berkeley's last months carry 98 commits importing from
NetBSD, FreeBSD and 386BSD, many signed by Charles Hannum and Adam
Glass. Of the 117 files changed on this branch, five were touched by
one:

  sys/i386/i386/locore.s	the 1993 NetBSD merge, above
  sys/i386/i386/machdep.c	the same merge; our fixes predate it
  sbin/routed/trace.c		April 1995, "prettiness police", from
				Hannum; did not touch iftraceinit
  usr.bin/kdump/kdump.c		April 1995, KNF and err(3) cleanup,
				from Hannum; did not touch the include
				path changed here
  usr.sbin/config/mkglue.c	January 1991, "386BSD additions to
				config"

The last is worth stating plainly. The i386 vector generator patched
here to emit _C_LABEL was never Berkeley's own work: it came from
386BSD in 1991. That is part of why every later tree abandoned
generating vectors rather than maintaining the generator, and it means
the commit that changed it was extending a contribution rather than
Berkeley's design.

So 4.4BSD-Lite2 is not a pure CSRG artefact. It is Berkeley's tree
with a substantial last round of its own descendants' work merged in,
and in five of the files this branch touches the distinction between
"Berkeley's code" and "another BSD's code" does not hold cleanly.

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
  GENERIC.i386: console is pc0		Berkeley's ARGO and BLITZ
					configurations carry this line
					character for character.
  libcompat, usr.bin: SRCS and SUBDIR	named only what the release
					contains, which is what the file
					lists already imply.
  Makefile.i386: OBJECT_FMT		the conditional's shape is
					sparc/conf/Makefile.sparc's, the
					only one in this tree's kernel
					templates.
  i386 boot(): sync(&proc0, ...)	sparc/sparc/machdep.c's boot()
					in this tree makes this call; the
					i386 passed one null sigcontext
					pointer to the three-argument sync
					and panicked on every reboot with
					a writable filesystem. NetBSD 1.0's
					i386 makes the same call.

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
  machine/stdarg.h: va_start, va_arg and va_end over the compiler's
    builtins, NetBSD current, FreeBSD current and OpenBSD current --
    all three, and NetBSD's sys/sys/stdarg.h still carries this file's
    SCCS identifier. Of the five lines adopted, 2 are byte-identical
    to FreeBSD's, 1 to NetBSD's, 0 to OpenBSD's: the macro signatures
    C fixes over the builtin names GCC fixes. docs/provenance/stdarg.md
  bin/sh/cd.c: getpwd by getcwd(3) rather than a forked /bin/pwd,
    NetBSD current (whose cd.c carries this file's SCCS identifier)
    and FreeBSD current. This one is a body and not a line: 9 of 20
    lines are NetBSD's, longest identical run 4 -- the stalloc,
    getcwd, savestr, stunalloc core. docs/provenance/pwd.md
  i386 pmap.c: XXX_mmap for Berkeley's PTE pointer `mmap', NetBSD 1.0
    (FreeBSD 2.0 calls it ptmmap). As a common symbol the variable
    was resolved to the mmap(2) function, and pmap_bootstrap wrote a
    PTE address over that function's first four bytes: every mmap
    call panicked with `trap type 3'.
  libterm: tgetent's dummy pointer starts NULL, NetBSD 1.0 and
    FreeBSD 2.0; an unknown terminal freed stack garbage and tset
    died at every login.
  ctype.h: the functions behind the classifier macros declared,
    FreeBSD 2.0.5's lines. libc defines them in locale/isctype.c and
    C requires them to be functions; awk takes their addresses.
    tolower and toupper left out, being static inline here.
  i386 isa.c: isa_strayintr logs the irq number, and only five times,
    NetBSD 1.0's intr.c; FreeBSD 2.0 stops at five per irq. Berkeley's
    took ffs() of the number as if it were a mask and logged every
    one -- seventeen thousand lines on the console during one install.
  i386 fd.c: Sense Interrupt after reset, recalibrate and seek, and
    the status judged by its interrupt code alone, FreeBSD 2.0's
    fd.c; nec765.h's ST0 fields, FreeBSD 2.0's
  i386 isa.c, pmap.c: isa_dmastart, isa_dmadone, isa_dmarangecheck
    and isaphysmem in place of at_dma, FreeBSD 2.0's (NetBSD 1.0 and
    OpenBSD 1995 carry the same); ic/i8237.h, FreeBSD 2.0's
  i386 machdep.c: the TSS's I/O map past its end, NetBSD 1.0 and
    FreeBSD 2.0
  sysctl.h, vfs_syscalls.c: debug1 and syncprt under DIAGNOSTIC,
    FreeBSD 2.0
  amd fsinfo: no strdup declaration of its own, NetBSD 1.3
  tn3270: generators built with HOSTCC and found by .OBJDIR, NetBSD 1.1
  fpr: gettext defined void, NetBSD 1.4
  i386 psl.h: PSL_MBZ, PSL_USERSET, PSL_USERCLR, NetBSD 1.0 and
    FreeBSD 2.0; PSL_USERSTATIC and sigreturn's test of the flags,
    OpenBSD 1995
  top-level Makefile: kerberosIV only with KERBEROS, as NetBSD 1.0
  i386 MAKEDEV: `all', NetBSD 1.0, FreeBSD 2.0 and OpenBSD; the serial
    ports as tty0N, NetBSD 1.0's entry (FreeBSD 1.x and OpenBSD 1995
    alike), where Berkeley's matched hp300's `dca' and made nothing
  vnode_if.src, link(2), nfsrv_link, union_link: vop_link takes the
    directory first, FreeBSD 2.2 and 3.0
  top-level Makefile: build and afterinstall by ${.CURDIR} and
    ${MAKE}, FreeBSD 2.0, and its own makes from ${.CURDIR}, FreeBSD
    3.0
  zoneinfo, fortune/datfiles: zic and strfile by name, from the
    tools the build makes first, FreeBSD 2.0 and 4.0 and FreeBSD
    2.2's build-tools; the rot13 by tr, FreeBSD 4.0
  boggle: mkdict and mkindex found in their object directories,
    OpenBSD 1995; the words from the tree's web2, NetBSD 1.6
  phantasia: setup writes where it runs, and the Makefile installs
    under DESTDIR, FreeBSD 2.0; setup built for the build machine,
    FreeBSD 4.0's build-tools
  bsd.prog.mk: a hidden game's link not chowned, NetBSD 1.0 (cgd)
    and OpenBSD 1995
  share/dict, tmac, man5, man7: what the Lite cut took dropped from
    the lists, and words a link to web2, FreeBSD 2.0, NetBSD 1.0 and
    OpenBSD 1995 alike
  etc/mtree: 13.viref among usd's directories, OpenBSD 1995; and
    11.vitut, 12.exref and smm/19.perl the same way
  share/man: makedb reads the pages under DESTDIR, FreeBSD 2.0,
    NetBSD 1.0 and OpenBSD 1995
  libc db hash.h: the errno field named err, NetBSD 1.3 and OpenBSD
  usr.bin: Lite2's a.out ar, ranlib, nm, size, strip, strings, gprof
    and gcore not built where binutils stands in, OpenBSD's
    ELF_TOOLCHAIN line and NetBSD 1.4
  contrib/gawk builtin.c: initstate, setstate and random declared only
    where the library lacks them, OpenBSD 1995
  contrib/flex: installed as /usr/bin/lex, with flex and flex++ and
    the page as lex.0 and flex.0, NetBSD 1.0's names
  contrib/: programs built in object directories, GNU's Makefiles run
    there as OpenBSD's Makefile.bsd-wrapper runs them
  share/mk/bsd.subdir.mk: a directory's Makefile.bsd-wrapper read in
    place of its makefile, OpenBSD (niklas, December 1995) -- for
    contrib/perl-4.036, whose makefile Configure wrote
  contrib/bind named/ns_func.h: doupdate's and validate's last
    arguments without the #ifdef inside __P, as Vixie's next
    ns_func.h (rev 1.12, July 1994, FreeBSD 2.0) has them
  contrib/perl x2p: its unused setenv renamed my_setenv, as perl's
    own util.c names the same routine, clear of <stdlib.h>'s
  xinstall: the target reopened after strip, NetBSD's xinstall.c 1.9
    (jonathan, December 1995; NetBSD 1.2). binutils' strip writes a
    new file and renames it over the old, so Lite2's install set the
    mode through a descriptor on a file already gone, and every
    program it installed with -s was left mode 0600
  include/Makefile: headers copied by default, FreeBSD 2.2.5 to today
    (SHARED?= copies, in the same Berkeley file, 8.2 1/4/94), by the
    maintainer's choice; SHARED=symlinks still chooses Berkeley's,
    whose machine lines are FreeBSD 2.0's: rm -rf, and a link to /sys
    without DESTDIR
  zoneinfo, fortune/datfiles: Lite2's zic, strfile and caesar kept as
    the defaults, named as variables as NetBSD 1.0 names ZIC and
    STRFILE and FreeBSD 2.2 CAESAR, set by ?= as NetBSD 1.6 sets ZIC,
    so that build/make.sh gives the build machine's: the tree's zic and
    strfile built for it, and tr for the rot13, as OpenBSD 1996,
    NetBSD 1.6 and FreeBSD 4.0 make it
  contrib/gcc-2.95.3: /usr/libexec/gcc2/cpp GCC's cpp driver, run by
    Lite2's usr.bin/cpp script, as OpenBSD's script ran it as
    /usr/libexec/cpp from 1999 (espie); GCC 2.95 gives the target's
    predefinitions through its drivers, and cccp alone has none
  contrib/gcc-2.95.3: its passes, libgcc and headers in
    /usr/libexec/gcc2 itself, as Lite2's gcc-2.3.3 Makefile made that
    directory its libsubdir; found by MD_EXEC_PREFIX, as GCC's own
    netbsd.h finds NetBSD's in /usr/libexec; its directories in
    etc/mtree, and made by the install only if missing, by gcc-2.3.3's
    own line
  contrib/binutils-2.11.2: strip stripped by the strip already there
    and installed without -s, FreeBSD 3.0 (jdp, 1998)
  contrib/: bind, MH, groff, perl, GCC and binutils built in place,
    as Berkeley built its contributed software (contrib/Makefile gives
    contrib no object directories; gcc-2.3.3's makefile, gdb-4.7.LBL's
    ./configure), and in a copy of their own only with
    MAKEOBJDIRPREFIX, as OpenBSD's perl wrapper builds in place unless
    there is an object directory (downsj, 1996)
  mkdir: -p takes a directory already there, FreeBSD 2.2.5's build()
    (rev 1.5.8.1, 1997), which extends Berkeley's loop to the last
    component; FreeBSD 2.0's `if (!pflag)' hid every error and FreeBSD
    dropped it; NetBSD 1.1's mkpath replaced Berkeley's routine
  nvi: vi, ex and view in /usr/bin, linked to nvi, FreeBSD 2.0's two
    LINKS lines, and its pages linked as FreeBSD 2.2's: where
    <paths.h>'s _PATH_VI and mail's _PATH_EX name them, and where
    Berkeley's vi and ex, which usr.bin/Makefile still names, were
  i386 Makefile.i386: SYSTEM_LD and SYSTEM_LD_TAIL in Berkeley's own
    hp300 form, if/then/else and `|| true', as NetBSD 1.0's i386
    Makefile has them; the i386's && forms failed under Lite2's sh -e
  sys/compile: in the tree and the source set, empty but for
    .keep_me, as FreeBSD 2.0, NetBSD 1.0 and OpenBSD ship it
  bin/sh eval.c: a failed first command of an && list no longer
    ends the shell under -e, the three lines FreeBSD's eval.c 1.6
    (steve, 1996) and NetBSD's 1.31 (christos, 1996, PR 2808) both
    added, comment and all; FreeBSD 2.2 and NetBSD 1.2.1
  bin/sh parser.c: a command of redirections only before && and ||,
    TAND and TOR as NetBSD's parser.c 1.30 has them (christos, 1996,
    PR 2808, "from FreeBSD"), FreeBSD's since 1995, FreeBSD 2.1
  share/zoneinfo: REDO=posix_only, NetBSD's from 2 September 1993
    (cgd), NetBSD 0.9 on; OpenBSD the same, FreeBSD 2.0 likewise
    unless LEAPSECONDS
  contrib/rcs-V5.6: GNU diff's lines of Berkeley's configuration
    section, the values NetBSD 1.0's and FreeBSD 2.0's conf.h have;
    conf.h made on the system by conf.sh and kept, as FreeBSD 2.0
    kept Wollman's (1994), with the Makefile's own conf_h
  ps_strings: the argv and envp vectors' addresses, not their first
    strings -- sys/exec.h's char **, kern_exec.c, and libkvm's
    kvm_argv reading the strings through them -- NetBSD 1.1's
    (Charles M. Hannum, 16 to 25 May 1995), FreeBSD 2.2 the same
  contrib/emacs-18.57: unexelf.c's ELF definitions from
    <sys/exec_elf.h> with three names mapped, as GNU's emacs 20.1
    (1997) has it for NetBSD; xemacs installed without -s, as GNU's
    own 18.59 Makefile installs it; the ELF settings in one #ifdef
    __ELF__ block in s-bsd4-4.h, as emacs's s/netbsd.h and
    s/freebsd.h hold theirs, each as one of emacs's own files has it
  etc/root/dot.profile: TERM given a value before Berkeley's `export
    TERM for single user shells', FreeBSD 2.2's ${TERM:-cons25}, here
    pc3, Lite2's termcap entry for the console

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
  contrib bib, hunt, jove, kermit:	a makefile of their own, with
    their Makefiles' programs,		the same objects and CFLAGS,
    objects and CFLAGS			since those compile $*.c or
					name sources in the directory
					they run in; no tree built
					these four in object directories
  contrib/jove: setmaps by LOCALCC,	the Makefile's own provision for
    ${HOSTCC} -ansi			cross-compiling; -ansi keeps a
					Linux libc's getline() out
  contrib/kermit: wart by ${HOSTCC}	as HOSTCC builds what a build
					runs; its ckcpro.c compared
  contrib/mh: Berkeley's configured	the descendants' ports build in
    Makefiles, in a copy of the		a copy of the sources; the PATH
    sources in the object directory	gives cc, ld, ar and ranlib
					their target's, from GCC's and
					binutils' tooldir
  contrib bind, perl: their own	in a copy of the sources, as
    Makefiles, run in a copy		MH's; perl's header lists name
					the target's under DESTDIR, and
					it is installed by name, since
					installperl runs ./perl
  build/sysroot.sh: grops and devps	for MH's paper, which its own
    in the build machine's groff	`make all' formats, as on
					Berkeley's machine

## D -- transplanted, never in 4.4BSD-Lite2

	lib/csu/*		619 lines	NetBSD 1.6 (see below)
	lib/libkvm/kvm_i386.c	221 lines	NetBSD 1.0, adapted
	vmstat/names.c i386 case  6 lines	NetBSD 1.0
	lib/libl/Makefile	  4 lines	NetBSD 1.0
	lib/libc/gen/nlist_elf32.c	NetBSD 1.3, adapted
	usr.sbin/kvm_mkdb/nlist_elf32.c	NetBSD 1.3, adapted
	sys/exec_elf.h: Shdr, Sym	NetBSD 1.3
	sys/boot/i386			FreeBSD 4.1, adapted
	usr.sbin/btxld			FreeBSD 4.1, adapted
	i386/autoconf.c: MFS_ROOT  4 statements	FreeBSD 2.0.5
	release/write_mfs_in_kernel.c	FreeBSD 2.0.5, adapted
	usr.sbin/crunch			FreeBSD 3.0, adapted
	distrib/i386/floppies		NetBSD 1.1, adapted
	i386/isa/wd.c, atapi.c, wcd.c	FreeBSD 2.1.0, adapted
	i386/isa/if_ed.c, if_edreg.h	FreeBSD 2.1.0, adapted
	the slice layer, 4 files	FreeBSD 2.1.0, 2 lines changed
	config: isa_biotab		FreeBSD 2.1.0, adapted
	contrib/gcc-2.95.3		GNU's release, NetBSD 1.6's changes, a target
	contrib/binutils-2.11.2		GNU's release, NetBSD 1.6's changes, a target
	lib/libcompat/4.1: gtty, stty, ftime	NetBSD 1.0
	usr.bin/m4/m4.1			NetBSD 1.0
	kern/sys_process.c		FreeBSD 2.2, adapted
	i386/i386/procfs_machdep.c	Pendry, as NetBSD had it (Jan 1994), adapted
	i386 machdep.c: ptrace_set_pc, ptrace_write_u	FreeBSD 2.2, adapted
	i386 ptrace.h: PT_GETREGS family	FreeBSD 2.2
	i386 reg.h: struct reg, NetBSD (1993); struct fpreg, FreeBSD 2.1.0
	lib/libc/sys/ptrace.2		FreeBSD 2.2, adapted
	kern/genassym.sh, sys/assym.h	FreeBSD 5.0, unmodified
	contrib/diff-2.6		GNU diff 2.6, NetBSD 1.0's, the system's diff
	contrib/grep-2.0		GNU grep 2.0, NetBSD 1.1's, the system's grep

The csu files are ELF startup code, which did not exist in 1995 and
could not have.

Why 1.6 and not something closer to this tree's era, which every other
choice here prefers: NetBSD 1.4 has an ELF csu, three years earlier,
and it is the wrong one. Its crtbegin.c is 123 lines against 1.6's
201, the difference being DWARF2 exception-frame registration, which
the compiler this tree is built with expects a crtbegin to do; its
crtend.c is ten lines against 26. ELF startup code tracks the
toolchain and not the tree, so here later is closer, and this is the
only place in this project where that is true.

The other two are Berkeley's code that reached this tree through
NetBSD rather than NetBSD's own work, and both carry Regents
copyrights. kvm_i386.c is Berkeley's kvm_hp300.c ported to a machine
Berkeley did not do: the same four functions in the same order.
NetBSD 1.0 is not merely the earliest source for it but the only
workable one -- this tree's kvm.c calls _kvm_freevtop, _kvm_initvtop,
_kvm_kvatop and _kvm_uvatop, and of the four trees only NetBSD 1.0
provides all four. NetBSD 1.1 and OpenBSD 1996 had dropped
_kvm_uvatop; FreeBSD 2.0.5 has _kvm_vatop in its place.

The other three fill holes in Berkeley's i386 port:
kvm_i386.c and the names.c case are in no CSRG tree at all, so nothing
removed them -- they were never written.

**Reading an ELF kernel's symbols.** This kernel is ELF, and nlist(3)
and kvm_mkdb read only a.out: at every boot savecore said `/vmunix:
nlist: Undefined error: 0' and kvm_mkdb `bad magic number', and
everything that names kernel variables -- savecore, dmesg on a dump,
vmstat, iostat, netstat, pstat -- went through the same nlist.

Surveyed: FreeBSD 3.0 keeps one nlist.c with its a.out reader and an
ELF reader side by side, which made it the first choice, but its ELF
reader maps the section headers and the string table at their own
file offsets, and this kernel's mmap refuses an offset that is not
page-aligned, as 4.4BSD's did; and for every local symbol it sets
n_type to N_EXT alone, losing the type, while no global gets N_EXT.
NetBSD 1.2 has no ELF reader. NetBSD 1.3 (Christopher G. Demetriou,
1996) maps the whole file from offset 0, checks the ELF header, the
section header table and the symbol and string sections against the
file's size -- not each symbol's name offset, nor sh_link -- and has
the binding the right way round, as OpenBSD's current nlist.c also
does; and the tree's sys/exec_elf.h is
already NetBSD's, revision 1.1 from NetBSD 1.1. NetBSD 1.3's readers
were taken, for libc and for kvm_mkdb, with what they need from
NetBSD 1.3's revision of the same header: Elf_Byte, the section types,
Elf32_Shdr, Elf32_Sym and the symbol binding and type enumerations.

Shrink-wrapped to one size and one machine: NetBSD builds each reader
twice through ELFSIZE and the ELFNAME macros, and chooses its formats
in nlist_private.h and its machine cases in elf_machdep.h; here the
32-bit names are written out, the two formats are listed in each
nlist.c, and the i386 cases are spelt as this tree's exec_elf.h spells
them. Berkeley's a.out readers stay, renamed __fdnlist_aout and
create_knlist_aout, the latter returning -1 for a file that is not
a.out, with NetBSD 1.3's two lines, so the ELF reader can be tried.
kvm_mkdb's exits are Berkeley's errx and err where NetBSD calls a
punt() this tree does not have, and one NULL test is added, from the
libc reader of the same release. The licence is Demetriou's
four-clause one, with an advertising clause.

The header's own note claimed revision 1.1 had Elf32_Shdr and the SHT_
constants; it has neither. Corrected there.

**Boot blocks that read ELF.** Lite2's own, sys/i386/stand, read an
a.out kernel through disk drivers of their own and load it at
physical 0; this kernel is ELF, and an ISO is booted through the
BIOS. Robert Nordier's boot1 and boot2 (1998) read an ELF kernel from
a UFS root through the BIOS, under BTX, a small protected-mode
supervisor that makes the BIOS calls in virtual-8086 mode; they load
each segment at its physical address, so the kernel was first moved
to 1 MB, as every descendant's i386 kernel had been before Lite2
shipped (commit *i386: the kernel at physical 1 MB*); 4.1's boot2.c
is 805 lines with nothing beneath it but BTX.

The lineage, and why none of it was taken. The i386 BSDs' common
loader through the Lite2 years was the CMU Mach boot Julian Elischer
ported to 386BSD in the 1992 patchkit (sys.386bsd/i386/boot, its
README.386BSD: "uses the BIOS to load the kernel"): NetBSD 0.9 to
1.2 shipped it as sys/arch/i386/boot, FreeBSD 1.0 to 4.7 as
sys/i386/boot, its biosboot directory from 2.0. It reads a.out only, in every release --
FreeBSD 3.0's sys/Makefile calls it "the old aout only boot loader"
-- so it would need an ELF loader written for it. Lite2's own
sys/i386/stand, Jolitz's, drives the disk controllers itself, which
an emulated CD floppy does not have. Every loader that reads ELF is
new code of 1997-2003: Nordier's boot2 (FreeBSD 3.0, 1998);
Matthias Drochner's libsa-based NetBSD boot, new in 1.3 (1997),
reading ELF from 1.4 (1999); OpenBSD's, ELF from 2003, for 3.4.

NetBSD 1.4's was tried before this choice was confirmed. Built with
GCC 14 from netbsd-1-4 -- two flags, -fno-reorder-functions and
-fno-aggressive-loop-optimizations, and 22 assembler lines respelt --
it booted this tree's disk to the login prompt, and passes the same
howto and bootdev through its COMPAT_OLDBOOT path. Not taken: it is
105 files and some 14,700 lines against FreeBSD's 3,240, and built
against NetBSD's own headers; it is 44 KB, of which 7.5 KB fits the
boot area and the rest is a /boot file found through a block list
that installboot writes, and installboot is a NetBSD program that
mounts the filesystem and asks for the label by ioctl, so it does not
run on the build host and nothing here could stand in for it without
writing one; and its third argument to the kernel, where this kernel
reads cyloffset, is the address of NetBSD's bootinfo, harmless only
because wd.c ignores a value that large. FreeBSD's fits the 8 KB
boot area and is installed with dd, by disklabel -B, or into a
floppy image for El Torito.

FreeBSD 3.0 was the first release with them, and is not the one
taken: its btx.s and boot1.s were written for the assembler of the
day, with m4 macros for 16-bit code and operand sizes current GNU as
rejects. 4.1 (July 2000) is the first whose sources use `.code16`
and assemble unchanged. Taken from it: boot1.s, boot2.c, lib.h,
sio.s, btx.s and BTX's client library -- btxcsu.s, btxsys.s,
btxv86.s, btxv86.h -- and btxld, the host program that joins boot2
to BTX. Not taken: btxldr, the third-stage loader and libi386, mbr,
boot0, kgzldr, cdldr, pxeldr, and btxld's ELF header template.

Shrink-wrapped, each change noted at its site: boot2 reads ELF only,
one PT_LOAD or two; no bootinfo, BIOS probes or symbol table, which
this kernel does not read; no third stage; no DOS slices, since
Lite2's wd driver reads its label from sector 1 and knows no
partition table; the flags this kernel has (-a -s -r -d) passed and
the console ones kept; this tree's majors in the boot device; and
/vmunix. btxld writes `-f bin' only.
The Makefiles are FreeBSD's, for this tree's make and GNU binutils;
boot2's says why each flag changed, -fno-reorder-functions above all.
The boot blocks install to /usr/mdec under disklabel's names.

The licence of boot1, boot2, BTX and lib.h is Nordier's own: use and
redistribution provided the notice and disclaimer are kept. btxld's
is the two-clause BSD licence.

**A root inside the kernel.** Berkeley's MFS already has a miniroot:
mfs_initminiroot() takes a filesystem in memory and makes it the root,
and pmax and news3400 call it for an image their boot program loads
after the kernel (pmax/stand/mkboottape.c; share/doc/smm/01.setup/
2.t). boot2 loads one file, so the image goes inside the kernel
instead, which is what FreeBSD's boot floppy did from 2.0.5 (June
1995) and, unchanged, in 2.1.0 (January 1996): an initialised array, `options MFS_ROOT=n` KB, marked by a string, and
Poul-Henning Kamp's write_mfs_in_kernel to write an image over it
after the link. Taken: the three declarations of the array and the
call to mfs_initminiroot in configure(), and the program, under its
own beer-ware licence, with SBOFF written out and <string.h> added so
it builds on the host, and a size check added before its loops.
Setting RB_DFLTROOT with a miniroot is Berkeley's, from pmax.

Why not the i386's own precedents. Every i386 install medium of the
Lite2 years kept its root on the floppy it booted from: Berkeley's
sys/i386/floppy in Lite2 itself (1993, 31 small programs), FreeBSD
1.x's kernel-copy and filesystem floppies, NetBSD 1.0 to 1.2's
install floppies, FreeBSD 2.0's boot floppy. A kernel booted from a
CD in floppy emulation cannot read that floppy: the emulation is the
BIOS's, and the kernel's fd driver talks to the controller. The root
has to be in memory. NetBSD reached that with a RAM-disk driver,
sys/dev/ramdisk.c in 1.1 (for the sun3) and md and mdsetimage for
the i386 in 1.3; OpenBSD with its ramdisk floppies of 1997. Each is a
block driver beside MFS; FreeBSD's is MFS, which Lite2 has. The image
itself is made with the host's makefs, NetBSD's, as mtree is.

**One binary for the install root.** James da Silva's crunch
(University of Maryland, 1994) links many programs into one binary
that runs the one its argv[0] names, so they share one copy of libc:
it is how the BSDs' install floppies were made. It first shipped in
FreeBSD 1.1.5 (contrib/crunch, crunch 0.2, June 1994) and NetBSD 1.0
(distrib/crunch, 0.3); crunchgen.c changed little after -- NetBSD
1.3's differs from 1.0's in some sixty lines.

The donor is set by crunchide, which hides each program's globals but
its entry point, and so must read the object format. The a.out ones,
FreeBSD 1.1.5 to 2.2.8 and NetBSD 1.0 to 1.2, cannot read this tree's
objects. NetBSD 1.3 has an ELF back end, exec_elf32.c, but its
extern.h enables it on the i386 only after the i386 itself went ELF;
and the netbsd-1-3 branch's revision of it, 1.4.2.1 of 1998, renames
symbols and refuses an object whose string table is not the last
section -- which a current `ld -r' never makes, since it puts
.shstrtab last. FreeBSD 3.0 (October 1998), the first BSD release
whose i386 is ELF, carries NetBSD's earlier revision, 1.4 of August
1997, which turns the hidden globals into locals and has no such
condition, and enables it for the i386. Measured: built on the host,
it hides a duplicate global in `ld -r' output and the two link. So
FreeBSD 3.0's crunchgen and crunchide are taken, without the example
configurations, the a.out back end or the ELF64 one.

Adapted, each change noted at its site: crunchide is ELF32 alone and
builds against the host's <elf.h> with build/elf-host.h, which names
the aliases FreeBSD's ELF headers gave it; crunchgen takes the object
directory from ${MAKEOBJDIRPREFIX}, which FreeBSD's own read by 4.3,
where 3.0 wrote /usr/obj, and links with ${LD}, as NetBSD's did from
1.4. Both are host tools, and build/sysroot.sh gives build/make.sh's
PATH a `make' that is bmake, the name crunchgen runs. The licence is
the University of Maryland's permission notice, and the crunchide
back end Christopher Demetriou's.

**The install root and its installer.** NetBSD's i386 install
floppy, distrib/i386/floppies, Christopher Demetriou's of 1994: the
crunchgen configuration of its one binary, the list of what goes on
it with the two tools that run the list, and the installer, install.sh
with its profile and the set-loading functions. The same tree is in
NetBSD 1.0 (October 1994), with other kernel-copy floppies, and
changed little to 1.2; it is NetBSD's own i386 installer of the Lite2
years. NetBSD 1.2's distrib/miniroot, considered first, is a
machine-independent installer; that release carries per-port parts
for atari, hp300 and sparc, and the Makefile fragment that builds it
for atari alone. NetBSD 1.1 (November 1995) is taken, the first
release after Lite2 that carries it. Not taken: the kernel-copy and
upgrade floppies.

Adapted: built on the host into a directory and made into an image by
makefs, sized for the install kernel's MFS_ROOT, with its device nodes
in an mtree specification instead of a MAKEDEV run; the program list
less what this tree has no source for, and less more, with awk, pax as
tar and -lrpc. Each change is noted at its site, and install.md, *The
install root*, has the build. The installer is adapted in place: the
system's name, IDE as the default, pax for GNU tar, ufs in fstab,
Configure writing Lite2's netstart and making the password and
termcap databases through chroot; install.md, *Installing with the
installer*, has a whole install. The licence is Demetriou's
four-clause one, with an advertising clause.

**The network card: if_ed.** Lite2's i386 has 386BSD's Ethernet
drivers, if_ne for the NE2000 and if_we for the WD8003, and if_ne,
the one QEMU's ne2k_isa answers, waits for ever with the interrupts
blocked when a packet's length is odd: nefetch asks the card for len
bytes and reads len/2 words, so the transfer never ends (neput, its
send side, rounds up). No descendant carried either: FreeBSD from 1.0
and NetBSD 1.0 to 1.2 carried David Greenman's if_ed (1993), one
driver for the 8390-based cards -- NE1000 and NE2000, WD/SMC 8003 and
8013, the 3Com 3c503 -- and OpenBSD took NetBSD's. NetBSD's is
written to Torek's new autoconfiguration, which Lite2's i386 does not
have; FreeBSD's to the old isa_driver and isa_device this tree's i386
uses, unchanged in their fields through FreeBSD 3.5.1 (decided
2026-10-18; roadmap.md, *Tier 2*). So FreeBSD 2.1.0's if_ed.c and
if_edreg.h are taken, the release this tree's wd.c is from: the
import commit is them as shipped, the next adapts them. Cut:
FreeBSD's devconf registration, as from wd.c; <machine/clock.h>,
for DELAY, which Lite2's <machine/param.h> defines; isa_driver's
sensitive_hw, which Lite2's lacks; and SIOCSIFMTU, which Lite2 does
not have. An address is given as Lite2's if_we gives it, ac_ipaddr
and arpwhohas, for FreeBSD's arp_ifinit: Lite2's in_ifinit sets the
ARP route hooks itself. A received packet's type is put in host
order before ether_input, as Lite2's drivers put it and Lite2's
ether_input takes it; FreeBSD's ether_input converts it itself, and
without the change every packet was dropped. And the i386's mbuf
clusters are 2048 bytes, as Lite2's hp300's and sparc's are and every
descendant's i386 is, where 386BSD's were 1024: if_ed takes a frame to
fit in one cluster, and dropped longer ones as corrupt. Left: the hooks init, start, reset and
watchdog return void and ioctl takes an int, where Lite2's ifnet says
int and u_long; nothing uses what they return. GENERIC.i386 has ed0
and ed1 at Berkeley's ne0's and we0's ports, interrupts and memory;
if_ne.c and if_we.c stay, and files.i386 still names them. The
licence is Greenman's own notice.

**The disks: IDE, an ATAPI CD, and slices.** Lite2's i386 wd.c is
386BSD's: one controller, one drive, no CD. A CD drive on a PC is an
ATAPI drive on an IDE channel, and that is all QEMU emulates -- no
ISA SCSI, Mitsumi, Sony or Panasonic. ATAPI support was written by
Serge Vakulenko of Cronyx (atapi.c version 1.5 and wcd.c 1.8, both
of September 1995), and FreeBSD 2.1.0, of November 1995, is the first
BSD release to carry it. NetBSD's and OpenBSD's ATAPI is Manuel
Bouyer's, in OpenBSD's tree from June 1996 and in NetBSD 1.3, and is
written to NetBSD's growth of Torek's new autoconfiguration --
cfattach, disk_attach, isa_intr_establish, none of which Lite2 has;
Lite2 carries Torek's cfdriver and config_found, for sparc alone. Taking
it means converting the i386 port's autoconfiguration first, and then
some. FreeBSD
2.1.0's drivers are written to the old i386 one, isa_device and
isa_driver, which this tree has. So FreeBSD 2.1.0's wd.c, wdreg.h,
atapi.c, atapi.h and wcd.c are taken, with the two things its wd.c
cannot be had without: <sys/cdio.h>, and the slice layer --
kern/subr_diskslice.c, kern/subr_dkbad.c, i386/isa/diskslice_machdep.c
and <sys/diskslice.h> -- which its wd.c opens, reads and checks every
transfer through. The import commit is them as shipped; the next
commit adapts them.

The slice layer reads the MBR's partition table and gives each BSD
slice its own label. A disk with no table -- this tree's, labelled
whole by disklabel -B -- is one slice, the whole disk, and the
compatibility slice that the old minors name, unit * 8 + partition,
is it. FreeBSD 2.1.0's sys/types.h widened minor() to keep bits 16
to 31, where dkslice() reads the slice; with Lite2's 8-bit minor()
every dev reads as the compatibility slice, and dsinit, which reads
the table through the whole-disk slice, read nothing. Its minor() is
taken.

Adapted, each change noted or plain at its site. Cut: FreeBSD's
devconf registration (kern_devconf, never in Lite2), the BIOS
geometry branch that reads its boot loader's bootinfo, and wd.c's
optional 32-bit VLB transfer path, whose insl and outsl this port's
locore.s does not have -- cut rather than written, so the transfers
are 16-bit, as they are by default in FreeBSD too. wddump declares
Maxmem itself, which FreeBSD's <machine/md_var.h> declared, and calls
no cncheckc, which Lite2's console lacks; wdattach fills no dk_names,
which Lite2 lacks. Taken
alongside from the same release: GENERIC's controller and disk lines;
files.i386's and conf/files' entries; the isa_device id_flags field
and IO_WDCSIZE; <sys/conf.h>'s d_*_t types, <sys/systm.h>'s timeout_t,
<sys/buf.h>'s b_pblkno and iodone_chain, <sys/disklabel.h>'s
dos_partition and minor-number macros, <sys/dkbad.h>'s in-core table,
and ufs_disksubr.c's correct_readdisklabel and correct_writedisklabel;
MAKEDEV's wcd entry, at this tree's majors; and config's isa_biotab(),
which writes the per-drive table wd.c walks, in Lite2's isa_device
layout. conf.c gives wd a close routine, as the slice layer needs to
count opens, and wcd block major 5 and character 15. Debugger()
becomes panic() in one check, and diskslice_machdep.c defines
offsetof as this tree's <stddef.h> does, which the kernel does not
include. The licences: wd.c is the Regents' (from William Jolitz's
contribution), atapi.c and wcd.c carry Cronyx's permission notice,
and the slice files are Bruce Evans's, with the Regents' on the parts
taken from Berkeley's disk code.

Not changed, and measured: with no drive present, wd.c's probe waits
out a ten-second timeout -- once for each absent drive, and twice
more in probing a channel whose master is ATAPI -- so under QEMU, with
one disk and a CD, autoconfiguration takes about forty seconds, and
with two disks and a CD about thirty. FreeBSD 2.1.5 ships the same
wd.c.

Nor this: <sys/dkbad.h> now gives DKBAD_MAGIC as FreeBSD's 0x4321,
the bt_flag its bad144 writes and subr_dkbad.c checks, while this
tree's bad144, Lite2's, defines its own as 0, after the header, with
a warning for the redefinition -- so a table it writes is one the
kernel declines. Bad-sector forwarding applies only to a label flagged
D_BADSECT, and nothing here makes one.

Nor these. wd.c takes iostat slots while dk_ndrive is below
DK_NDRIVE, counting up from 0 as FreeBSD's does; Lite2's kern_clock.c
starts it at DK_NDRIVE, so the disks keep no statistics -- as Lite2's
own wd.c kept none. And the wider minor() is a signed int, as in
FreeBSD: a dev with bit 31 set, which only mknod or a Rock Ridge
device entry on a mounted CD could make, reads as a negative minor,
which a driver that checks only `minor(dev) >= N', as tty_pty.c
does, lets through.

**The toolchain: GCC 2.95.3 and binutils 2.11.2.** What 4.4BSD built
itself with was GNU's -- sys.mk's `CC ?= gcc', contrib/gcc-2.3.3 and
contrib/gas-1.38, GNU ld in usr.bin/ld -- and so is this: the pair
NetBSD 1.6 built itself with, in contrib/gcc-2.95.3 and
contrib/binutils-2.11.2 as Lite2's contrib/ holds its own GNU
programs, one directory per release, named for its version. Why these
two, and why GNU's at all, is in roadmap.md, *self-hosting*.

Each came in as three kinds of commit. GNU's release, whole and
untouched: gcc-2.95.3.tar.gz, whose MD5 is the one GNU's md5sum.txt
gives it, and binutils-2.11.2.tar.gz, every file of which matches the
release's own md5.sum and, but for the CGEN sources and the md5.sum,
the 2.11.2a re-release that GNU's key signs. Then NetBSD's changes,
as its netbsd-1-6 branch last had them, recovered by comparing its
gnu/dist/toolchain with each release, file by file and byte for byte
(the commit messages say how a file of NetBSD's single tree of GCC,
binutils and gdb was assigned to one release or the other): GCC's,
"2.95.3 20010315 (release) (NetBSD nb3)", 78 files changed, 18 added
and 6 removed; binutils', "2.11.2nb1", 114 changed and 30 added. Then
this tree's own, each its own commit: the system's name,
i386-unknown-bsd4.4elf, in bfd, gas, ld and GCC, where GCC's
i386/bsd44-elf.h takes NetBSD's i386 ELF definitions whole -- the
csu and the header types are NetBSD 1.6's -- and changes only the
names predefined (__BSD_4_4__, as GCC's own m68k/hp3bsd44.h has them)
and the link, static; struct relax_type moved in gas as binutils 2.16
moved it, for the build host's compiler; and bfd/po's two lists the
release left out. The licence is the GPL, version 2, with the LGPL or
GCC's runtime exception on the libraries, file by file.

Built on the build host by build/sysroot.sh as a cross toolchain, the
way NetBSD 1.6's tools/toolchain built it, and used for everything:
every library and program of the release, the startup files, the
boot blocks and both kernels, with the CD it makes installing a
system that comes up multi-user. Four things in the tree outside
contrib/ needed GCC 2.95: <machine/stdarg.h> and <machine/ansi.h>
give it a char * va_list as NetBSD 1.6's do; tip's signal handlers
take the int their prototypes say; and lib/csu and boot2 lost flags
that were for GCC 14. binutils is also built to run on the system,
by `make build', from a README.BSD and makefile in its directory, as
Lite2 built contrib/gcc-2.3.3 and gdb-4.7.LBL, and installs fourteen
programs in /usr/bin in place of Lite2's a.out ones (patch 153). Not
yet: GCC the same way, and the system building the toolchain itself.

**Removed: pcc and binutils 2.16.1.** The toolchain first chosen, for
its licence: pcc 20220331 in NetBSD's framework, with a target made
from pcc's os/litebsd, and NetBSD 5's binutils 2.16.1 cut to the
i386 ELF tools, with NetBSD's reachover Makefiles. With them the
whole release built and its CD installed a system that booted
(from commit 42744e41 on). They were removed once GCC 2.95.3 did
the same. What they changed outside contrib/ stays, since each change
was one a later BSD had made: setmode.c's compress_mode returning
void, ping's pr_addr taking a struct in_addr, genassym compiled and
not run (kern/genassym.sh), the i386 descriptor structures packed
whole, and getrpcent, more and tip given the function types NetBSD
gives them.

## E -- a reimplementation of something the Lite cut removed

	lib/libcompat/4.3/regex.c   93 lines	NetBSD 1.0
	bin/expr			679 lines	NetBSD 1.0, unmodified
	sys/kern/subr_rmap.c		NetBSD 1.2, a note added
	usr.bin/awk			Kernighan, 20121220, unmodified
	usr.sbin/cron, usr.bin/crontab	NetBSD 1.0, NetBSD's later fixes
	usr.bin/at, libexec/atrun	NetBSD 1.0, NetBSD's and OpenBSD's
					later fixes

The same 93-line file is in NetBSD 1.1 and OpenBSD 1996 as well, with
the same contributor line; FreeBSD 2.0.5's is 96 lines and differs. It
carries a Regents copyright and says it was contributed to Berkeley by
James da Silva of the University of Maryland, so it is Berkeley-
licensed code that Berkeley had, not a NetBSD invention.

Berkeley's regex.c is 407 lines of regular expression engine marked
%sccs.include.proprietary.c%. This is not that file: it is a shim that
implements re_comp and re_exec in terms of regcomp and regexec, over
the engine already in this tree. NetBSD wrote it because Berkeley's
could not be redistributed.

**Which means this import was avoidable**, and
`docs/provenance/audit.md` records it as such. Both sides of the shim
are here: `lib/libcompat/regexp/regexp.c` is the engine and
`lib/libcompat/regexp/regexp.h` the header the file includes, in the
same library directory. Ninety-three lines could have been written
against them rather than taken.

Not acted on. The file is Berkeley's own copyright, a correct shim
would look like it, and rewriting correct code to shorten a list is
churn. Recorded so that the next file considered for import is asked
the question first: does this tree already have both sides of it?

**sys/kern/subr_rmap.c** replaces three `Body deleted' stubs -- rminit,
rmalloc, rmfree, the resource map swap allocates from -- with NetBSD
1.2's file, revision 1.11, whole: Wolfgang Solfrank and TooLs GmbH,
1992 and 1994, written from how the swap code uses the interface and
from sys/map.h's comments, by its own account. docs/provenance/clist.md
surveyed this file and chose NetBSD 1.0's revision 1.9; 1.11 is the
same code with <sys/systm.h> included and the two longs printed with
%ld, and NetBSD 1.3's 1.13 differs from it only in indentation.
FreeBSD 2.0 had replaced resource maps for swap with its own rlist,
which would change both callers. TooLs GmbH's licence is four-clause,
with an advertising clause. install.md, *Swap*, has the measurement.

**usr.bin/awk** is the One True Awk -- Aho, Weinberger and Kernighan's
own, the AT&T awk that 4.4BSD-Lite lost with the settlement -- at
version 20121220, the sources of Kernighan's own repository's first
commit (github.com/onetrueawk/awk, 87b9493), byte for byte: LICENSE,
README, awk.1, awk.h, awkgram.y, b.c, lex.c, lib.c, main.c, maketab.c,
parse.c, proto.h, run.c, tran.c. Not taken: his makefile and the test
archive. The licence is Lucent's of 1997, a permission notice with a
disclaimer and no advertising clause.

Chosen by the user over GNU awk, which is what NetBSD 1.0 and FreeBSD
2.0 shipped -- and, a correction, what 4.4BSD-Lite2 itself shipped as
/usr/bin/awk, from contrib/gawk-2.15.2, which the choice was made
without knowing; asked again, the user kept this awk, and gawk is
installed beside it as /usr/contrib/bin/gawk (patch 154); it is what
OpenBSD, FreeBSD and NetBSD carry now. Of its versions, 20121220 is the oldest whose sources can be had as
Kernighan published them: the GitHub repository begins there, and the
earlier releases survive in the BSDs' trees with their own changes
(FreeBSD 5.0's contrib/one-true-awk is 20021129). It is also the last
release before the 2018 maintenance and the 2023 UTF-8 rewrite.

The Makefile is this project's, in the shape of OpenBSD's: yacc, then
maketab built with HOSTCC and run here to make proctab.c. The one
change outside the directory is usr.bin/Makefile's SUBDIR, which names
awk again. Building it found that include/ctype.h declared none of the
classifier functions; that fix is kind B above.

**bin/expr** is expr.y, expr.1 and the Makefile, byte for byte as
NetBSD's netbsd-1-0 branch has them. Berkeley's expr was AT&T's and
went with the settlement; this one says it was written by Pace
Willisson, placed in the public domain, and largely rewritten by J.T.
Conklin. The manual page is separate work, copyright Winning
Strategies, Inc. 1993, under a four-clause licence whose third clause
asks for an acknowledgement in advertising, as Berkeley's own did.

The survey, all read and the yacc ones built here under GCC 14:

	NetBSD 1.0	expr.y	builds unmodified
	FreeBSD 2.0	expr.y	the same file, plus an unused %left
				UNARY and <sys/types.h>; drops the
				free(vp) in free_value, a leak, and
				an (int) cast in op_colon
	FreeBSD 2.0.5	expr.y	2.0's, plus a yyparse declaration,
				less an unused local
	NetBSD 1.1	expr.c	hand-written, no yacc
	OpenBSD now	expr.c	NetBSD 1.1's, carried forward
	NetBSD now	expr.y	a 2000 rewrite
	FreeBSD now	expr.y	the 1993 file, much grown

NetBSD 1.0 is the earliest, needs nothing changed, and is the release
every other choice in this file prefers. Nothing was cut from it,
because nothing in it is unused here: it is one grammar and the
operators POSIX names. It needs a yacc, and build/make.sh's is byacc.

Tested on the system: `expr 3 + 4`, `expr abc : 'a\(.*\)'`,
`expr 6 \* 7` and `expr 7 / 0` (status 2, "division by zero"), and
MAKEDEV's `expr $unit '*' 8 + 0` for every wd0 minor.

**usr.sbin/cron and usr.bin/crontab** are Paul Vixie's cron 3.0, and
**usr.bin/at and libexec/atrun** Thomas Koenig's at with David
Parsons's changes, each first committed byte for byte as NetBSD's
netbsd-1-0 branch has it (decided 2026-10-22). Berkeley's cron, at and
atrun were AT&T's (`att/bsd/shared' in the CSRG history) and went
with the settlement; every BSD carried these two in their place, and
Lite2's own /etc/crontab, with its user field, is the system crontab
Vixie's cron reads. Vixie's notice asks that changes be marked; every
change here is, at its site.

FreeBSD 2.0's cron is the same code in Vixie's own three-directory
layout with a libcron, Makefiles in FreeBSD 2.0's newer manual-page
style, and a misspelt HAVE_SAVED_SUIDS that NetBSD 1.0 had corrected.
NetBSD 1.0's Makefiles are in Lite2's own; one line changed, for
Lite2's bsd.prog.mk, which leaves `all' to a Makefile that defines it.

NetBSD 1.0's copies had holes NetBSD closed later, and those fixes
are taken, each its own commit (decided 2026-10-22):

	cron	MAILTO off sendmail's command line (-oi -t), and
		cron_popen's argv bounded: Hannum, 14 April 1995, NetBSD 1.1
	cron,	copies bounded throughout; env.c's name buffer, which a
	crontab	crontab variable name of over 100 characters overran as
		root (SNI-02, 16 December 1996): NetBSD's pull-ups of 26
		January and 6 March 1997 (Harvey, Thorpe), NetBSD 1.2.1 --
		with crontab's `sizeof(User - 1)' as NetBSD 1.3 corrected
		it, and env.c's quote-stripping strcpy kept, as FreeBSD 2.2
		kept it, where the pull-up's strncpy cut values short
	at	atq and atrm no longer made root: Thorpe, 25 March 1995,
		NetBSD 1.1
	at	the file -f names opened as the user, and panic and perr
		taking root back to remove a job: NetBSD 1.3's lines
	atrun	a job run with its owner's groups (initgroups): Demetriou,
		2 March 1995, NetBSD 1.1
	at	12am and 12pm: OpenBSD, de Raadt, 2 January 1996, from
		Mark Weaver, NetBSD PR 1743

Tested on a fresh install (patch 181): cron starts at boot and runs a
crontab's line on the minute; an ordinary user's crontab is installed,
listed, run as that user and removed; `crontab -u daemon' keeps the
whole name. As an ordinary user, at queues a job, atq lists only that
user's, atrm of root's job says "Not owner", and `at -f
/etc/master.passwd' is refused; atrun runs root's job with root's
groups and the user's with the user's alone; 12pm, 12am and 11:30pm
queue for noon, midnight and half past eleven. The at commands as an
ordinary user needed the saved ids execve now sets (patch 181).

## F -- Berkeley's own file, restored

	sbin/savecore/zopen.c	741 lines	NetBSD 1.0
	usr.sbin/sendmail	8.8.8		FreeBSD 2.2.8's tree

Marked %sccs.include.redist.c% in Berkeley's tree. It is absent from
the Lite releases because usr.bin/compress went whole for the LZW
patent, not for its licence. NetBSD's copy is Berkeley's file, so this
is that code coming back rather than another system's arriving.

**usr.bin/diff** was here from patch 155 to patch 157: Berkeley's
4.3-Reno diff and diff3, which the Lite cut removed as
%sccs.include.proprietary.c%, as OpenBSD took them back under
Caldera's licence on 25 June 2003, with Berkeley's diff3.sh. Patch 157
removed them by the maintainer's rule -- an alternative not derived
from Caldera-licensed code is preferred (deferred.md, *Caldera's
licence, and what the BSDs took under it*) -- and GNU diff 2.6, kind
D, is the system's diff in their place.

**usr.sbin/sendmail** is Eric Allman's sendmail, Berkeley's program
under the Regents' licence -- here because it is Berkeley's, though in
a later release than any Lite2 carried: not a file restored as it was,
but Berkeley's program at its last release under Berkeley's terms. Berkeley's tree held 8.7.Beta.7 (21 June
1995); Lite2's README says the beta was left off the tape at its
developers' request, and named the release to fetch. Decided
2026-10-23, after NetBSD 1.0 to 1.4, FreeBSD 2.0 to 3.0 and OpenBSD
from its fork to 2.3 were read:

	NetBSD   8.6.9 (1.0), 8.6.12 (1.1), 8.8.5 (1.2.1), 8.8.8 (1.3, 1.4)
	FreeBSD  8.6.9 (2.0) to 8.6.12 (2.1), 8.8.5 (2.2.0), 8.8.7
	         (2.2.5), 8.8.8 (2.2.8); contrib/ from 3.0
	OpenBSD  NetBSD's 8.6 at the fork, 8.7.3 to 8.7.6 (2.0), 8.8.4 to
	         8.8.8 (2.1 to 2.3), 8.9 from 1998

8.8.8 (October 1997) is the last release under the Regents' licence --
8.9 moved to the Sendmail License -- and the one all three shipped,
with the fixes for CERT's CA-96.20, CA-96.24 (any local user root, 8.7
to 8.8.2) and CA-97.05 (remote root by MIME, 8.8.3 and 8.8.4). The
tree is FreeBSD 2.2.8's, which keeps sendmail's own top-level Makefile
(8.15) and Berkeley's src/Makefile "for 4.4BSD only" (8.8), and whose
own changes are a few lines; NetBSD's and OpenBSD's Makefiles were
rewritten for their makes. Adapted, each change marked: the manual
pages by their formatted names, install for ${INSTALL}; no NIS; the
process title rewritten in place (SPT_REUSEARGV), sendmail's own
lines for FreeBSD 2.x, since this system's ps_strings holds vectors;
generic-bsd4.4.cf as /etc/sendmail.cf, with FreeBSD freebsd.mc's one
line making /etc/sendmail.cw optional; /etc/rc starting it by full
path, as all three did by 1996; and the installer running newaliases,
as FreeBSD's sysinstall does.

Tested on a fresh install (patch 182): the installer makes
aliases.db; sendmail starts at boot, "accepting connections on port
25" in ps; mail to root, to an ordinary user and to postmaster through
its alias is delivered by mail.local; `sendmail -bv' tells known users
from unknown; cron mails a job's output to its crontab's MAILTO;
SIGHUP restarts the daemon by its path; and the system's own m4 makes
the same sendmail.cf from cf/cf, but for the build stamp and three
blank lines.

## G -- ours

build/make.sh, build/sysroot.sh, build/kernel.sh, build/release.sh,
lib/csu/i386_elf/Makefile, and everything under docs/. The generated
specs file and sys/i386/conf/LINK.i386 were ours and are gone. See precedent.md for the inventory and for what is
claimed over them.

## What was not done, and what it costs

These are refusals, not omissions. Each one gives up something.

  The i386 disk, floppy and tape drivers -- since done
	wd.c is FreeBSD 2.1.0's (kind D); fd.c and wt.c are Berkeley's,
	corrected (kind B), and configured in GENERIC.i386.

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
	gtty, stty and ftime are NetBSD 1.0's now (kind D), and the five
	games that wanted them build.
	Cost: getpw, vlimit and vtimes, as in NetBSD 1.0.

  The twenty-five programs the SUBDIR lists name
	ed, expr, bc, dc, spell, learn, at, cron, sa, compress and the
	rest, all proprietary-marked or patent-encumbered. Several
	BSDs wrote their own. expr has since been taken, NetBSD 1.0's
	(kind E above), because MAKEDEV cannot make a disk without it.
	Cost: no editor, no calculator, no scheduler, no compression.

  usr.bin/vacation's libdbm and the fourteen games -- since done
	vacation links libc's dbm; the games build, but ching, whose
	program the cut took, and xneko and xroach, until X11R5.

  Kerberos, in eleven Makefiles
	4.4BSD-Lite2 turns Kerberos on unconditionally. There is no
	conditional anywhere in Berkeley's tree, and kerberosIV is
	shipped to support it, so login, su, rlogin, rsh, passwd,
	telnet, rlogind, rshd, telnetd and rcp were all meant to
	authenticate against a Kerberos KDC. They now do local
	password authentication only. That is a reduction against what
	Berkeley intended, and the sharpest one on this list.

	It was not forced. kerberosIV can be built: four of its seven
	DES table generators compile and run with the host compiler
	unchanged, and what remains is a byte-order define that
	include/conf.h asks for, one undeclared variable, two link
	failures, HOSTCC for the generators as bin/sh has, and the
	literal `make' in the parent Makefile that librpc needed too.
	A day's work of the kind this branch is made of. An earlier
	note in those Makefiles said the tree "cannot build
	kerberosIV", which was untested and wrong.

	Why it is still not done: Kerberos IV was superseded by
	Kerberos 5 in 1993, two years before this release, and its DES
	authentication has been breakable for decades. Every later BSD
	made it conditional and then deleted it -- OpenBSD ships no
	Kerberos at all today, and FreeBSD 13's login has none. There
	is nothing here to authenticate against, and the same day spent
	on wd.c would give the kernel a disk. The direction is what
	every other BSD took; only the fact that Lite2 never took it
	makes this a change.

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
