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

	lib/csu/*		619 lines	NetBSD 1.6 (see below)
	lib/libkvm/kvm_i386.c	221 lines	NetBSD 1.0, adapted
	vmstat/names.c i386 case  6 lines	NetBSD 1.0
	lib/libl/Makefile	  4 lines	NetBSD 1.0
	lib/libc/gen/nlist_elf32.c	NetBSD 1.3, adapted
	usr.sbin/kvm_mkdb/nlist_elf32.c	NetBSD 1.3, adapted
	sys/exec_elf.h: Shdr, Sym	NetBSD 1.3
	sys/boot/i386			FreeBSD 4.1, adapted
	usr.sbin/btxld			FreeBSD 4.1, adapted

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
shipped (commit *i386: the kernel at physical 1 MB*). NetBSD's i386
boot programs read only a.out until 1.4 (1999), and come with
NetBSD's stand-alone library beneath them; 4.1's boot2.c is 805
lines with nothing beneath it but BTX.

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

## E -- a reimplementation of something the Lite cut removed

	lib/libcompat/4.3/regex.c   93 lines	NetBSD 1.0
	bin/expr			679 lines	NetBSD 1.0, unmodified
	sys/kern/subr_rmap.c		NetBSD 1.2, a note added
	usr.bin/awk			Kernighan, 20121220, unmodified

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
2.0 shipped; it is what OpenBSD, FreeBSD and NetBSD carry now. Of its
versions, 20121220 is the oldest whose sources can be had as
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
	BSDs wrote their own. expr has since been taken, NetBSD 1.0's
	(kind E above), because MAKEDEV cannot make a disk without it.
	Cost: no editor, no calculator, no scheduler, no compression.

  usr.bin/vacation's libdbm and the fourteen games
	Cost: those programs.

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
