# Roadmap

Three tiers, in dependency order. Tier 0 finishes what `dev`/`dev2`
set out to do. Tier 1 is the first tier where anything runs. Tier 2 is
4.5BSD's stated target and is listed so that nothing in it is
rediscovered later.

Nothing here is a schedule. Items are listed with what is known about
each, and with the donor where the record names one. `docs/deferred.md`
holds the reasoning behind most of them; this file holds the ordering.

## Tier 0 — finish the heritage restoration

The `dev`/`dev2` goal was 4.4BSD-Lite2 as Berkeley shipped it,
compiling under a modern GCC. Measured against the tree as it stands:

| | done | total |
|---|---|---|
| program directories that build | 341 | 341 |
| libraries built by `sysroot.sh` | 13 | 13 |
| kernel sources compiled | 166 | 853 |
| kernel configurations that build | 1 | 1 (i386) |
| ports whose kernel has ever been compiled | 1 | 8 |
| kernels that link | **1** | — |
| kernels that run | **1** | — |

So the userland restoration is done for the i386, and the kernel one
is done for the i386 alone. What was listed here, and what remains:

**~~The kernel link.~~ Done.** The four symbols — `chrtoblk`, `fuswintr`,
`suswintr`, `wddriver`, plus `memset` on a gcc 14 host. Models for
each in `docs/status.md`. `libkern.a` is built by `build/kernel.sh`.

**~~`GENERIC.i386` does not build.~~ Done** (`f2975f92`). Corrected
line by line from configurations Berkeley, FreeBSD and NetBSD wrote,
with the floppy and tape as Berkeley's had them; it builds, boots and
installs, and `LINK.i386`, its stand-in, is gone.

**~~The DIAGNOSTIC build does not compile.~~ Done** (`99f245e0`), as
FreeBSD 2.0's headers have it; a DIAGNOSTIC kernel boots multi-user.

**~~`fd` and `wt` are not configured.~~ Done.** `wd` is FreeBSD
2.1.0's; `fd` reads and writes after three corrections (`4928f76c`);
`wt` is configured and has no hardware on QEMU to find.

**Seven ports have never been compiled at all. DEFERRED by the
maintainer's decision (2026-10-14): not needed soon; the i386 comes
first.** — hp300 (74 sources),
luna68k (59), sparc (55), pmax (48), news3400 (57), vax (61), tahoe
(161). Whether 4.5BSD keeps them is an open question, but "4.4BSD-Lite2
compiles" is not true of the tree until they do or are removed
deliberately. Each wants a cross toolchain for its processor; GCC
2.95.3 has m68k, sparc, mips and vax and no longer has the tahoe.
Lite2 itself cut vax's and tahoe's `include` directories.

**~~`libresolv` and `liby` are not built.~~ Done** (`63b740be`). The
root has every library `lib/Makefile` names but libmp and libplot,
whose directories the Lite cut removed.

**~~18 program directories still fail.~~ Done**: every directory
named for the i386 builds, and what cannot build here is no longer
named (deferred.md, *Programs that still do not build*). X11's two
games wait on contrib/X11R5.

**~~A build-ordering defect.~~ Gone.** Eight directories failed a
first `make.sh NOMAN=noman depend all` and succeeded on a second.
Measured again with the system's own compiler, each of the eight --
`sbin/disklabel`, `sbin/restore`, `sbin/route`, `usr.bin/lorder`,
`usr.bin/pagesize`, `usr.bin/shar`, `usr.sbin/arp`, `games/phantasia`
-- builds on a first pass into an empty object directory. The cause
was not established; it went with the host compiler.

**~~`YACC` still names the packaged byacc.~~ Done** (`acb899a0`): the
tree's own yacc and flex build it, and `ftpcmd.y` stands as Berkeley
wrote it.

## Tier 1 — boot on QEMU's legacy machine

**Reached.** On QEMU's `pc` machine the kernel boots single user,
installs the system from a ustar archive on a raw disk with its own
`dd` and `pax`, and reboots multi-user: `/etc/rc` runs to the end,
getty prints `login:`, root logs in to csh, `ps`, `w`, `dmesg`,
`savecore` and `kvm_mkdb` work, and `reboot` syncs and halts.
`docs/provenance/install.md` has the procedure, the faults it found
in the order it found them, and what /etc/rc still reports.

The original list, for the record, all done: the link; `libkern.a`;
loading the kernel (`build/shim` then, boot2 now -- see Tier 1½);
`DELAY()`; the serial console; `KERNBASE` at `0xF0000000`; a root
device; `pmap_remove` on the self-map.

**The settlement and Net/2 bodies.** Filled by import: the clists
(`tty_subr.c`), the buffer cache's write path (`vfs_bio.c`, NetBSD
1.0), accounting (`kern_acct.c`), raw I/O (`kern_physio.c`) and the
resource maps swap allocates from (`subr_rmap.c`, NetBSD 1.2). Still
`Body deleted`: `tty_subr.c`'s ndqb, stubbed again deliberately
(its own note says why), and `hp/dev/hil_subr.c`, which this build
does not compile. `ptrace` (`kern/sys_process.c`) is FreeBSD 2.2's,
on Lite2's procfs hooks, with its manual page (`a56c2f9b`).

**What is left before Tier 1 is solid**, in the order proposed:

1. ~~Owners and groups as the tree names them~~ -- `build/install.sh`.
2. ~~`/usr/share/misc/termcap`~~ -- built on the host, indexed on the
   system.
3. ~~**Swap.**~~ `subr_rmap.c` from NetBSD 1.2, a b partition from
   `build/mklabel.c`, `swapon -a` from fstab; measured paging under
   `-m 16`.
4. ~~**MAKEDEV's pty majors**~~, now 5 and 6 as conf.c has them, and
   ptys configured in the kernel; `script` runs through ttyp0. Still
   open: std's stdin, stdout, stderr and fd/ at major 53, which the
   i386 cdevsw has no driver for, and the panic from removing the
   device node the root was remounted through (`vinvalbuf: dirty
   bufs`). That one is understood and left: `mount -u` looks the
   device up by name, and checkalias, finding the root's vnode still
   tagged VT_NON from bdevvp, cleans it and makes it that file's vnode;
   remove the file and its vnode -- the mounted root's device -- goes
   with it. NetBSD 1.0 and 1.3 and FreeBSD 2.0 have the same
   checkalias. Do not remove /dev/wd0a.
4½. ~~**awk.**~~ Kernighan's own, version 20121220: MAKEDEV's pty
   and fd cases run, and ptys and fd/ are made at install.
5. **`-fno-common`. DEFERRED TO THE GCC UPGRADE -- must be done
   before any compiler newer than GCC 2.95.3 builds the kernel.**
   The `mmap` fault (install.md, fault 10) was a common symbol
   resolved to a function. GCC 2.95.3, the system's compiler now,
   defaults to `-fcommon`, as every BSD kernel of the period was
   built, so nothing is needed under it. GCC 10 and later default to
   `-fno-common`, and the planned move to newer compilers (the GCC
   4.2.1 pair below, and later ones) will meet it: 101 symbols are
   tentatively defined in more than one kernel object -- `nbuf`,
   `bufpages` and `nswbuf` in some sixty each, from headers that say
   `int nbuf;` -- and each needs an `extern` in its header and one
   definition, as the BSDs did when GCC 10 changed the default.
   Until then `build/kernel.sh` refuses a kernel in which any common
   symbol is a text symbol, the exact class of the `mmap` fault.
6. ~~**`ptrace`**~~, for gdb. FreeBSD 2.2's `sys_process.c`, which
   keeps Lite2's whole request set, on Lite2's procfs hooks; i386's
   hooks are Pendry's own, for both of this port's frame kinds, and
   the register requests are the PT_GETREGS family all three
   descendants define (`a56c2f9b`); its manual page is FreeBSD 2.2's.
   `GENERIC.i386` has `PROCFS` (`a1b09daf`). The FPU registers wait
   (deferred.md). Found and closed with it: a process could raise its
   IOPL through sigreturn (`f19baeb2`) and use most I/O ports outright
   (`579fed2d`).
7. ~~**`ps_strings`.**~~ execve fills it in, and kvm_argv bounds
   addresses by USRSTACK; `ps -ax` shows every command's arguments.
   Left: `ps -e` on init shows a garbage environment -- init has none,
   and kvm_argv given a count of 0 still returns one string.

## Tier 1½ — an installable `.iso`

What the BSDs of 1994-98 shipped: a boot image carrying a kernel and a
small root, an installer on that root, and the distribution as files
beside it. FreeBSD 2.0 installed from a boot floppy with shell
scripts; FreeBSD 2.0.5 (1995) introduced the menu-driven sysinstall;
NetBSD and OpenBSD used scripted installers on a RAM-disk root, NetBSD
adding sysinst in 1.3. Bootable CDs came after El Torito (1995), first
as the boot floppy image in floppy emulation.

1. ~~**A bootloader that reads ELF.**~~ FreeBSD 4.1's boot1 and
   boot2, Robert Nordier's, under BTX, shrink-wrapped in
   `sys/boot/i386`, after the kernel moved to physical 1 MB as every
   descendant's had. An installed disk boots in QEMU with no
   `-kernel`, to `login:`; install.md, *Booting from the disk*.
   `build/shim` stayed for the install recipe until 2, 3 and 4
   replaced it.
2. ~~**An install kernel with its root inside it**~~ -- the miniroot,
   FreeBSD 2.0.5's MFS-in-kernel boot floppy -- holding sh, the disk
   tools and pax. The mechanism is in: `options MFS_ROOT`, Berkeley's
   mfs_initminiroot, FreeBSD 2.0.5's write_mfs_in_kernel, an image
   from the host's makefs, booted from a 2.88 MB floppy image
   (install.md, *A root inside the kernel*). crunchgen is in, FreeBSD
   3.0's (install.md, *One binary*). The install root is in:
   NetBSD 1.1's i386 install floppy, 42 programs in one binary, in an
   install kernel that boots from a 2.88 MB floppy image to NetBSD's
   installer (install.md, *The install root*).
3. ~~**A scripted installer**~~: NetBSD 1.1's i386 install.sh and
   Configure, adapted. From the install floppy image it labels a
   blank disk with the boot blocks, makes and populates the root,
   and after the base set is extracted names the host and makes
   /dev, pwd.db and termcap.db; the disk then boots by itself, to
   login (install.md, *Installing with the installer*). The sets
   come from the CD, 4.
4. ~~**The image**~~: El Torito, the boot image in floppy
   emulation as NetBSD's tree first describes it, in 1.4, and the
   set as a ustar file on the ISO 9660 filesystem, which Lite2's
   cd9660 reads. The CD driver is
   FreeBSD 2.1.0's IDE and ATAPI drivers with its slice layer, and
   options CD9660 (imports.md, *The disks*). The CD boots the install
   kernel, the installer extracts the set from it, and the disk then
   boots by itself (install.md, *Installing from the CD*).

## After Tier 1½ — self-hosting

The system building itself. Lite2 has no compiler for its i386: its
contrib/gcc-2.3.3 carries Berkeley's BSD-make Makefile for hp300,
luna68k, mips and sparc and not the i386, and contrib/gas-1.38 and
usr.bin/ld, GNU ld 1.x, are a.out tools; all C89. So this began with
a decision: which compiler, assembler and linker, from where, and
whether the first stage is a cross-build from the host.

**Decided first, and superseded below: pcc, to reach a self-hosting
system soonest.** The Portable C Compiler -- BSD-licensed, about 1.5
MB, C99 with parts of C11 -- as NetBSD carries it in
src/external/bsd/pcc (1.2.0.DEVEL, 20160208, imported 2016-02-09, with
BSD-make Makefiles for cpp, ccom and the driver); OpenBSD carried an
earlier one in usr.bin/pcc from 2007 to 2012. It emits GNU-syntax
assembly and runs the system's `as` and `ld`.

**The assembler, linker and binary tools: NetBSD 5's binutils 2.16.1**
-- gnu/dist/binutils with the reachover Makefiles of
gnu/usr.bin/binutils and gnu/lib/lib{bfd,opcodes,iberty}, whose i386
config headers NetBSD generated ahead (mknative), so it builds with
BSD make and no GNU configure. GPLv2; i386 ELF; about 126,000 lines
of bfd, gas, ld and opcodes for i386; and NetBSD 5 is the first branch
that carries external/bsd/pcc beside it, the pairing pcc was used
with. Surveyed across all three trees:

	NetBSD   1.0-1.2 gas 1.92 and GNU ld 1 (a.out); 1.4-1.5 2.9.1;
	         1.6 2.11.2; 2-5 2.14, 2.15, 2.16.1 (GPLv2, reachover);
	         6 on 2.21.1 to 2.46 (GPLv3, external/gpl3)
	FreeBSD  3.0-12 contrib/binutils 2.9.1 to 2.17.50 (GPLv2,
	         reachover, gnu/usr.bin/binutils/*/Makefile.i386);
	         elftoolchain's nm, size, strings, strip from 11, objcopy
	         from 12, its own BSD ar from 8; lld and no GNU from 13
	OpenBSD  2.7 to 2.15, then 2.17 (GPLv2), each built by a
	         Makefile.bsd-wrapper that runs GNU configure; lld and
	         clang on i386 from 2018-19

FreeBSD 12's 2.17.50, the last GPLv2 snapshot, is the fallback --
newer i386 opcodes, about 139,000 lines, Makefiles tied to FreeBSD's
src.opts.mk. No BSD has shipped a BSD-licensed ELF assembler or
linker. Later, permissive tools can replace the binary ones: nm and
size from OpenBSD's usr.bin/nm (BSD, ELF), ar and ranlib from FreeBSD
8's usr.bin/ar, strip and objcopy from elftoolchain's elfcopy.

**Permissive alternatives looked for, and why not now.**

	OSF Mach 7.3 / MkLinux  gcc and GNU binutils (osfmk makedefs);
	                        its own tools are ODE, mig and config
	Apple cctools ld        APSL, Mach-O only; its as is GNU as 1.38
	Minix 3 / ACK           BSD-3, ANSI C; its own assembler syntax
	                        and object format, ELF only by aelflod
	                        after linking; no GNU inline asm -- every
	                        .s rewritten, no mixing with ELF objects
	Plan 9 8c/8a/8l         MIT since 2021; 8l writes ELF but links
	                        only Plan 9 objects, in Plan 9 assembly
	yasm (-p gas)           BSD; measured: 30 of libc's 31 i386 .s
	                        files assemble, 29 to the bytes GNU as
	                        gives, one on VAX-style `0r' floats; the
	                        kernel's locore.s crashes it, first at
	                        `.set X,(0x3c0 << 22)' (yasm 1.3.0)
	elftoolchain ld         BSD-2; measured: links a one-instruction
	                        program, but a static link against libc
	                        crashes it or writes an empty file
	neatcc, scc, cproc+QBE  no inline asm or no i386 target
	tcc                     LGPL; compiler, assembler and linker in one
	lld, mold               permissive, but C++, which pcc cannot build

So the one GPL part is as and ld, GPLv2, and a fully permissive chain
waits on yasm or elftoolchain ld maturing.

Measured before deciding: NetBSD's pcc, built on the host as an
i386-pc-netbsdelf cross-compiler (it needs -fcommon under GCC 14) and
pointed at this tree's sysroot, compiles and links bin/cat to an i386
ELF executable, and all but a handful of files of bin, sbin and libc:
a `return;' in an int function (gen/setmode.c), a declaration it
misreads in gen/nlist_elf32.c and one in <rpc/auth.h>, and one
internal error passing a struct by value (sbin/ping). libkern
compiles whole. 64-bit division calls __divdi3 and its kin, which
libc/quad has. Not yet run on the target.

Done: pcc in contrib/pcc, built on the host as a cross-compiler and
chosen by L2_CC=pcc; with it, libc, the startup files, bin, sbin and
eleven more programs build, and run on the target; and the kernel,
`L2_CC=pcc sh build/kernel.sh', which boots (imports.md, *The C
compiler: pcc*). Then binutils 2.16.1 in contrib/binutils, built on
the host beside pcc and used by it as its as and ld; ar, ranlib, nm
and strip in the Makefiles are still the host's (imports.md, *The
assembler, linker and binary tools*). With them, the whole release
builds with `L2_CC=pcc', and its CD installs a system that boots.

The stages: (1) pcc as a host tool, replacing the GCC specs file
(deferred.md, *The specs file stands in for a cross-compiler*), the
tree rebuilt with it; (2) pcc and binutils 2.16.1 built by that pcc
and installed on the target; (3) the target rebuilding itself.

**Superseded: the GNU toolchain, as 4.4BSD itself used.** pcc was
chosen for its licence. Asked what Lite2 actually used, the tree
answers plainly: sys.mk says `CC ?= gcc`, contrib/gcc-2.3.3 and
contrib/gas-1.38 are GNU's, usr.bin/ld is GNU ld, and the 4.4BSD
setup notes (share/doc/smm/01.setup/3.t) list "Replacement of pcc
with gcc suite" among the release's changes. So the toolchain is GNU's
again, GPL and all, in two generations:

	contrib/gcc-2.95.3, contrib/binutils-2.11.2   the system's cc, as,
	        ld: NetBSD 1.6's pair, "2.95.3 (NetBSD nb3)", "2.11.2nb1"
	contrib/gcc-4.2.1, contrib/binutils-2.15      beside it, for newer
	        C: FreeBSD 7.0 and 8.0's pair, GCC's C compiler only

Each is GNU's release, whole and untouched, with the donor BSD's
changes committed on top as a separate commit, recovered by comparing
the donor's tree with the release. Each builds the way Lite2 built
contrib/gcc-2.3.3 and contrib/gdb-4.7.LBL: a README.BSD and a small
BSD `makefile` that configures GNU's sources for the machine and runs
GNU's own Makefiles under the system's make. The target is
`i386-unknown-bsd4.4elf`, this system's own name, not NetBSD's.

Done: the 2.95.3 pair imported, with NetBSD 1.6's changes, and this
system added to both as i386-unknown-bsd4.4elf; built on the host by
build/sysroot.sh as a cross toolchain, and used for everything:
`sh build/release.sh' compiles every library, program, boot block and
both kernels with it, and the CD installs a system that comes up
multi-user (imports.md, *The toolchain*). pcc and binutils 2.16.1,
the host GCC's specs file and L2_CC are gone with it.

Next, in order: README.BSD and the makefile in each directory --
binutils-2.11.2's done, the system's as, ld and the rest built to run
on it -- and the system rebuilding its own compiler, for which the
whole tree in the release now gives make, share/mk and the rest of a
build environment; then the 4.2.1 pair the same way. The kernel must be made to build with `-fno-common`
before any of the newer pairs compiles it (Tier 1, item 5).

**~~The whole tree on the CD.~~ Done** (patch 153). Decided
(2026-10-14): the whole tree, installed by Berkeley's own top-level
`make build`, as one base set, as Lite2 intended -- grep is Lite2's
own, Woods' egrep. `build/release.sh` now runs exactly that; what it
took (docs/deferred.md, *The whole tree by `make build`*):

- the top-level Makefile in FreeBSD 2.0's `${.CURDIR}` form, and
  kerberosIV behind KERBEROS;
- object directories where Lite2's own `make obj` made them, and none
  for documents or below contrib/;
- the tree's own zic, cap_mkdb, strfile and col built for the build
  machine and run by name, FreeBSD 2.2's build-tools;
- what the Lite cut emptied (diff, ex, file, sort, uucp, ac, quot,
  sendmail, old) no longer named; dict, tmac and the manual-page lists
  without the files the cut took, as FreeBSD 2.0, NetBSD 1.0 and
  OpenBSD all have them;
- the hidden games' links not chowned (NetBSD 1.0, cgd); chpass made
  immutable by the installer;
- Lite2's a.out-only ld, ar, nm, ranlib, size, strip, strings, gprof
  and gcore not built for the i386, and binutils 2.11.2 built to run
  on the system in their place, from contrib/binutils-2.11.2's own
  README.BSD and makefile, in Lite2's manner.

**Next: contrib/, one program at a time.** Decided (2026-10-16):
Lite2's contrib/Makefile is the plan -- its programs, their own build
systems and Lite2's /usr/contrib, not the descendants' gnu/ and ports
-- taken up one by one, the C programs first and the two in C++,
groff and libg++, after GCC's C++ (below). Built in object directories,
as every descendant built them, OpenBSD's Makefile.bsd-wrapper way
for those configured by GNU's configure (decided 2026-10-16).
gcc-2.3.3, gas-1.38 and gdb-4.7.LBL are not named for the i386: they
make and read a.out; GCC 2.95.3 and binutils 2.11.2 stand in, and a
gdb that reads ELF is to come.

Done: binutils 2.11.2 (patch 153); ansi, nvi, flex as /usr/bin/lex,
gawk as /usr/contrib/bin/gawk beside Kernighan's awk, and gzip (patch
154). Left, with what each is known to need:

	rcs-V5.6      diff and diff3 at run time, which the Lite cut
	              emptied (deferred.md, *Programs the Lite cut
	              emptied*); its conf.sh runs test programs, which a
	              cross build cannot
	bib, hunt, jove-4.14.6, kermit-5A.188, mh-6.8.3a, bind-4.9.2
	              their own Makefiles, each with generators or
	              configuration that run on the build machine
	perl-4.036    Berkeley's config.sh and makefile
	emacs-18.57   dumps itself by running temacs: on the system, or
	              not at all, until it self-hosts
	groff-1.08, libg++-2.3
	              C++

**Later, and noted: a test framework and a kernel debugger.** Lite2
has tests but no framework to run them: Berkeley's db test driver,
sed.test, the m4 and libterm/libedit test programs, and sys/tests
(benchmarks, filesystem tests, Sun's Connectathon NFS suite). NetBSD
1.0's regress/ (1994) is the period's framework, wrapped around
Berkeley's own db test; OpenBSD kept regress/ to this day; NetBSD and
FreeBSD later moved to ATF and Kyua. Lite2 has no ddb -- only kgdb
stubs, and kadb named in the configurations with its sources cut --
and ddb, CMU Mach's via 386BSD, is in NetBSD 1.0 and FreeBSD 2.0.

**Later: GCC's other languages.** GCC 2.95.3 is imported whole, with
its C++, Objective C, Fortran 77, CHILL and Java front ends. C comes
first, then C++, which Lite2's own groff and libg++ are written in.
Fortran
(g77, libf2c) and Java (gcc/java; its runtime, libgcj, was a separate
release) are to be taken up once C builds the system.

## Ports

**Decided: FreeBSD's ports framework, as of 1997.** Jordan Hubbard's
bsd.port.mk (public domain), from FreeBSD 2.2.2's ports.tgz on
archive.freebsd.org (May 1997), the earliest copy that survives
whole: /usr/ports, packages installed under /usr/local and recorded
in /var/db/pkg, as hier(7) in this tree already describes those
directories. Measured: both its 1995 and 1997 frameworks run under
Lite2's make, and NetBSD's pkgsrc of the 1.6 era does not (it needs
NetBSD's make and bsd.own.mk). The 1997 framework over the 1995 one:
24 new documented variables and four dropped, eight more targets,
and each installed package recording which others need it
(+REQUIRED_BY), so that pkg_delete can tell what is still in use.

What it needs that the release does not yet carry: make and
/usr/share/mk in the base set, and tr, sort, patch, xargs, cksum,
gzip, mtree and ftp; and from FreeBSD 2.2, pkg_install (pkg_add,
pkg_create, pkg_delete, pkg_info), md5, fetch and a tar that reads
gzip.

**Deferred: removing a program together with what it pulled in.**
Installing a port installs its dependencies with it; removing one
removes that package alone, as in FreeBSD at the time, and leftover
dependencies are removed by hand, newest first. pkg_cutleaves (Stefan
Walter, 2003; the 2006 version uses plain pkg_delete) is the period
tool that does this, as a port: it needs Perl 5, which FreeBSD 2.2's
ports tree carries as lang/perl5 (the base system has Perl 4.036, in
contrib/perl-4.036). To be taken up after the framework is in.

## Tier 2 — modern hardware

Each of these is a measured absence, not a stylistic gap. Counts are
from the tree as it stands.

**No PCI bus.** Zero files. A grep for `pci` across the whole kernel
returns nine hits and every one is a false positive — `profile.h`'s
`_mcount` internals, `pccons.c`, `nfs.h`, a test program. Nothing on a
modern machine is findable without this, and everything below depends
on it. `sys/i386/eisa/` and `sys/i386/mca/` each contain a single
`tags` file and no code.

**No ACPI.** Zero files. Post-2000 machines describe interrupt
routing, power and device topology there.

**No APIC or IOAPIC.** Zero files. The 8259 PIC is still emulated on
modern chipsets but increasingly vestigially.

**Build-time interrupt wiring.** `usr.sbin/config/mkglue.c` generates
`vector.s` with one stub per configured interrupt and the handler
baked in. Two properties of modern interrupt hardware cannot be
expressed in it at all: PCI lines are *shared*, so a handler chain
must be walked at run time and its length is unknown until the bus is
enumerated; and MSI/MSI-X allocate a vector per device from a pool
when the device is found, so the vector number does not exist at
config time. Neither is a performance argument.

  `5a0ade80` establishes that `mkglue.c` came from 386BSD in January
  1991 as "386BSD additions to config" and was never Berkeley's
  design, which is why every later tree dropped it rather than
  maintaining it. **No Berkeley heritage is lost by replacing it.**
  Donors: NetBSD 1.1 has a static `vector.s` dispatching through a
  run-time table; OpenBSD 1996 has no `mkglue.c` at all; FreeBSD
  2.0.5 carries `register_intr()`/`unregister_intr()` in an `isa.c` of
  1056 lines against this tree's 254, and every driver's attach path
  changes with it.

**No storage driver that matches anything.** `wd.c` is an ST-506/IDE
driver of 1995, FreeBSD 2.1.0's, with ATAPI for CDs; it runs PIO. No AHCI, no NVMe.

**No USB.** On many machines that means no keyboard.

**No modern NIC.** `if_ne` is an NE2000; `if_we`, `if_ec` and
`if_apx` are of the same era.

**32-bit, no PAE.** The pmap assumes a 32-bit physical address space.
Modern machines have more than 4 GB.

**BIOS boot only.** The bootstrap, FreeBSD 4.1's boot1 and boot2,
reads ELF but starts only from a BIOS; UEFI-only machines cannot boot
this at all.

**Uniprocessor.** No kernel threading; `struct proc` is the unit of
scheduling and there is no thread abstraction to carry forward.

### The question Tier 2 cannot start without

"Modern hardware" spans two targets that differ enormously in scope:

- **roughly 2005–2015**: legacy BIOS, PCI, IDE-compatible SATA,
  USB-legacy or PS/2 input. Large work, but every piece has a lineal
  BSD implementation that could be traced.
- **2020 onward**: UEFI-only, NVMe, no CSM, USB-only input. Almost
  none of that has 4.4BSD ancestry to be genetic about, because it
  postdates the lineage.

Which one 4.5BSD targets decides whether Tier 2 is six items or
twenty, and it should be settled before any Tier 2 work begins.
