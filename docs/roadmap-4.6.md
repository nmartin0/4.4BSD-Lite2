# Roadmap: 4.6BSD

4.6BSD is cut from 4.5BSD once 4.5BSD is complete (`docs/roadmap.md`),
and continues it. Its aims, as the maintainer set them (2026-10-22):
new kernel primitives, more drivers, modern hardware, modern tools,
POSIX 2024, a modern C standard -- a modern, up-to-date, fully capable
system that is none of OpenBSD, NetBSD or FreeBSD, but a development
continuation of 4.5BSD, loyal to its 4.4BSD-Lite2 heritage, minimal,
and close to its roots.

Nothing here is begun before 4.5BSD is released. What is recorded
here was found while working on 4.5BSD and is kept so that it is not
rediscovered. Items still to be placed are listed in roadmap.md, *To
settle*.

## The compiler and C

- **Newer compilers.** GCC 4.2.1 and binutils 2.15, FreeBSD 7.0 and
  8.0's pair, were planned beside GCC 2.95.3 for newer C (roadmap.md,
  *After Tier 1½*); a modern C standard needs later ones still.
- **`-fno-common`**, before any compiler newer than GCC 2.95.3 builds
  the kernel: 101 symbols tentatively defined in more than one kernel
  object (roadmap.md, Tier 1, item 5; deferred.md).
- **Changes made for GCC 14** are kept in the tree for these
  compilers (deferred.md, section 5).
- **Permissive binary tools**, when they mature: nm and size from
  OpenBSD, ar and ranlib from FreeBSD 8, strip and objcopy from
  elftoolchain; a permissive assembler and linker (yasm, elftoolchain
  ld), measured and not yet able (roadmap.md, *After Tier 1½*).

## POSIX 2024

Not yet surveyed. Found so far, each left as Lite2 has it for 4.5BSD:

- sh: a redirection error on a builtin that is not special ends a
  shell that is not interactive, and so does one on a command with no
  command word. NetBSD 2.0 (2002) and FreeBSD 7.0 (2006) fixed the
  first; FreeBSD 9.0 (2010) and NetBSD after 8 the second (deferred.md).
- sort: a leading -n, -r or the like applies to no key given by
  position (deferred.md).

## Kernel primitives

Stage 5 of Torek's move (roadmap.md, *Berkeley's driver framework on
the i386*):

5. Locking, OpenBSD's way, with the interrupt levels the converted
   drivers name: KERNEL_LOCK, nothing on one processor; mutex(9) as
   spl and an owner check, `mtx_init(&m, IPL_NET)'; rwlock(9) and
   msleep; lockmgr on rwlocks, its LK_* interface kept. spl, tsleep
   and the drivers stay as they are.

**And Mach, looked at (2026-10-18).** Lite2's locks are Mach's
already: kern_lock.c's lockmgr is McKusick's 1995 rework of the Mach
lock package (its notice credits Avadis Tevanian, Michael Wayne Young
and the Mach project at CMU), simple_lock and the vm/ system are
Mach's, and vm.h maps simple_lock_data_t and lock_data_t onto them.
CMU Mach 3.0 (MK83A), Utah's Mach 4 and OSF's kernel as MkLinux DR3
carried it were read for anything better. None fits better than
OpenBSD's line: MK83's and Mach 4's lock_t is what lockmgr replaced,
without its wait message, priority, timeout or signals; OSFMK's mutex
sleeps where OpenBSD's mtx spins at an spl, its lock_t lost the
recursion vnode locking needs, and everything in it rests on Mach
threads, assert_wait and thread_block's continuations, which Lite2's
processes and u-area cannot carry. OSF's code is permissive: each
of its source files carries OSF's 1998 notice ("Permission to use,
copy, modify, and distribute this software ... for any purpose and
without fee is hereby granted"); the academic-only text in its
conf/copyright.osf is a 1995 OSF/RI notice no source file carries and
the build never reads. OSFMK 6.1 itself was not found to read. Worth
having, as design or as code: OSFMK's lock
debugging -- a per-processor stack of held locks, lock-held and
none-held assertions, a spin that times out into the debugger --
alongside OpenBSD's MUTEX_ASSERT_LOCKED and witness(4); and ddb,
which is CMU Mach's code, taken from NetBSD or OpenBSD, where it is
already a BSD's. Lites, CMU's 4.4BSD-Lite server on Mach, folded spl,
tsleep and one master lock together, the precedent for KERNEL_LOCK.
FreeBSD's zones (vm_zone.c, John Dyson, 1997-98) borrow only Mach's
names; OpenBSD's pool(9) is the BSD equivalent.

**The BSDs' locking, surveyed (2026-10-22)**, for stage 5 and for
taking drivers; read from the trees. Lite2's own: spl levels,
tsleep/wakeup, and Mach's simple_lock and McKusick's lockmgr; Torek's
cfdriver, each softc beginning with struct device.

- OpenBSD grew those in place. spl and tsleep stayed; rwlock(9),
  November 2003 (Artur Grabowski); mutex(9), July 2004, "cpu-owned
  spinning locks with spl semantics", mtx_init(&m, IPL_NET); the
  kernel lock with SMP, June 2004; simple_lock folded away in 2010;
  lockmgr put on recursive rwlocks in 2013 (Ted Unangst, after
  Thordur Bjornsson) and its interface removed in 2016; splassert
  (2002), rwsleep and NET_LOCK (2016), nanosecond sleeps (2019). Its
  em(4) of 2026 still mixes splnet(), KERNEL_LOCK(), mtx at an IPL
  and an IPL_NET | IPL_MPSAFE interrupt.
- NetBSD replaced them at once. simple_lock and lockmgr through 4.0
  with a big lock; in 5.0 (2009) both removed for Solaris's set --
  adaptive mutexes, mutex_init(&m, MUTEX_DEFAULT, IPL_x) ("For porting
  code written for Solaris"), rwlock, condvar, sleep queues,
  workqueues -- spl kept as IPLs.
- FreeBSD took another system's. 5.0 (2003): BSDi's SMPng from BSD/OS
  (its mutex.h copyright 1997 Berkeley Software Design) -- every spl a
  no-op returning 0, interrupts in kernel threads, Giant, mtx, sx,
  condvar; rwlock in 7.0 (2008).

The canonical primitives for this system are OpenBSD's, the one line
that kept Lite2's model and grew it -- as stage 5 above already
decided. And for drivers, the interface counts for more than the
locks. FreeBSD left Torek's framework in 3.0 (1998) for newbus (Doug
Rabson, 1997-98), so its drivers' attach, interrupt and locking code is
rewritten whatever this system chooses. NetBSD and OpenBSD kept
Torek's, with NetBSD 1.2's cfattach, which OpenBSD's struct cfattach
still is: size, match, attach, detach, activate. In order of ease:
OpenBSD's drivers of any year -- the same softc, cfattach, bus_space,
bus_dma and, with its primitives, the same locks; NetBSD's to 4.0, the
mid-2000s and before, Torek-shaped (struct device first, CFATTACH_DECL)
and locked mostly by spl alone; NetBSD's from 5.0, whose softc holds a
device_t (CFATTACH_DECL_NEW, device_private()) and whose locks are
Solaris's, adaptable by a thin layer and by mapping mutex_enter to
mtx_enter or rw_enter and cv_wait to msleep or rwsleep; then
FreeBSD's, whose hardware knowledge carries over and little else.

## Modern hardware

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

**No modern NIC.** `if_ed`, FreeBSD 2.1.0's (patch 166), drives the
NE1000 and NE2000, the WD8003 and 8013 and the 3c503; Lite2's `if_ne`,
`if_we`, `if_ec` and `if_apx` are of the same era.

**32-bit, no PAE.** The pmap assumes a 32-bit physical address space.
Modern machines have more than 4 GB.

**BIOS boot only.** The bootstrap, FreeBSD 4.1's boot1 and boot2,
reads ELF but starts only from a BIOS; UEFI-only machines cannot boot
this at all.

**Uniprocessor.** No kernel threading; `struct proc` is the unit of
scheduling and there is no thread abstraction to carry forward.

### The question this cannot start without

"Modern hardware" spans two targets that differ enormously in scope:

- **roughly 2005–2015**: legacy BIOS, PCI, IDE-compatible SATA,
  USB-legacy or PS/2 input. Large work, but every piece has a lineal
  BSD implementation that could be traced.
- **2020 onward**: UEFI-only, NVMe, no CSM, USB-only input. Almost
  none of that has 4.4BSD ancestry to be genetic about, because it
  postdates the lineage.

Which one 4.6BSD targets decides whether this is six items or
twenty, and it should be settled before any of it begins.

## Modern tools

To be listed. Shared libraries are to be settled (roadmap.md, *To
settle*, and *Later: shared libraries*).
