# The system installs its own /bin

Until now the root image held what `build/addfile-host.c` could put on
it: `/sbin/init`, `/bin/sh` and `/dev/console`. Twenty-four of the
twenty-five programs in `usr/src/bin` build -- only `csh` fails, on a
generated `const.h` -- but the host tool could not place them, and the
way past that turned out to be the most searching test this port has
had: have the system install them itself, the way 4.4BSD did from a
distribution tape, with its own `dd` and its own `pax` reading a raw
disk.

It found five faults, four of them in the system. This records the
procedure and what each run showed.

## The procedure

One disk image, labelled twice so the filesystem covers only its
first half and an archive can sit in the second:

	dd if=/dev/zero of=disk.img bs=1M count=80
	mklabel disk.img 40960          # partition a: the first 20 MB
	mkfs-host disk.img              # the filesystem fills a
	mklabel disk.img 163840         # c now spans all 80 MB

The second label leaves partition a larger than the filesystem in it,
which is harmless -- FFS mounts by the superblock's size, as a
filesystem made with `newfs -s' always has -- and makes c reach the
archive.

The scaffolding the install needs, one file to a directory because
that is what `addfile-host` does with each pair:

	sbin/init  bin/sh  dev/console        the system as before
	stand1/pax                            the extractor
	stand2/dd                             to read past the filesystem
	stand3/rwd0c  c 3 2                   wd0c, character: raw reads
	stand4/null   c 2 2                   for measuring
	stand5/mount                          to make the root writable
	stand6/wd0a   b 0 0                   the root, block, for mount
	etc/fstab     /stand6/wd0a / ufs rw 1 1

The device numbers are `etc/etc.i386/MAKEDEV`'s. The archive, made on
the host with `pax -w -x ustar` from a staging tree holding `bin/*`,
is written at sector 40960, the start of the second half:

	dd if=dist.tar of=disk.img bs=512 seek=40960 conv=notrunc

Boot single user, then:

	# /stand5/mount -u /
	# cd /
	# /stand2/dd if=/stand3/rwd0c bs=10240 skip=2048 count=284 | \
	      TMPDIR=/ /stand1/pax -r -pe

`skip=2048` is sector 40960 in 10240-byte records; `count=284` is the
archive's length in them.

Three of those lines exist because of the environment, not the system:

- **`mount -u /`**: single user mounts the root read only, as 4.4BSD
  always has; `/etc/rc` is what makes it writable, and there is none.
- **`/stand6/wd0a` and the fstab line**: `mount -u /` looks the root up
  in fstab, and ffs_mount refuses an update unless the device it names
  is a block device -- the same vnode the root was mounted from, which
  `checkalias` makes it.
- **`TMPDIR=/`**: see *Noted, not changed* below.

## What it found, in the order it found it

Each was isolated before it was fixed, one QEMU run per question, and
each fix is a single commit.

**1. `__syscall` was nosys.** `dd if=<any regular file>` died with
`Bad system call`, status 140. lseek, mmap, truncate and ftruncate all
reach the kernel through syscall 198, which each port's `syscall()`
must intercept, and the i386 did not. Commit *i386: the off_t system
calls, which had never worked*, after hp300.

**2. Quad arguments were truncated.** With 198 handled, `pax -f` on a
file found the same header for ever: `syscall()` copied `sy_narg`
words, not `sy_argsize` bytes, so lseek lost its `whence`. Same
commit, after hp300.

**3. A failing off_t call returned (garbage : -1).** libc's `cerror`
set `%eax` and not `%edx`, so pax took a pipe for a seekable file and
stopped after one member. Same commit, after hp300, luna68k, sparc and
mips, every port whose libc was carried to 4.4BSD's off_t.

**4. Opening a raw partition reset the drive under the mounted root.**
`dd if=rwd0c | pax` hung, or panicked in `wdintr` on a NULL buffer:
`wdopen` emptied the drive's queue on every open, including while pax
paged in from wd0a. Commit *wd: an open, labelled drive is not reset
by a later open*, after FreeBSD 2.0.

**5. copyout's protection faults panicked.** The first `echo /bin/*`
after pax had written into /bin panicked at copyout's `rep movsl`.
CR0_WP and copyout's fast path came from NetBSD 1.0; the line in
`trap()` that NetBSD 1.0 disabled for them did not. Commit *i386: let
copyout's protection faults reach vm_fault*, after NetBSD 1.0.

And one in this project's own tool, found the same way:

**6. Directories made by `addfile-host` read as empty.** Each entry
ran past the 512-byte chunk it belonged in. Commit
*build/addfile-host.c: directories of one DIRBLKSIZ; block devices*,
after mkfs.c's own `makedir()`.

Four of the six were answered inside this tree -- three by other
ports, one by mkfs.c -- and two by the descendants finishing a job
this tree had already started from them: the CR0_WP half of copyout
from NetBSD 1.0, and FreeBSD 2.0's reading of the same 386BSD driver.

## The result

	# ls -l /bin
	total 5824
	-r-xr-xr-x  1 0  0   65136 Oct 12 00:49 cat
	-r-xr-xr-x  1 0  0   86456 Oct 12 00:49 chmod
	...
	-r-xr-xr-x  1 0  0   73672 Oct 12 00:49 test
	# echo hello world | cat
	hello world
	# date
	Mon Oct 12 08:17:25  2026
	# df
	Filesystem   512-blocks     Used    Avail Capacity  Mounted on
	/stand6/wd0a      39038     7366    29720    20%    /
	# mkdir /tmp && echo made > /tmp/f && cat /tmp/f && cp /tmp/f /tmp/g \
	    && mv /tmp/g /tmp/h && ls /tmp && rm /tmp/f /tmp/h && rmdir /tmp \
	    && echo cleaned
	made
	f       h
	cleaned
	# test -f /bin/ls && echo test-ok
	test-ok

All twenty-five, the right sizes and modes, extracted by the system
from a raw disk and then run. `ps -ax` says `/dev/mem: No such file or
directory`, which is the image lacking a node and not a fault.

The double space in `date`'s output is an empty time zone name. There
is no `/etc/localtime` on the image; whether that is the whole reason
has not been looked into.

## Noted, not changed

- **`pax -r` does nothing, silently, without a writable temporary
  directory.** `dir_start()` in `bin/pax/tables.c` calls
  `tempnam(NULL, NULL)` and, if that fails, returns -1 with no warning;
  `extract()` then returns, and pax exits 0 having extracted nothing.
  There is no `/tmp` or `/var/tmp` on this image, hence `TMPDIR=/`.
  It would have saved a run to have said so; later paxes do.
- **`wdopen` still resets a drive left `OPENRAW`**, opened with no
  label, on every open -- the same hazard as fault 4 for raw access to
  an unlabelled drive with other I/O in flight. None is in use here.
  FreeBSD 2.0's `DKFL_LABELLING` and `wdsleep()` are the answer when
  one is.
- **luna68k's `syscall()` still copies `sy_narg` words**, so fault 2 is
  latent there; its `__syscall` switch is hp300's. That port is not
  built here.
- **`wdclose` keeps every partition ever opened in `dk_openpart`.** The
  `overlaps open partition (a)` warning on opening wd0c is correct here,
  because the root is mounted, but would also print after (a) closed.
  FreeBSD 2.0's `wdclose` maintains the open masks.
- **MAKEDEV and wd.c disagree on minors for drive 1 and up.** MAKEDEV
  numbers `wd${unit}a` as `unit*8 + part`; wd.c decodes the unit from
  bits 5-6, `unit<<5`. They agree only for drive 0. And wd.c sizes
  everything by `NWD`, which `config` sets from the number of `wd`
  controller lines, not drives -- so `disk dk1 at wd0 drive 1` is a
  no-op here, which was tried first and is why this install uses one
  disk and not two.
- **`trap` takes signal numbers only** (`docs/provenance/pwd.md`).

# Multi-user, to a login prompt

The same procedure carried one step further: everything this tree
builds and needs for `init` to run `/etc/rc` and `getty`, plus etc/'s
own files, installed by the system from a raw disk, then booted
without -s. It reaches `login:`, takes root, runs csh, and reboots
cleanly. Two faults in the kernel turned up on the way, and one
missing program.

## Staging

The target root build/sysroot.sh makes is the staging tree, as
NetBSD's build.sh uses its DESTDIR: the programs are installed into it
by their own Makefiles, etc/'s files by build/distribution.sh, and the
whole of it archived.

	export L2_BUILD=$HOME/.cache/lite2
	sh build/sysroot.sh
	r=$(pwd)
	for d in bin sbin libexec/getty usr.bin/login usr.bin/passwd \
	    usr.sbin/pwd_mkdb usr.sbin/dev_mkdb usr.sbin/kvm_mkdb \
	    usr.sbin/syslogd usr.sbin/chown usr.sbin/update \
	    usr.bin/logger usr.bin/find usr.bin/true usr.bin/false \
	    usr.bin/tty usr.bin/id usr.bin/who usr.bin/more usr.bin/wc \
	    usr.bin/head usr.bin/tail usr.bin/env usr.bin/uname usr.bin/w
	do
		(cd usr/src/$d && sh "$r/build/make.sh" \
		    depend all install NOMAN=noman INSTALLFLAGS=)
	done
	sh build/kernel.sh
	install -m 755 usr/src/sys/compile/LINK.i386/vmunix \
	    $L2_BUILD/root/vmunix
	sh build/distribution.sh $L2_BUILD/root
	tar -C $L2_BUILD/root --numeric-owner --format=ustar \
	    -cf dist.tar .

`INSTALLFLAGS=` because rcp's Makefile sets `-fschg`, a 4.4BSD file
flag GNU install does not know. `NOMAN` because there is no nroff.
Of sbin, all but disklabel is installed: its Makefile formats its
manual page whatever NOMAN says. reboot's install stops on
`boot_hp300.0`, and id's on `groups.0`, after the programs themselves
are in place. `/vmunix` is there for ps,
kvm_mkdb and savecore, which read the kernel's namelist; the shim
loads the kernel from the host, so nothing else would put it there.

The disk is the earlier one at twice the size, since the archive is
16 MB and the root holds it unpacked:

	dd if=/dev/zero of=disk.img bs=512 count=245760	# 120 MB
	mklabel disk.img 81920
	mkfs-host disk.img
	addfile-host disk.img ...	# the scaffolding, as before
	mklabel disk.img 245760
	dd if=dist.tar of=disk.img bs=512 seek=81920 conv=notrunc

## Installing

Single user, as before, then the two steps build/distribution.sh
leaves to the system:

	# /stand5/mount -u /
	# cd /
	# /stand2/dd if=/stand3/rwd0c bs=10240 skip=4096 count=1611 | \
	      TMPDIR=/ /stand1/pax -r -pe
	# /usr/sbin/pwd_mkdb -p /etc/master.passwd
	# cd /dev; sh MAKEDEV std wd0; cd /
	# echo '/dev/wd0a / ufs rw 1 1' > /etc/fstab
	# /sbin/reboot

`count` is the archive's length in 10240-byte records, 1611 here.
Then multi-user, log in as root, and only then remove the scaffolding
(see *Noted, not changed* below for why not before):

	login: root
	myname# rm -rf /stand1 /stand2 /stand3 /stand4 /stand5 /stand6

## What it found

**7. MAKEDEV could make no disk.** `sh MAKEDEV wd0` said `bad unit
for disk in: wd0`: every minor it computes, it computes with expr, and
4.4BSD-Lite has no expr. Restored from NetBSD 1.0; see
`docs/provenance/imports.md`, kind E.

**8. Every reboot with a writable filesystem panicked.** `syncing
disks...` and a page fault at address 0x10: i386's `boot()` called
`sync((struct sigcontext *)0)`, a call of an older sync, and sync
reads `p->p_ucred`. Commit *i386: boot() passes proc0 to sync*, as
sparc's boot() in this tree does.

**9. Seventeen thousand stray interrupts on the console.** `ISA
strayintr 0`, once per disk transfer or so, for the length of the
extraction. The number was wrong -- `ffs()` of an irq number, which
is 0 for every odd one -- and the irq was 15, the slave controller's
lowest line. The likely cause is the 8259 itself, which reports a
request masked between being raised and being acknowledged on that
line; that is not confirmed beyond the number. Every BSD logs these;
NetBSD 1.0 stops after five, FreeBSD 2.0 after five for each irq. Commit *i386: report a stray
interrupt by number, five times*.

## The result

	Automatic boot in progress: starting file system checks.
	/dev/rwd0a: FILESYSTEM CLEAN; SKIPPING CHECKS
	starting network
	...
	4.4BSD-Lite (myname.my.domain) (console)

	login: root
	Copyright (c) 1980, 1983, 1986, 1988, 1990, 1991, 1993, 1994
	        The Regents of the University of California.   All rights reserved.

	tset: Command not found.
	erase ^H, kill ^U, intr ^C status ^T
	Don't login as root, use the su command.
	myname# echo $shell; tty; who
	/bin/csh
	/dev/console
	root     console Oct 12 12:39
	myname# ps -ax
	  PID TT  STAT      TIME COMMAND
	    0 ??  DLs    0:00.01 (swapper)
	    1 ??  Is     0:00.04 (init)
	    2 ??  DL     0:00.01 (pagedaemon)
	   31 ??  Is     0:00.07 (syslogd)
	   45 ??  Is     0:00.00 (update)
	...
	myname# reboot
	syncing disks... done

`logout` brings getty back; a second login prints `Last login: ... on
console`. /etc/rc runs to the end, reporting each program this tree
does not build or install yet -- install, sed, sysctl, timed, quotaon,
lpd, sendmail, inetd, amd, portmap -- and the NFS daemons' missing
system call; none stops it.

## Noted, not changed, from this run

- **Removing the device node the root was remounted through, then
  rebooting, panics** with `vinvalbuf: dirty bufs`. `rm -rf /stand6`
  in single user does it; `/stand1` to `/stand5` do not, and neither
  does removing `/stand6` after a multi-user boot, when the root was
  remounted through `/dev/wd0a` instead. The node's vnode is the
  mounted device's, by `checkalias`. Not looked into further; the
  procedure above avoids it.
- **getty's banner and prompt arrive with the eighth bit set**: the
  `default` gettytab entry is `:ap:`, and getty then sends even
  parity, as 4.4BSD always did. Correct for a 7-bit terminal; a modern
  serial console shows `M-` characters. NetBSD 1.0 does the same.
- **The host's install resolves group names.** Makefiles that set
  BINGRP got Debian's numbers for it, not this tree's: ps, w and
  dmesg are setgid kmem, 15 for 2, and ps shows commands in
  parentheses; df and shutdown are operator, 37 for 5; dump and
  restore are tty, Debian's 5, which here is operator.
  distribution.sh resolves its own names from etc/group; the program
  installs do not.
- **MAKEDEV's majors are partly wrong for this kernel.** Its `pty`
  case makes `c 4` and `c 5` where conf.c has 5 and 6 (4 is drum,
  which `std` also gives 4). `std` makes stdin, stdout, stderr and
  fd/ at major 53, which the i386 cdevsw, ending at 14, does not have;
  fd/ needs awk, which there is none of; and `chown` is looked for in
  /bin and /usr/bin and lives in /usr/sbin.
- **root's group is 10**, which etc/group does not have; `id` says
  `gid=10`. That is Berkeley's master.passwd as shipped.
- **Without root, the archive's owners are yours.** distribution.sh
  passes -o and -g only as uid 0. Unprivileged, either run the whole
  staging under fakeroot(1), or give tar `--owner=0 --group=0`, which
  loses operator on dumpdates, staff on lastlog and utmp on utmp --
  /etc/rc remakes utmp in any case.
