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
| program directories that build | 302 | 320 |
| libraries built by `sysroot.sh` | 11 | 13 |
| kernel sources compiled | 138 | 853 |
| kernel configurations that build | 1 | 2 (i386) |
| ports whose kernel has ever been compiled | 1 | 8 |
| kernels that link | **1** | — |
| kernels that run | **1** | — |

So the userland restoration is substantially done and the kernel one
is not. What remains:

**~~The kernel link.~~ Done.** The four symbols — `chrtoblk`, `fuswintr`,
`suswintr`, `wddriver`, plus `memset` on a gcc 14 host. Models for
each in `docs/status.md`. `libkern.a` is built by nothing.

**`GENERIC.i386` does not build.** It names a `cn0` console no driver
provides and selects no filesystem or pager. `LINK.i386` carries the
corrections; Berkeley's own ARGO and BLITZ configs have the corrected
console line, so fixing GENERIC would be kind A — Berkeley's code
corrected to Berkeley's own intent.

**The DIAGNOSTIC build does not compile.** `6a81e5fe`: `debug1` has an
incomplete type in `vfs_subr.c`. A whole build configuration is
broken and nothing tracks it.

**`wd` is in `LINK.i386` and carries the root; `fd` and `wt` are
not.** `fd.c` still has the `b_actf`/`b_actb` sites recorded here
before, and `wt.c` fails on a static declaration following a
non-static one. Neither is needed to boot from disk.

**Seven ports have never been compiled at all** — hp300 (74 sources),
luna68k (59), sparc (55), pmax (48), news3400 (57), vax (61), tahoe
(161). Whether 4.5BSD keeps them is an open question, but "4.4BSD-Lite2
compiles under modern GCC" is not true of the tree until they do or
are removed deliberately.

**`libresolv` and `liby` are not built.** Every other library in
`lib/` is.

**18 program directories still fail**, each for a reason already
recorded: `usr.bin/pascal` (`libcpats.c` absent from Lite and Lite2),
`usr.bin/vacation` (`${LIBDBM}`, a library that does not exist — its
dbm is `ndbm.o` inside libc), `usr.bin/vmstat.sparc` and
`usr.sbin/eeprom` (sparc-only), `libexec/kpasswdd` (is Kerberos), and
13 games — three wanting `-fwritable-strings`, two calling `exit` with
no argument, two passing the wrong type to `log`, two building a
generator and running it, two wanting X11 headers, one declaring a
static function inside another, one wanting `gtty` from libcompat's
absent 4.1 sources.

**A build-ordering defect, found while measuring this and recorded in
no commit.** Eight directories fail a first `make.sh NOMAN=noman
depend all` and succeed on a second, with no change between the runs:
`sbin/disklabel`, `sbin/restore`, `sbin/route`, `usr.bin/lorder`,
`usr.bin/pagesize`, `usr.bin/shar`, `usr.sbin/arp`, `games/phantasia`.
Cause not yet established. Until it is, any sweep of the tree
undercounts, and the counts in this file were taken from a second
pass for that reason.

**`YACC` still names the packaged byacc** although `sysroot.sh` has
built this tree's own into `tools/bin` since `dc51dc50` and `make.sh`
puts that directory first on `PATH`. One line. It would let
`libexec/ftpd/ftpcmd.y` stand as Berkeley wrote it, since its `static`
on `yylex` was dropped only to satisfy byacc.

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
(its own note says why), `hp/dev/hil_subr.c`, which this build does
not compile, and:

| file | what | what it costs |
|---|---|---|
| `kern/sys_process.c` | `ptrace` | debuggers; returns ENOSYS |

**What is left before Tier 1 is solid**, in the order proposed:

1. ~~Owners and groups as the tree names them~~ -- `build/install.sh`.
2. ~~`/usr/share/misc/termcap`~~ -- built on the host, indexed on the
   system.
3. ~~**Swap.**~~ `subr_rmap.c` from NetBSD 1.2, a b partition from
   `build/mklabel.c`, `swapon -a` from fstab; measured paging under
   `-m 16`.
4. ~~**MAKEDEV's pty majors**~~, now 5 and 6 as conf.c has them, and
   ptys configured in LINK.i386; `script` runs through ttyp0. Still
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
5. **`-fno-common`.** The `mmap` fault (install.md, fault 10) was a
   common symbol resolved to a function. GCC has defaulted to
   `-fno-common` since 10; this tree passes `-fcommon` because 1990s
   code declares the same global in several files. Measured: 332
   common symbols in the kernel's objects, 101 of them tentatively
   defined in more than one -- `nbuf`, `bufpages` and `nswbuf` in some
   sixty each, from headers that say `int nbuf;` without `extern`.
   Moving to `-fno-common` means an `extern` in each of those headers
   and one definition apiece; not done. What is done:
   `build/kernel.sh` now refuses a kernel in which any common symbol
   is a text symbol, the exact class of the `mmap` fault, and was
   seen to refuse one with that fault put back.
6. **`ptrace`**, for gdb. Lower: nothing in the boot path needs it.
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
   `build/shim` stays for the install recipe, until 2 and 3 replace
   it.
2. **An install kernel with its root inside it** -- the miniroot,
   FreeBSD 2.0.5's MFS-in-kernel boot floppy -- holding sh, the disk
   tools and pax. The mechanism is in: `options MFS_ROOT`, Berkeley's
   mfs_initminiroot, FreeBSD 2.0.5's write_mfs_in_kernel, an image
   from the host's makefs, booted from a 2.88 MB floppy image
   (install.md, *A root inside the kernel*). crunchgen is in, FreeBSD
   3.0's (install.md, *One binary*). The install root is in:
   NetBSD 1.1's i386 install floppy, 42 programs in one binary, in an
   install kernel that boots from a 2.88 MB floppy image to NetBSD's
   installer (install.md, *The install root*).
3. **A scripted installer**: label, newfs, mount, extract the sets,
   pwd_mkdb, cap_mkdb, MAKEDEV, fstab. It is the single-user sequence
   install.md runs by hand. NetBSD 1.1's i386 install.sh, which the
   install root already carries, adapted to this tree: its disktab
   and disklabel -B, pax for GNU tar, and loading the sets.
4. **The image**: El Torito, the boot image in floppy emulation as
   the BSDs did it first, the sets as ustar files on an ISO 9660
   filesystem, which Lite2's cd9660 reads.

## After Tier 1½ — self-hosting

The system building itself. Lite2 ships no C compiler, so this begins
with a decision: which compiler, from where, and whether the first
stage is a cross-build from the host.

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
driver from 1991. No AHCI, no NVMe.

**No USB.** On many machines that means no keyboard.

**No modern NIC.** `if_ne` is an NE2000; `if_we`, `if_ec` and
`if_apx` are of the same era.

**32-bit, no PAE.** The pmap assumes a 32-bit physical address space.
Modern machines have more than 4 GB.

**BIOS boot only, and the bootstrap reads a.out.** UEFI-only machines
cannot boot this at all.

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
