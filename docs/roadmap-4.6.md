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
- **GCC's other languages** -- Objective C, Fortran 77, CHILL, Java,
  imported with GCC 2.95.3 and not built -- left for later work
  (decided 2026-10-22; roadmap.md, *Later: GCC's other languages*).
- **Changes made for GCC 14** are kept in the tree for these
  compilers (deferred.md, section 5).
- **The i386 stack alignment, to be checked against the System V ABI
  (2026-10-27).** 4.5BSD compiles the kernel `-mpreferred-stack-boundary=2`,
  a 4-byte stack, as FreeBSD 4.0 does, because GCC 2.95's 16-byte
  default padded every call and made ddb's trace read sixty arguments
  for `Debugger()` (imports.md, patch 203). The belief behind it is
  that the original System V i386 ABI, which the systems of the
  period follow, requires only 4-byte alignment, and that 16 bytes
  came later with SSE: Apple's i386 ABI first, then a revision of the
  System V i386 psABI. That is from memory and has not been checked
  against either document. Before 4.6BSD moves to a newer compiler or
  ABI: read the System V i386 psABI and its revisions for the exact
  alignment each version requires and when; follow the version 4.6BSD
  targets, in the kernel and in userland (the flag here touches only
  the kernel's own compilation, and userland keeps GCC's default);
  drop or change the flag if the ABI asks for 16; and look for kernel
  code that depends on either alignment (SSE or aligned vector types,
  assembly that assumes it, ddb's `db_numargs`, which reads the
  instruction after a call and is only right for an unpadded call).
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

**ddb: tracing another process or thread (2026-10-26).** ddb's
`trace/t <pid>` is for 4.6BSD, not 4.5BSD. In Lite2 every process runs
its kernel stack at the same address, the one just under the top of
user space, and the address means the stack of whichever process the
processor is running. A sleeping process's saved frame pointer is such
an address, so ddb, which runs on the current process's stack, would
read the current process's frames and print rubbish; its u-area has
another address (`p->p_addr`), but the frame pointers inside the
stack do not follow it. So the 4.5BSD ddb prints "can't trace thread"
and returns, as NetBSD 1.0, OpenBSD and FreeBSD 2.0 and 2.1 do for this
reason (they have the same fixed stack; they then went on to use an
unset frame pointer, which 4.5BSD does not). ddb.4 does not document
`/t`. The first to trace by process was NetBSD 1.3 (sommerfe, March
1997), which gave each process's kernel stack an address of its own;
FreeBSD 5.0 (2003) does it for threads from the saved `pcb_ebp`, and
CMU Mach 3.0 does it for threads from the saved state of the thread.
Giving each kernel thread a kernel stack at a unique address is what
threads need anyway, and once 4.6BSD has it `trace/t` is NetBSD 1.3's
block: `pfind`, `P_INMEM`, the pcb's saved frame pointer (Lite2
calls it `pcb_fp`), printed as "trace: pid N at <frame>", and then the
frame walk ddb already has.

**ddb: watchpoints (2026-10-26).** `watch`, `dwatch` and `show
watches` are in ddb and in ddb.4, and do nothing, as they did in every
BSD of the period: the pages are made read-only (db_set_watchpoints,
pmap_protect) and nothing looks at the fault. 4.5BSD leaves them so.
Tried, and why it does not work here, for whoever takes it up:

- *Mach 3.0's way*, which is what db_watch.c is written for: after
  vm_fault succeeds on a write fault, trap.c asks db_find_watchpoint
  and enters ddb with T_WATCHPOINT (FreeBSD 2.0 has the matching
  IS_WATCHPOINT_TRAP and a T_KDBTRAP case in kdb_trap; trap.c never
  makes the call). It rests on vm_fault making the page writable
  again, which is so for memory a vm_object holds. Lite2's kernel text,
  data and bss are not held by one: vm_fault on them gives a fresh
  zero-filled page, and the first store to a watched global
  (`hostname`) replaced the real page; `msgbufp` read back as 0,
  kprintf faulted on it again and again, and the kernel stack ran out
  (QEMU: a page fault repeated to a double fault). So the page scheme
  can watch only malloc'd or user memory, never a kernel global, which
  is most of what one wants to watch.
- *pmap_protect and pmap_remove read `curproc->p_vmspace` with no
  check*, and ddb reaches them from the idle loop, where `curproc` is
  NULL (a fault at address 0x20). FreeBSD 2.2.0 and 3.0.0 write the
  guard in other places of their i386 pmap.c, `!curproc || (pmap !=
  &curproc->p_vmspace->vm_pmap)`. pmap_protect also does not flush the
  TLB (its TBIS is commented out), so ddb needs a pmap_update() after
  it, as db_write_bytes has one.
- *The debug registers*, which would watch a global: DR0-DR3 and DR7
  set by `watch` (four at most), a trap after the store. NetBSD
  1.0-1.3 and OpenBSD have the other half only: locore.s's debug vector
  saves %dr6 in the trap frame's error slot and db_machdep.h tests
  `(type)==T_TRCTRAP && (code)&15`, but trap.c passes kdb_trap a code of
  0 and nothing sets DR7. Mach 3.0 has dr0() to dr3() in locore.s and a
  T_DEBUG case in kdb_trap that reads dr6, and nothing in ddb that calls
  them; FreeBSD 2.1 to 4.1 print "Watchpoints are not supported. The
  debug exception type is in %dr6 and not yet in the args to this
  macro". No BSD of 1994-97 wired it up. (QEMU is said to implement them
  in its TCG; not tried.)
- Left for 4.6BSD: hardware watchpoints on the debug registers, with the
  vector change, a DR7 setter and the code passed through trap(); then
  either remove the page-protect scheme or keep it for allocated memory,
  with the two pmap guards above. The experiment's diff is not kept; the
  hook is the `db_find_watchpoint` call above at the point where
  user_page_fault returns KERN_SUCCESS.

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

To be listed. Carried over from 4.5BSD so far:

- spell: OpenBSD's 2002 redesign, as NetBSD 2005 has it with a front
  end in sh; Berkeley's needs AT&T's word list (deferred.md).
- bc and dc: Otto Moerbeek's 2003 rewrites, with libcrypto's bn(3) for
  dc (deferred.md); GNU bc 1.03 stands in 4.5BSD.
- ncheck: OpenBSD's ncheck_ffs (deferred.md).
- ecvt, fcvt and gcvt: OpenBSD's of 2002, in libc on __dtoa (deferred.md).
 Shared libraries are to be settled (roadmap.md, *To
settle*, and *Later: shared libraries*).
