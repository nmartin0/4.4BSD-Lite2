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
	    usr.bin/head usr.bin/tail usr.bin/env usr.bin/uname usr.bin/w \
	    usr.bin/xinstall usr.bin/sed usr.bin/tset usr.sbin/sysctl \
	    usr.sbin/quotaon usr.sbin/accton usr.sbin/timed usr.sbin/lpr \
	    usr.sbin/inetd usr.sbin/portmap usr.sbin/amd usr.bin/cap_mkdb \
	    usr.bin/script usr.sbin/pstat usr.bin/awk
	do
		(cd usr/src/$d && sh "$r/build/make.sh" \
		    depend all install NOMAN=noman)
	done
	(cd usr/src/share/termcap && sh "$r/build/make.sh" termcap)
	$L2_BUILD/tools/bin/install -o bin -g bin -m 444 \
	    $L2_BUILD/obj$r/usr/src/share/termcap/termcap \
	    $L2_BUILD/root/usr/share/misc/termcap
	sh build/kernel.sh
	install -m 755 usr/src/sys/compile/LINK.i386/vmunix \
	    $L2_BUILD/root/vmunix
	sh build/distribution.sh $L2_BUILD/root
	tar -C $L2_BUILD/root --numeric-owner --format=ustar \
	    -cf dist.tar .

`NOMAN` because there is no nroff. Run as root, or under fakeroot,
for the tree's owners and groups: build/install.sh, which every
Makefile's `install' goes through, turns `bin`, `kmem`, `operator`
and the rest into this tree's numbers, and drops them unprivileged;
and make.sh gives BINOWN and BINGRP as bin's numbers, so the few
Makefiles that run chown themselves get them too. Still the host's:
a name a Makefile writes into its own chown (games.bin), and
SHARED=copies, which copies the headers with pax -pa and so with the
source tree's owners. It
also drops `-fschg`, a 4.4BSD file flag rcp's Makefile asks for and a
Linux filesystem cannot hold; an earlier form of this recipe passed
`INSTALLFLAGS=` for that.

termcap is share/termcap's own `termcap' target -- `ex' on the host
running the tree's reorder script over termcap.src -- installed by
hand, through the same install wrapper, because its Makefile's install
also wants termcap.db, which cap_mkdb makes, and cap_mkdb is a program
for the target. It is run on the system below, as pwd_mkdb is. Host
`ex' is Debian's vim; it runs the script as written.
Of sbin, all but disklabel is installed: its Makefile formats its
manual page whatever NOMAN says. reboot's install stops on
`boot_hp300.0`, and id's on `groups.0`, after the programs themselves
are in place. `/vmunix` is there for ps,
kvm_mkdb and savecore, which read the kernel's namelist; the shim
loads the kernel from the host, so nothing else would put it there.

The disk is the earlier one at twice the size, since the archive is
16 MB and the root holds it unpacked:

	dd if=/dev/zero of=disk.img bs=512 count=278528	# 136 MB
	mklabel disk.img 81920
	mkfs-host disk.img
	addfile-host disk.img ...	# the scaffolding, as before
	mklabel disk.img 278528 32768	# b: 16 MB of swap
	dd if=dist.tar of=disk.img bs=512 seek=81920 conv=notrunc

## Installing

Single user, as before, then the steps the host cannot do --
build/distribution.sh's two, and cap_mkdb:

	# /stand5/mount -u /
	# cd /
	# /stand2/dd if=/stand3/rwd0c bs=10240 skip=4096 count=1611 | \
	      TMPDIR=/ /stand1/pax -r -pe
	# /usr/sbin/pwd_mkdb -p /etc/master.passwd
	# cd /dev; sh MAKEDEV std wd0 pty0; cd /
	# /usr/bin/cap_mkdb /usr/share/misc/termcap
	# echo '/dev/wd0a / ufs rw 1 1' > /etc/fstab
	# echo '/dev/wd0b none swap sw 0 0' >> /etc/fstab
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
- **The host's install resolved group names** -- ps, w and dmesg
  setgid to Debian's kmem, 15 for this tree's 2; df and shutdown to
  its operator, 37 for 5; dump and restore to its tty, 5, which here
  is operator. Answered since by build/install.sh. Measured after:
  `-r-xr-sr-x  1 bin  kmem  /bin/ps`.
- **ps shows every command in parentheses** all the same, and w its
  own. That is not the group: kvm_getargv reads struct ps_strings at
  the top of the process's stack, and this tree's execve -- a body
  written for this project, kern/kern_exec.c -- copies the arguments
  but never fills ps_strings in. Recorded in docs/roadmap.md.
- **MAKEDEV's majors are partly wrong for this kernel.** Its `pty`
  case made `c 4` and `c 5` where conf.c has 5 and 6 (4 is drum,
  which `std` also gives 4) -- fixed since; see *Pseudo-terminals*. `std` makes stdin, stdout, stderr and
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

# What /etc/rc found next

Eleven more directories, for programs /etc/rc and .login run --
install, sed, sysctl, quotaon, accton, timed, lpd, inetd, portmap, amd
and tset -- were added to the staging. Each built and installed as it
stands; what running them found:

**10. Every mmap(2) panicked the kernel.** With nlist able to read
the kernel (11), the first program to map a file -- savecore, then
kvm_mkdb, then a small test program mapping /bin/csh -- stopped the system
with `trap type 3' at the first instruction of mmap(). QEMU's monitor
showed the four bytes there already overwritten at the single-user
prompt, and a watchpoint caught the writer: pmap_bootstrap's SYSMAP,
storing the PTE pointer Berkeley named `mmap'. Built -fcommon, that
uninitialised variable was a common symbol, and the linker resolved it
to the mmap() function. Nothing installed until now had called mmap()
at all -- the a.out nlist rejected the ELF kernel by its header before
mapping anything -- so the first call was the first to reach the
damage. Commit *i386: pmap's PTE pointer is XXX_mmap, not mmap*, after
NetBSD 1.0. Checked across the whole kernel: no other common symbol
resolves to code.

**11. nlist(3) and kvm_mkdb could not read an ELF kernel.**
`savecore: /vmunix: nlist: Undefined error: 0` and `kvm_mkdb:
/vmunix: bad magic number` at every boot. NetBSD 1.3's ELF readers,
for libc and for kvm_mkdb, with the definitions they need added to
sys/exec_elf.h from the same release. Commit *nlist(3), kvm_mkdb:
read ELF*; `docs/provenance/imports.md`, kind D, has the survey.

**12. tset died at every login.** `.login` runs `tset -Q \?$TERM`;
with an unknown terminal libterm's tgetent() freed an uninitialised
pointer. Commit *libterm: tgetent starts its entry pointer at NULL*,
as NetBSD 1.0 and FreeBSD 2.0 do. tset then asked again, because
there was no /usr/share/misc/termcap yet -- see below.

Afterwards, at boot: `savecore: no core dump`, a fresh
`/var/db/kvm_vmunix.db`, and `Terminal type? [unknown]`, which a
return or ^D answers.

Still reported by /etc/rc, and why:

- `named`, `sendmail`, `cron`, `/usr/libexec/ex3.7preserve`: not in
  this tree's build. bind is in contrib/; sendmail's directory holds a
  README; cron is one of the programs missing.md lists; and
  ex3.7preserve belongs to the old ex 3.7, of which usr.bin/ex keeps
  only the manual pages and USD document -- contrib's nvi does not
  provide it.
- `nfsd`, `nfsiod`, `mountd`: the kernel is configured without NFS.
- `ifconfig: ... no such interface` seven times: netstart configures
  ace0, ex0, imp0, le0, ne0, we0 and en0. Five are not in this
  configuration; ne0 and we0 are, and their probes find no card.
- `chmod: /dev/tty[pqrs]*`: no ptys. The kernel has them since and
  MAKEDEV's majors are right, but `MAKEDEV pty0' needs awk; see
  *Pseudo-terminals*.
- `accton: /var/account/acct`: the file is not made by etc/Makefile.
- `timed` prints its usage: rc passes it `$timedflags` from netstart,
  which is `YES`, a switch and not an option.

/usr/share/misc/termcap is made since -- see the staging recipe --
and `Terminal type? [unknown] vt100` is accepted.

# Swap

kern/subr_rmap.c was three stubs reading `Body deleted', and rmalloc
returned 0, so the swap pager could allocate nothing: the system lived
in RAM. It is now NetBSD 1.2's (imports.md, kind E). With it, and with
a b partition -- build/mklabel.c takes a swap size as a third argument
-- and an fstab line for it, /etc/rc's `swapon -a` enables it:

	myname# pstat -s
	Device      1K-blocks     Used    Avail Capacity  Type
	/dev/wd0b       16384        4    16380     0%    Interleaved

Measured under load with QEMU's `-m 16`, `real mem = 16384000`: a test
program that mallocs 8 MB and writes each page three times, then the
same at 12 MB alongside one asking for 20 MB, which malloc refuses (its
power-of-two request passes the 32 MB data limit). The 12 MB run
completed with the right checksum and swap showed 3284 KB in use. The
kernel finds b by itself: i386/swapgeneric.c makes the swap device the
root's minor plus one, and autoconf.c sizes it from the label.

# Pseudo-terminals

LINK.i386 now configures `pseudo-device pty 16', which GENERIC.i386
lacked and every other port's GENERIC has, and MAKEDEV's pty case makes
ttyXX at character major 5 and ptyXX at 6, where conf.c has them; it
had 4 and 5, drum's major and the slaves'. Measured with nodes made by
hand:

	myname# script /tmp/s
	Script started, output file is /tmp/s
	myname# tty; echo through-a-pty
	/dev/ttyp0
	through-a-pty

By hand, because `sh MAKEDEV pty0' generates its mknod lines with
awk, and this tree has no awk: 4.4BSD-Lite's was AT&T's and went with
the settlement. MAKEDEV std's fd/ wants it too. Recorded in
docs/roadmap.md.

# awk

usr.bin/awk is Kernighan's own, version 20121220, unchanged
(imports.md, kind E), and `sh MAKEDEV std wd0 pty0' now runs whole:
fd/ gets its 64 nodes, ttyp0-f and ptyp0-f appear at 5 and 6, and the
`chmod 666 /dev/tty[pqrs]*' in /etc/rc finds them. On the system:

	myname# echo 'a b c' | awk '{ print $2, NF }'
	b 3
	myname# awk 'BEGIN { for (i = 1; i <= 3; i++) s += i * i;
	    printf "%d %s\n", s, toupper("ok") }'
	14 OK
	myname# awk -F: '$3 == 0 { print $1 }' /etc/master.passwd
	root
	toor
	myname# echo 'x123y' | awk '{ gsub(/[[:digit:]]+/, "N"); print }'
	xNy

Two faults in the tree found on the way. include/ctype.h declared none
of the classifier functions libc defines, so awk's table of their
addresses would not compile; it declares them now, after FreeBSD
2.0.5. And MAKEDEV's PATH lacked /usr/sbin, so `std' could not find
chown; it is etc.hp300/MAKEDEV's PATH now.

# ps's arguments

ps and w showed every command in parentheses: kvm_getargv reads struct
ps_strings at PS_STRINGS, under USRSTACK, and this tree's execve put
the argument strings in those bytes and never wrote the structure.
It now puts the structure there and the strings below it, the order
NetBSD 1.0 and FreeBSD 2.0 both use, and fills it as FreeBSD 2.0's
exec_copyout_strings does. And kvm_argv refused any string address at
or above VM_MAXUSER_ADDRESS, which on this port is a page below
USRSTACK, so every argument string was refused; it bounds by USRSTACK
now, the same address on the ports where the two are equal. After:

	myname# ps -ax
	  PID TT  STAT      TIME COMMAND
	    1 ??  Ss     0:00.06 /sbin/init
	   31 ??  Ss     0:00.08 syslogd
	   52 ??  Ss     0:00.01 routed -q
	   81 ??  I      0:00.04 amd -l /var/log/amd -r -a /amd -p /home ...
	   90 ??  Ss     0:00.10 -csh (csh)
	   96 ??  R+     0:00.03 ps -ax

# Booting from the disk

Until now QEMU loaded the kernel from the host, through build/shim.
FreeBSD 4.1's boot blocks, shrink-wrapped in `sys/boot/i386` (kind D
in imports.md), load it from the disk:

	(cd usr/src/sys/boot/i386 && sh "$r/build/make.sh" all install)
	m=$L2_BUILD/root/usr/mdec
	dd if=$m/boot1 of=disk.img conv=notrunc
	dd if=$m/boot2 of=disk.img bs=512 skip=1 seek=2 conv=notrunc
	qemu-system-i386 -drive file=disk.img,format=raw -nographic

boot1 goes in sector 0 and boot2 in sectors 2 to 15; sector 1 is the
label, and the first 512 bytes of boot2 are the hole left for it. That
is the layout `disklabel -B` writes from /usr/mdec/wdboot and bootwd,
the names make install links to boot1 and boot2, so the system can
put its own boot blocks on a disk once disklabel is installed (see
*Staging*, above, for why it is not yet). Here they are written from
the host, onto the disk the install above made. The kernel must be
the one at physical 1 MB, as built since commit *i386: the kernel at
physical 1 MB*; the one before loads at 0, where a BIOS loader cannot
put it.

	Booting from Hard Disk...
	>> 4.4BSD/i386 BOOT
	Default: 0:wd(0,a)/vmunix
	boot:
	...
	changing root device to wd0a
	Automatic boot in progress: starting file system checks.
	/dev/rwd0a: FILESYSTEM CLEAN; SKIPPING CHECKS
	...
	4.4BSD-Lite (myname.my.domain) (console)

	login:

boot2 waits five seconds at `boot:`. `-s` there boots single user;
`wd(0,a)/vmunix` names a kernel; a line in /boot.config is read as if
typed. `changing root device to wd0a` is setroot decoding the boot
device boot2 passes, the first time this kernel has been given one.

boot2 writes to the screen, as 4.1's did; under `-nographic` it is
seen, and typed to, on COM1 all the same, because SeaBIOS copies the
screen to the serial line and what arrives there to the keyboard
buffer. The kernel's console is
COM1 itself. On a machine with a serial terminal and no screen,
`-h` in /boot.config moves boot2 to COM1. boot2 does not start on
COM1 unasked: with no UART there, its serial code waits for ever.

# A root inside the kernel

The install kernel carries its own root: `options MFS_ROOT=n` reserves
n KB inside the kernel, `write_mfs_in_kernel` writes a filesystem
image there after the link, and the kernel mounts it from memory
(kind D in imports.md, FreeBSD 2.0.5's). The image is made on the
host with `makefs`, NetBSD's, which Debian packages:

	sudo apt install makefs
	: ${L2_BUILD:=$HOME/.cache/lite2}; R=$L2_BUILD/root
	cd usr/src/sys/i386/conf
	sed -e 's/^ident[[:space:]]*LINK/ident		INSTALL/' \
	    -e '/^options[[:space:]]*GENERIC/d' \
	    -e 's/^config[[:space:]].*/config		vmunix	root on wd0 swap on wd0/' \
	    LINK.i386 > INSTALL
	printf 'options\t\t"MFS_ROOT=1024"\n' >> INSTALL
	cd -; sh build/kernel.sh INSTALL

	mkdir -p mr/sbin mr/bin mr/dev mr/etc mr/tmp
	cp $R/sbin/init mr/sbin; cp $R/bin/sh $R/bin/echo mr/bin
	strip mr/sbin/* mr/bin/*
	sudo mknod mr/dev/console c 0 0
	makefs -t ffs -B le -o version=1,bsize=8192,fsize=1024 \
	    -o density=4096,minfree=0,optimization=space -s 1024k mr.img mr
	cp usr/src/sys/compile/INSTALL/vmunix .
	$L2_BUILD/tools/bin/write_mfs_in_kernel vmunix mr.img

Without GENERIC, and with root and swap on wd0: with GENERIC,
setconf() would put swap and dumps on the miniroot's own major, 255,
which is past the end of the block-device table. Then a 2.88 MB floppy
image -- the size El Torito also emulates -- with boot1 and boot2
and that kernel as /vmunix in partition a, which mklabel puts at
sector 256:

	mkdir fl; cp vmunix fl
	makefs -t ffs -B le -o version=1,bsize=8192,fsize=1024 \
	    -o minfree=0,optimization=space -s $(( (5760-256-32)*512 )) \
	    fl.img fl
	dd if=/dev/zero of=floppy.img bs=512 count=5760
	mklabel floppy.img 5760
	dd if=fl.img of=floppy.img bs=512 seek=256 conv=notrunc
	dd if=$R/usr/mdec/boot1 of=floppy.img conv=notrunc
	dd if=$R/usr/mdec/boot2 of=floppy.img bs=512 skip=1 seek=2 \
	    conv=notrunc
	qemu-system-i386 -drive file=floppy.img,format=raw,if=floppy \
	    -boot a -nographic

It boots with or without a disk attached:

	>> 4.4BSD/i386 BOOT
	Default: 0:fd(0,a)/vmunix
	...
	4.4BSD-Lite #0: ...
	    .../compile/INSTALL
	...
	Enter pathname of shell or RETURN for sh:
	# PATH=/sbin:/bin; export PATH; mount
	mfs_root on / (local)
	# echo written > /tmp/x; read l < /tmp/x; echo got: $l
	got: written

The root is read-write from the start; commit *mfs: a root in memory
is mounted read-write* says why it has to be. makefs's FFS is NetBSD
2019's version 1 format, which this kernel mounts and writes.

What does not fit yet is a useful root: each program here is linked
statically, with its own copy of libc, so sh alone is 394 KB stripped
and pax 285 KB. The BSDs' install floppies put every program into one
binary with crunchgen; that is next. 1 MB was enough for init, sh
and echo; the kernel is then 1.6 MB, and a 1.44 MB floppy is out of
reach in any case.

# One binary

Each program here is linked statically, so each carries its own libc
and a root that holds the installer's tools would not fit. crunchgen,
FreeBSD 3.0's (kind D in imports.md), links them into one binary that
runs the program its argv[0] names. build/sysroot.sh builds it and
crunchide for the host, and puts a `make' that is bmake first on
build/make.sh's PATH, since crunchgen runs make by name. A
configuration names the programs, where their sources are, and the
libraries they link with:

	srcdirs /path/to/usr/src/bin /path/to/usr/src/sbin
	progs sh echo ls init mount
	libs -ll -ledit -ltermcap -lutil

and a Makefile, run through build/make.sh, runs crunchgen on it and
then the makefile crunchgen writes:

	all:
		crunchgen ${.CURDIR}/t.conf
		make -f t.mk objs exe

That builds each program's objects in its own object directory, links
each set with ld -r, hides all but its entry point with crunchide,
and links the five and libc once. The binary is 456424 bytes, against
957724 for the five stripped and apart. In the install kernel's root,
with sh, echo, ls, init and mount all hard links to it, it boots as
init and runs as each:

	# ls -li /bin /sbin; mount
	/bin:
	10 -rwxr-xr-x  5 0  0  456424 Oct 12 23:34 echo
	10 -rwxr-xr-x  5 0  0  456424 Oct 12 23:34 ls
	10 -rwxr-xr-x  5 0  0  456424 Oct 12 23:34 sh
	/sbin:
	10 -rwxr-xr-x  5 0  0  456424 Oct 12 23:34 init
	10 -rwxr-xr-x  5 0  0  456424 Oct 12 23:34 mount
	mfs_root on / (local)

makefs keeps the hard links. `make depend', which the generated
makefile runs in each program's directory first, prints mkdep's
complaints about headers it cannot find, as it does for every
program here, and goes on.

# The install root

The install kernel's root is NetBSD 1.1's i386 install floppy,
distrib/i386/floppies (kind D in imports.md), built on the host: its
list run into a directory, makefs over that, and the image written
into a kernel built with MFS_ROOT. Run as root or under fakeroot, for
the tree's owners:

	cd usr/src/distrib/i386/floppies/inst
	sh ../../../../../build/make.sh
	O=$L2_BUILD/obj$(pwd)

That crunches the 42 programs of inst-common/instbin.conf into one
binary, 1205800 bytes, and makes $O/inst-11.fs, 1600 KB, 83% full.
Then the kernel, configured as in *A root inside the kernel* with
`"MFS_ROOT=1600"`, and the floppy image as there:

	write_mfs_in_kernel usr/src/sys/compile/INSTALL/vmunix $O/inst-11.fs

The kernel is 2200040 bytes. Booted from the floppy image with a
blank disk attached:

	wd0 at 0x1f0 irq 14 on isa
	...
	wd0: bad disk label
	/etc/rc: Can't open /etc/rc
	Enter pathname of shell or RETURN for sh:
	erase ^?, werase ^W, kill ^U, intr ^C
	kernfs: Operation not supported by device
	Welcome to the NetBSD 1.1 installation program.
	...
	Proceed with installation? [n]

and at the shell, after declining, the root holds what the list put
there:

	# ls /bin /sbin; df
	/bin:
	[       chmod   dd      expr    ls      mv      rm      stty    test
	cat     cp      df      ln      mkdir   pwd     sh      sync
	/sbin:
	disklabel       init            mount_kernfs    reboot          umount
	fsck            mknod           mount_nfs       route
	halt            mount           newfs           shutdown
	ifconfig        mount_cd9660    ping            slattach
	Filesystem  512-blocks     Used    Avail Capacity  Mounted on
	mfs_root          3054     2524      530    83%    /

What is left is the installer's, step 3 of Tier 1½ in roadmap.md. It
runs as shipped and is NetBSD's: it names NetBSD 1.1, asks for SCSI
first, and uses GNU tar's options; the profile mounts a kernfs this
kernel is not configured with. And the prompt for a shell's pathname
is this tree's init, built with DEBUGSHELL, which goes to single user
because there is no /etc/rc; RETURN gives sh, whose /.profile starts
the installer.

# Installing with the installer

NetBSD 1.1's installer, adapted (kind D in imports.md), now installs
the system. The sets have to reach it, and this install kernel can
read one disk and nothing else: no CD driver, no floppy driver, and
the wd driver knows a single drive. So, until a CD driver is in, the
base set -- a ustar archive of the staged root, as dist.tar in
*Multi-user, to a login prompt* -- goes on the target disk past the
part given to 4.4BSD, and is read from there raw, as that section
reads it:

	dd if=/dev/zero of=disk.img bs=512 count=252000	# 250 x 16 x 63
	dd if=base.tar of=disk.img bs=512 seek=211680 conv=notrunc
	qemu-system-i386 -drive file=floppy.img,format=raw,if=floppy \
	    -drive file=disk.img,format=raw -boot a -nographic

At the installer: IDE, wd0; 512-byte sectors, 250 cylinders, 16
heads, 63 sectors; cylinders as the unit; 203 of them for 4.4BSD at
offset 0, 160 for root and 43 for swap; `yes'. It writes the disktab
entry, labels the disk with disklabel -w -B -- which writes boot1 and
boot2 from /usr/mdec -- makes and mounts the root, copies its own
root over and writes an fstab. Then, at its shell:

	# dd if=/dev/rwd0d bs=10240 skip=10584 count=2008 | \
	      (cd /mnt; tar -xpf -)
	# Configure
	What is this machine's hostname? [unknown.host.domain] testhost.my.domain
	...
	Making the password and termcap databases... done.
	Making device nodes (may take a while)... done.
	# umount /mnt; halt

skip is sector 211680 in 10240-byte records; count, the set's length
in them. Booted from the disk alone:

	>> 4.4BSD/i386 BOOT
	Default: 0:wd(0,a)/vmunix
	...
	changing root device to wd0a
	Automatic boot in progress: starting file system checks.
	...
	login: root
	testhost# hostname; df
	testhost.my.domain
	Filesystem  1K-blocks     Used    Avail Capacity  Mounted on
	/dev/wd0a       77983    23034    51049    31%    /

Configure makes the databases first and then runs MAKEDEV inside the
new root, from the base set's /dev/MAKEDEV: MAKEDEV's `chown -R
bin.bin fd' looks `bin' up, and this libc finds a user only in a
pwd.db, which the install root does not have. After: fd/ is owned by
bin, kmem's group is kmem, and ttyp0 is there. The set measured was an
earlier staging of the root with the tree's current MAKEDEV in it.

## Noted, not changed

- **The wd driver knows one drive.** Its configuration makes one
  isa_device, and wdunit() reads the drive from bits 5 and 6 of the
  minor, where MAKEDEV numbers a second drive's partitions from 8 --
  partition 8 of drive 0 to the driver, its raw partition. So
  MAKEDEV's wd1 is wd0, and the install root carries no wd1.
- **Configure's ethernet questions** append an ifconfig line to
  netstart, whose own ifconfig lines for ace0, ex0 and the rest are
  Berkeley's template and fail on every boot, as they did before.
  Run twice, Configure appends the line twice.
- **Load_fd and Load_tape**, NetBSD's set loaders, cannot work: this
  install kernel has no floppy, msdos or tape driver.
- **The installer's files stay on the new root** -- /install,
  /.instutils, /.commonutils, /mnt2, /usr/mdec and
  /etc/disktab.preinstall -- as on a NetBSD 1.1 install, for running
  its functions from the installed system. Its /.profile does not:
  the base set's replaces it.
