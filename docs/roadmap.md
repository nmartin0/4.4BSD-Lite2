# Roadmap: 4.5BSD

Two releases, decided by the maintainer (2026-10-22).

**4.5BSD** is 4.4BSD-Lite2 restored: the system as it should have been
at its release, completed into a capable, running system. Lite2's own
code, made whole and correct; what the Lite cut took, put back or
replaced; the gaps filled from the descendants of the period, as they
filled them. Its compiler is GCC 2.95.3, its C is C89, its machine the
legacy PC that QEMU's `pc` emulates.

**4.6BSD** is cut from 4.5BSD once 4.5BSD is complete, and continues
it: new kernel primitives, more drivers and modern hardware, modern
tools, POSIX 2024, a modern C standard. A modern, fully capable system
that is none of OpenBSD, NetBSD or FreeBSD, loyal to its Lite2
heritage, minimal, and close to its roots. Its roadmap is
`docs/roadmap-4.6.md`.

The test for which release an item belongs to: does a capable 4.4BSD
system of its own time need it, or does it modernize one? The first
is 4.5BSD's; the second waits for 4.6BSD.

This file is 4.5BSD's: what is left; what is still to be settled
between the two; what is noted for later; what is done, in the order
it was done; and the two plans decided and not yet begun, the ports
and Torek's autoconfiguration.
Nothing here is a schedule. `docs/deferred.md` holds the reasoning
behind most items; this file holds the ordering.

## What is left for 4.5BSD

The work list, settled by the maintainer (2026-10-22): the i386 alone
for now, X11R5 in, PCI in, sh's -e bugs fixed (patch 180), GCC's other
languages and Kerberos left for later. Each item is taken up with its
precedent read first. Proposed order: 1, then 2 and 6 as they fall
out of 1, then 3, 4 and 5; 7, then 9; 8 last.

**1. What the system itself reaches for and has not got.** Measured
on an installed system booting multi-user (patch 178's test, from the
console log):

- `cron: not found`. /etc/rc starts cron; the Lite cut deleted
  usr.sbin/cron, and nothing runs /etc/daily, weekly or monthly.
- `sendmail: not found`. /etc/rc starts the mail daemon; the Lite cut
  emptied usr.sbin/sendmail. Mail is not delivered, not even locally.
- `timed: not found`. /etc/netstart asks for it; Berkeley's own
  usr.sbin/Makefile leaves timed off the i386's list.
- `NFS not available`. nfsd and mountd start; GENERIC.i386 leaves out
  `options NFS`, which Berkeley's own i386 configurations carried.
- `/usr/libexec/ex3.7preserve: not found`. /etc/rc preserves the old
  ex's files; the system's editor is nvi, whose own recover script
  does this.
- `/var/log/secure: No such file or directory`, at syslogd's start.
- /etc/daily runs `sa -s`, and /etc/rc turns on accounting; sa and ac
  were deleted or emptied by the cut.

**2. What the Lite cut took, to be decided one by one** (deferred.md,
*Sources and directories named but absent*, *Programs the Lite cut
emptied*, *Caldera's licence, and what the BSDs took under it*): at,
bc, dc, compress, deroff, spell, units, file (contrib/file-3.12 is on
hand), quot, uucp, fsdb, icheck, ncheck, mkproto, graph, plot, spline,
xsend, libmp, libplot, and libcompat's ecvt and gcvt (in the Single
UNIX Specification until 2008, which removed them). Those no BSD
has -- learn, struct, diction, ptx -- stay out.

**3. A debugger.** gdb-4.7.LBL reads a.out only; a gdb that reads ELF
is to come (*After Tier 1½*, below). And ddb, which the kernel has
not got: CMU Mach's, as NetBSD 1.0 and FreeBSD 2.0 carry it (*Later, and
noted: a test framework and a kernel debugger*, below).

**4. Ports** (decided 2026-10-14; *Ports*, below): FreeBSD 2.2.2's
framework, and what it needs from FreeBSD 2.2 -- pkg_install, md5,
fetch.

**5. A test framework**: NetBSD 1.0's regress/, around Berkeley's own
tests (below).

**6. The kernel's loose ends of the period**, each in deferred.md or
status.md: a statistics clock (FreeBSD 2.0.5's and OpenBSD 1996's, on
IRQ 8); `CLKF_INTR`; the FPU registers for ptrace, once npx saves on
demand; dial-out devices for the serial ports; vmstat's `??0` drive
names; fd's debugging printf; the kernel linked without a linker
script; the kernel's warnings, not yet audited. The `vinvalbuf`
panic is understood and left (Tier 1, item 4).

**7. Berkeley's own driver framework on the i386**: Torek's
autoconfiguration, stages 1 to 4, PCI with bus_space and bus_dma among
them (decided 2026-10-22; *Berkeley's driver framework on the i386*,
below). 4.4BSD shipped the framework and converted one port
(deferred.md, section 6); finishing the move is restoration.

**8. The release itself.** The kernel still calls itself
`4.4BSD-Lite` (sys/conf/newvers.sh: ostype, osrelease, version);
release notes; the sets and the installation guide as a release
carries them; and docs/status.md, which still describes the kernel
before it linked, brought up to date.

**9. X11R5** (decided 2026-10-22). Lite2's contrib/X11R5-* is built
for hp300 and luna68k as shipped; the i386 needs an X server for its
display, which Lite2's contrib does not carry: Lite2 has the
libraries and the hp300's and luna68k's builds only. To be read first:
X386, Thomas Roell's PC server, which MIT's X11R5 distribution
carried, and XFree86, which grew from it and which the BSDs of 1993-95
shipped. X11's two games, xneko and xroach, wait on it.

## To settle: 4.5BSD or 4.6BSD

- **Shared libraries** (*Later: shared libraries*, below). No 4.4BSD
  had them; NetBSD 1.3 and FreeBSD 3.0 did, in 1997-98. Lean: 4.6BSD.

Settled (2026-10-22), from the list that stood here:

- **The seven other ports** -- hp300, luna68k, sparc, pmax, news3400,
  vax, tahoe -- are left out for the time being; 4.5BSD is the i386's
  (Tier 0, below).
- **X11R5**: 4.5BSD (item 9, above).
- **PCI and bus_space/bus_dma**, Torek's stage 4: 4.5BSD (item 7).
- **GCC's other languages**: later (roadmap-4.6.md).
- **sh's two remaining -e bugs**: fixed, patch 180.
- **Kerberos** stays conditional and off for the time being.

Placed in 4.6BSD (roadmap-4.6.md): newer compilers and `-fno-common`
(Tier 1, item 5); locking, Torek's stage 5; the modern hardware this
file once listed as Tier 2; sh's redirection errors on builtins, left
as every BSD had them until 2002 and later (deferred.md).

## Noted for later, with what is known

Found while the system was brought up, each with what is known so
far. Where each falls is in the two lists above.

**Later, and noted: what Caldera's licence freed.** deroff, spell,
ching's missing files, compress, bc and dc, and 4.4's
Caldera-licensed documents -- not needed to bootstrap (decided
2026-10-16). Code this tree adds is taken under Caldera's licence
only where OpenBSD, NetBSD or FreeBSD took it, preferring an
alternative not derived from it, and nothing under the old Berkeley
notice is added (decided 2026-10-17); what came with Lite2 is kept.
The findings, the kernel among them, and the choice for each in
deferred.md, *Caldera's licence, and what the BSDs took under it*.

**Later, and noted: a test framework and a kernel debugger.** Lite2
has tests but no framework to run them: Berkeley's db test driver,
sed.test, the m4 and libterm/libedit test programs, and sys/tests
(benchmarks, filesystem tests, Sun's Connectathon NFS suite). NetBSD
1.0's regress/ (1994) is the period's framework, wrapped around
Berkeley's own db test; OpenBSD kept regress/ to this day; NetBSD and
FreeBSD later moved to ATF and Kyua. Lite2 has no ddb -- only kgdb
stubs, and kadb named in the configurations with its sources cut --
and ddb, CMU Mach's via 386BSD, is in NetBSD 1.0 and FreeBSD 2.0.

**Later: shared libraries** (noted 2026-10-22). The system links
every program statically, as 4.4BSD did; Lite2 has no shared
libraries. For ELF there is one family, from John Polstra's work of
1996: NetBSD's ld.elf_so (Polstra and Matt Thomas, 1996; in NetBSD 1.3,
1997, first on the alpha) and FreeBSD's rtld-elf (Polstra, 1996 to
2000; in FreeBSD 3.0, October 1998, the first i386 BSD release with
ELF shared libraries). OpenBSD's ld.so was written independently in
2000, first for MIPS and PowerPC. The shared libraries of NetBSD 0.9
and FreeBSD 1.x, Paul Kranenburg's SunOS-style ld.so, are for a.out
and do not apply. The lean, to be researched when taken up: NetBSD's
ld.elf_so, since this system's ELF pieces are NetBSD's already -- the
kernel's sys/exec_elf.h and loader, and lib/csu's start files -- and it
shares Polstra's original with FreeBSD's. A project of its own:
position-independent libraries, ld -shared, the kernel loading an
interpreter (PT_INTERP), the shared start files (crtbeginS), and the
library Makefiles. Emacs is not a blocker either way: it can stay
statically linked, emacs's own NO_SHARED_LIBS, as emacs's s/netbsd.h
had NetBSD/alpha do in 1997; or link dynamically as that file's
shared-library case does, with the system's start files and the one
line Berkeley added to sysdep.c, `char __progname[] = "emacs"', which
the system's crt0 also defines, put under a guard.

**Later: GCC's other languages.** GCC 2.95.3 is imported whole, with
its C++, Objective C, Fortran 77, CHILL and Java front ends. C comes
first, then C++, which Lite2's own groff and libg++ are written in.
Fortran
(g77, libf2c) and Java (gcc/java; its runtime, libgcj, was a separate
release) are to be taken up once C builds the system.

## Done for 4.5BSD

### Tier 0 — finish the heritage restoration

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
(161). Left out for the time being (decided 2026-10-22), but "4.4BSD-Lite2
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

### Tier 1 — boot on QEMU's legacy machine

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
   ptys configured in the kernel; `script` runs through ttyp0. std's
   stdin, stdout, stderr and fd/ are made at major 22, where the i386
   cdevsw now has fdopen, as every other port's in this tree does --
   FreeBSD 2.0's i386 major for it (patch 158; sort writes to
   /dev/stdout). Still open: the panic from removing the
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
   And `ps -e` on init, which has no environment, showed one string of
   garbage: kvm_argv found the strings by counting NULs and, given a
   count of 0, still returned the first. Now ps_strings records where
   the argv and envp vectors are, and libkvm reads the strings through
   them, as NetBSD 1.1 has it (Charles M. Hannum, May 1995) and
   FreeBSD 2.2 (patch 176).

### Tier 1½ — an installable `.iso`

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

### After Tier 1½ — self-hosting

**Reached** (patches 168 to 170). From its own sources on its own CD,
the system runs Berkeley's `make build` -- headers, libraries, the
whole tree with contrib -- and builds its own kernel by config and
make, the setup guide's way, and boots it as /vmunix: everything
compiled by its own GCC 2.95.3 and linked by its own binutils
(install.md, *The system builds itself*, *Its own kernel*). The work
can move from the build machine to the system itself.

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
on it; and gcc-2.95.3's done (patch 161), the system's own C compiler,
installed where 4.4BSD put its GCC, under /usr/libexec/gcc2, with
/usr/bin/gcc and, by the maintainer's choice, /usr/bin/cc linked to
it: on the system it compiles and links against libc and libm, and
make, mkdep and bsd.prog.mk build a program with it -- and the system
rebuilding itself and its own compiler, for which the whole tree in
the release gives make, share/mk and the rest of a build environment,
and the CD the sources, extracted into /usr/src by hand (patch 162),
and the system's own nroff, groff 1.08, built with GCC's C++ (patch
164), so that make in /usr/src builds a program and its manual page
there; then the 4.2.1 pair the same way. The kernel must be made to
build with `-fno-common` before any of the newer pairs compiles it
(Tier 1, item 5).

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
groff and libg++, after GCC's C++ (below). Built among their
sources, as Berkeley built contrib -- its Makefile gives contrib no
object directories -- and in a copy of their own only on another
machine, as OpenBSD's perl wrapper built in place unless there was an
object directory (decided 2026-10-19, superseding object directories
for all, 2026-10-16); OpenBSD's Makefile.bsd-wrapper is the name of
the file for those configured by GNU's configure.
gcc-2.3.3, gas-1.38 and gdb-4.7.LBL are not named for the i386: they
make and read a.out; GCC 2.95.3 and binutils 2.11.2 stand in, and a
gdb that reads ELF is to come.

Done: binutils 2.11.2 (patch 153); ansi, nvi, flex as /usr/bin/lex,
gawk as /usr/contrib/bin/gawk beside Kernighan's awk, and gzip (patch
154); for rcs, diff, diff3 and sdiff -- GNU diff 2.6, the system's in
/usr/bin, as in NetBSD 1.0 and FreeBSD 2.0 (patch 155, and patch 157,
which took out Berkeley's diff, Caldera-licensed) -- and ed as /bin/ed
(patch 155); bib, hunt, jove, kermit and MH (patch 157); and sort,
Berkeley's own new one from contrib/, as /usr/bin/sort (patch 158);
bind 4.9.2 and perl 4.036, perl by OpenBSD's Makefile.bsd-wrapper, so
that its own Configure-written makefile stays as Berkeley shipped it
(patch 159); grep, egrep and fgrep -- GNU grep 2.0 as NetBSD 1.1 had
it, with 4.4BSD's -o, the system's in place of Lite2's, whose -v and
the like ran AT&T's old greps, which the Lite cut removed (patch 160);
groff 1.08, the system's nroff, troff, tbl, eqn, pic, refer and the
rest in /usr/bin, as Berkeley configured it, without groff's soelim
and me(7), which would replace Lite2's own, as NetBSD 1.0 built it --
built with GCC 2.95.3's C++, now built with it, g++ and c++ and its
library, libstdc++ 2.10.0 (patch 164); rcs 5.6, as Berkeley
configured it but for GNU diff's lines, with the conf.h its conf.sh
wrote on the system, kept as FreeBSD 2.0 kept the one Wollman
regenerated (decided 2026-10-19, patch 172); emacs 18.57, built on
the system, where it dumps itself, ported as GNU's emacs 20.1 ported
unexelf.c to NetBSD, and passed over by the build on another machine
(decided 2026-10-22, patch 173; install.md, *Emacs*).
Not built: libg++-2.3, written for g++ 2.3. GCC 2.95.3's libstdc++ is
the system's C++ library, as each BSD dropped libg++ when it took egcs
or GCC 2.95 -- NetBSD in 1.4 (1999), OpenBSD in May 1999 ("old libg++,
replaced by part of egcs"), FreeBSD by 4.0 (2000), none having paired
libg++ with GCC 2.95 -- and nothing in the tree uses it; it stays as
Lite2 shipped it, as gcc-2.3.3, gas-1.38 and gdb-4.7.LBL do (decided
2026-10-22, patch 175). With it, contrib is done.

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

What it needs: make and /usr/share/mk, tr, sort, patch, xargs, cksum,
gzip, mtree, ftp and tar, which the base set now carries (checked
2026-10-22; whether its tar reads gzip is not yet checked); and from
FreeBSD 2.2, pkg_install (pkg_add, pkg_create, pkg_delete, pkg_info),
md5 and fetch.

**Deferred: removing a program together with what it pulled in.**
Installing a port installs its dependencies with it; removing one
removes that package alone, as in FreeBSD at the time, and leftover
dependencies are removed by hand, newest first. pkg_cutleaves (Stefan
Walter, 2003; the 2006 version uses plain pkg_delete) is the period
tool that does this, as a port: it needs Perl 5, which FreeBSD 2.2's
ports tree carries as lang/perl5 (the base system has Perl 4.036, in
contrib/perl-4.036). To be taken up after the framework is in.

## Berkeley's driver framework on the i386

**Decided (2026-10-18): Torek's autoconfiguration for the i386, and
the old interface abandoned; OpenBSD's line followed.** Lite2 carries
two driver interfaces. The i386's, like the vax's, hp300's and pmax's,
is the old one each port has its own form of -- here 386BSD's
isa_driver and isa_device, a table config(8) writes, probed in order
at the addresses it names. The sparc's, and the shared SCSI code's in
sys/dev/scsi, is Chris Torek's of 1992 (LBL): struct device,
cfdriver, config.new and kern/subr_autoconf.c, a tree of devices each
bus finds its children for, by match and attach. FreeBSD kept and
grew the old one through 3.5.1 and replaced it with newbus in 4.0;
NetBSD moved its i386 to Torek's in 1994-95 and OpenBSD forked from
it in 1995. The i386 is to go wholly to Torek's, as NetBSD's did,
with no wrapper for old drivers: each driver is converted or replaced.

Which descendant, measured against Lite2 (research of 2026-10-18,
NetBSD and OpenBSD histories):

- The framework. NetBSD 1.0's device.h is Lite2's but for 2 lines;
  1.1's differs by 14 and its subr_autoconf.c by 222 (indirect
  configuration, config_scan), and OpenBSD of 1996-01-01 is
  byte-identical to 1.1. From there OpenBSD's device.h has stayed
  within 180 changed lines of Lite2's to 2026: struct device still at
  the head of each softc, allocated by ca_devsize, cfattach in
  NetBSD 1.2's shape with detach and activate, cf_parents and cf_loc,
  config.new's grammar with `attach' added. NetBSD's grew to 749
  (device.h) and 4017 (subr_autoconf.c): CFATTACH_DECL and
  name-based cfdata (2.0), interface attributes (4.0), the softc
  split from an opaque device_t (5.0), cfargs and devhandles now.
- Driver-facing interfaces. OpenBSD keeps Lite2's cdevsw tables in
  conf.c with struct proc, ether_ifattach(ifp), if_watchdog and
  dv_xname to 2026; its walls are the 2015 network rework (if_input
  with mbuf lists, ifq_*), m_devget without an ifp (2015), and
  malloc's flags renumbered in 2010 (M_WAITOK no longer 0: a Lite2
  call with 0 panics there). NetBSD's go from 2000-02 (bufq, the
  if_input hook, devsw_attach, CFATTACH_DECL) to a port, not a shim,
  in 2005-08 (struct lwp in devsw, the softc split, three hardware
  IPLs, mutexes and condvars). bus_space and bus_dma, NetBSD 1.2-1.3,
  are in both, and stable.
- Locking. Lite2 has spl, tsleep and wakeup, McKusick's lockmgr and
  simple_lock, written for multiprocessors and compiled away without
  them. OpenBSD went by small steps, each keeping the last working:
  rwlock (2003), MP with one recursive big lock, __mp_lock and
  KERNEL_LOCK, around a uniprocessor kernel (2004), mutex(9) with spl
  semantics (2004), msleep (2007), lockmgr's API on rwlocks (2013)
  until 2016, simple_lock gone in 2015, MP-safe interrupts (2013),
  NET_LOCK (2016-17); spl, tsleep and a uniprocessor GENERIC are
  still there. NetBSD's big lock was lockmgr's own spin mode (2002),
  then 5.0 (2007-08) replaced the model at once: lockmgr, simple_lock
  and ltsleep gone, adaptive mutexes, condvars, turnstiles, threaded
  soft interrupts, preemption, MULTIPROCESSOR mandatory on i386.

So: the i386 brought onto Torek's from NetBSD 1.1, the code OpenBSD
began from (device.h, subr_autoconf.c, isa_machdep's isa_intr_establish
with the interrupt-mask rework, isa.c, autoconf.c's configure(), the
new-syntax files.i386 and GENERIC), then NetBSD 1.2's cfattach split
and mainbus, then bus_space and bus_dma as OpenBSD took them in 1996
and 1998; drivers after that from OpenBSD, as far forward as the
walls above allow. The first steps towards locking are OpenBSD's: a
KERNEL_LOCK that is nothing on one processor, then mutex(9) as spl
and an owner check, then rwlocks, msleep, and lockmgr on rwlocks with
its LK_* interface kept -- spl, tsleep and the drivers left as they
are. if_ed and wd, FreeBSD 2.1.0's, are old-interface drivers taken
for now, to be replaced as the i386 moves.

**The order** (decided 2026-10-18), each stage passing the whole
QEMU test -- the CD boots and installs, the disk boots, the network
answers -- before the next begins:

1. The framework: NetBSD 1.1's changes to Lite2's device.h and
   subr_autoconf.c (14 and 222 lines), the code OpenBSD began from;
   Lite2's sparc and sys/dev/scsi, Torek's users already, still built;
   config.new the i386's config, as NetBSD 1.1 made it.
2. The i386's bus and interrupts: NetBSD 1.1's isa.c and
   isa_machdep.c, isa_intr_establish, and the interrupt-mask rework of
   icu.s and vector.s in place of config's generated glue (mkglue.c);
   configure() starting the tree; then NetBSD 1.2's cfattach split and
   mainbus -- the shape OpenBSD has kept.
3. Every i386 driver converted at once, as NetBSD's were in November
   1994, since without the old interface the kernel boots only when
   its console and disks are converted: cpu, clock and npx; the
   console and com (the console is a serial line); wdc and wd with
   ATAPI and wcd; fd; Ethernet (NetBSD 1.2's or OpenBSD's of 1996, for
   the stopgap if_ed); wt. All from one donor snapshot, OpenBSD of
   1996-97 or NetBSD 1.2, so that they speak one set of interfaces.
4. bus_space and bus_dma, as OpenBSD took them from NetBSD (1996 and
   1998); then PCI -- the bus, a PCI-ISA bridge, PCI IDE -- from
   OpenBSD. (Whether this stage is 4.5BSD's is to be settled, above.)
Stage 5, locking, OpenBSD's way, is 4.6BSD's (roadmap-4.6.md, *Kernel
primitives*).

**How drivers are taken**, so that they stay this tree's:

- The kernel's interfaces move first, and each driver comes from the
  donor snapshot whose interfaces the kernel has -- one snapshot for a
  subsystem.
- A driver needing an interface the kernel lacks (bus_space,
  isa_intr_establish, timeout_set) brings that interface in, from the
  same donor and era, with its provenance; the driver's calls are
  left alone.
- Compatibility macros for renames only, never for changed meaning:
  OpenBSD's malloc flags of 2010, where a Lite2 call passing 0 panics,
  are fixed at the call or avoided by an earlier driver.
- The walls are respected: OpenBSD's network rework of 2015 and its
  malloc flags of 2010; NetBSD after 1.3.
- Each import as for wd and if_ed: a commit as released, a commit
  adapting it, each change marked at its site, an entry in
  provenance/imports.md, and the licence checked under this tree's
  rules (no Caldera-licensed code a BSD did not take; nothing under
  the old Berkeley notice).
- config's language is Torek's, grown only by what the donor grew
  (NetBSD 1.2's `attach ... with').
