# What 4.4BSD-Lite2 names and does not contain

The build lists in this tree refer to programs, libraries, sources and
documents that are not here. None of them is missing from this fork:
4.4BSD-Lite lacks every one of them too, so they were never shipped in
the Lite releases rather than lost since. This file records what they
are, so that a build failure that traces back to one of them is
recognised rather than investigated again.

Measured against 4.4BSD-Lite, NetBSD 1.0, FreeBSD 2.0.5 and OpenBSD's
1996 tree.

## Programs and libraries named by SUBDIR

None is in 4.4BSD-Lite. The right-hand columns say which of the
contemporaries carry a program of that name -- in every case their own
rewrite, not Berkeley's code.

| named in | entry | NetBSD 1.0 | FreeBSD 2.0.5 | OpenBSD 1996 |
|---|---|:--:|:--:|:--:|
| `bin` | `ed` | yes | yes | yes |
| `bin` | `expr` | yes | yes | yes |
| `sbin` | `fsdb` | — | — | yes |
| `sbin` | `icheck` | — | — | — |
| `sbin` | `ncheck` | — | — | — |
| `usr.bin` | `at` | yes | yes | yes |
| `usr.bin` | `bc` | — | — | — |
| `usr.bin` | `compress` | yes | yes | yes |
| `usr.bin` | `dc` | — | — | — |
| `usr.bin` | `deroff` | — | — | — |
| `usr.bin` | `diction` | — | — | — |
| `usr.bin` | `graph` | — | — | — |
| `usr.bin` | `learn` | — | — | — |
| `usr.bin` | `plot` | — | — | — |
| `usr.bin` | `ptx` | — | — | — |
| `usr.bin` | `spell` | — | — | — |
| `usr.bin` | `spline` | — | — | — |
| `usr.bin` | `struct` | — | — | — |
| `usr.bin` | `units` | yes | — | yes |
| `usr.bin` | `xsend` | — | — | — |
| `usr.sbin` | `cron` | yes | yes | yes |
| `usr.sbin` | `mkproto` | — | — | — |
| `usr.sbin` | `sa` | yes | yes | yes |
| `lib` | `libmp` | — | — | — |
| `lib` | `libplot` | — | — | — |

The shape of that list is the settlement and a patent. The seventeen
that no BSD has are the AT&T-derived tools -- bc, dc, spell, learn,
struct, diction, ptx and the rest. The eight every BSD has are the
ones those projects rewrote from nothing after the lawsuit: their own
ed, expr, at, cron and sa. compress is there because they were willing
to carry the LZW patent, which Berkeley was not.

Also named and absent, in directories whose lists were trimmed so that
what is present would build:

  usr.bin/grep	old.bin.grep, old.egrep, old.fgrep, old.ucb.grep
  usr.bin/diff	diffh
  usr.bin/pascal	eyacc

## Documents named by SUBDIR

  share/doc/psd	01.cacm 02.implement 03.iosys 04.uprog 06.Clang
		08.f77 09.f77io 11.adb 15.yacc 16.lex 17.m4
  share/doc/usd	01.begin 02.learn 03.shell 05.dc 06.bc 09.edtut
		10.edadv 15.sed 16.awk 17.msmacros 21.troff
		22.trofftut 23.eqn 24.eqnguide 25.tbl 26.refer
		27.invert 29.diction
  share/doc/smm	08.sendmailop 09.sendmail 14.uucpimpl 15.uucpnet
		16.security 17.password
  share/man	man3f
  sys/tahoe/stand	vdformat

These are the papers describing the same removed tools, and the
sendmail and uucp documents.

## Sources named by SRCS

  lib/libcompat		thirteen of its twenty-two: the whole of 4.1
			(ftime.c, getpw.c, gtty.c, stty.c, tell.c,
			vlimit.c, vtimes.c) and six of 4.3 (ecvt.c,
			gcvt.c, regex.c, sibuf.c, sobuf.c, strout.c).
			The Makefile now names the nine that are here.
  lib/libkvm		kvm_i386.c. Berkeley never wrote one: this tree
			has hp300, luna68k, mips and sparc. Ten
			programs are waiting on it.
  usr.bin/pascal	libcpats.c.
  usr.bin/vmstat	names.c has cases for hp300, tahoe, vax, luna68k,
			mips and sun, and none for i386, so read_names
			was undefined and vmstat, vmstat.sparc and
			systat did not link. NetBSD 1.0's i386 case is
			now in the file: a stub returning success,
			which leaves vmstat.c's own fallback to label
			the drives ??0, ??1 and so on. It is the same
			stub NetBSD gives pc532.

			Deliberately not taken: FreeBSD 2.0.5's real
			implementation by Rodney W. Grimes, which reads
			namelist[X_DK_NAMES] from the kernel and gives
			the drives their true names. It depends on a
			kernel symbol this tree has not been shown to
			export, which cannot be checked until a kernel
			built from this tree runs. When one does, and
			if true drive names are wanted, that is the
			place to build this out faithfully. Neither
			lineage kept the file: NetBSD replaced it with
			dkstats.c at 1.2, FreeBSD dropped it by 3.0,
			and OpenBSD already had dkstats.c in 1996.
  sbin/savecore		zopen.c, which came from usr.bin/compress.
			Imported from NetBSD 1.0, the only file taken
			from another system so far.

## The kernel

Separate from all of the above, and larger: 35 functions in eight
files have their bodies deleted, with `Body deleted' where the code
was. 4.4BSD-Lite is identical here -- the same 35 in the same eight
files, seven of them byte-for-byte -- so the settlement removed them
from both releases.

  kern/vfs_bio.c	14, the buffer cache: bread, bwrite, brelse
			and the rest
  kern/kern_exec.c	execve
  kern/tty_subr.c	10, the character queues
  kern/kern_acct.c, kern/kern_physio.c, kern/subr_rmap.c,
  kern/sys_process.c, hp/dev/hil_subr.c

Nothing can run until these exist. They are not to be taken from XNU
or Rhapsody, which keep the UNIX System Laboratories notice on the
same files (see precedent.md); NetBSD 1.0 is the BSD-licensed donor.

## What is not being done

None of the above is being restored. This fork is 4.4BSD-Lite2 as
Berkeley shipped it, built with a modern toolchain; taking ed, expr,
at, cron, sa and compress from NetBSD or FreeBSD would make it a
hybrid of this tree and theirs. Where a missing file blocks something
that is here -- zopen.c did, kvm_i386.c does -- that is decided one
file at a time, and recorded.
