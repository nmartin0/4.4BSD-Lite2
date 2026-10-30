# Roadmap: 4.6BSD

4.6BSD is cut from 4.5BSD once 4.5BSD is complete (`docs/roadmap.md`),
and continues it. Its aims, as the maintainer set them (2026-10-22):
new kernel primitives, more drivers, modern hardware, modern tools,
POSIX 2024, a modern C standard -- a modern, up-to-date, fully capable
system that is none of OpenBSD, NetBSD or FreeBSD, but a development
continuation of 4.5BSD, loyal to its 4.4BSD-Lite2 heritage, minimal,
and close to its roots.

**Decided 2026-10-29: `docs/design-4.6.md` sets 4.6BSD's goal (POSIX.1-2024,
C17, the System V ABI and their errata, tested with the maintainer's
suite), its rules, the driver interface (OpenBSD's 1996-2014 plateau)
and the compiler-flag work. 4.5BSD is cut after a short floor of audit
fixes rather than completed; where this file and design-4.6.md
disagree, design-4.6.md is the later decision.**

**Recorded 2026-10-30: `guide-4.6.md` is the design guide that goes with
this roadmap -- the base (4.4BSD-Lite2, not a daughter), the decisions,
the sources by system, the route to each feature modern systems share,
the research, and what of 4.4BSD's tradition stays. Its section 8
gathers the open decisions, including those added since the list
below.**

**Recorded 2026-10-29 as the plan for later: *The continuation roadmap*
below -- each subsystem's backports in order, their plateaus, and the
decisions still open -- drawn from `analysis-4.6-continuations.md` and
`analysis-md-minimality.md`.**

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

## Backporting: which donor, how far, and how much

4.6BSD's changes are backports, not new engineering, and the aim is to
change what exists and leave its interfaces alone. Measured
2026-10-28 against the donor trees. The method: each of this tree's
363 `.c` and `.h` files in `kern`, `vm`, `net`, `netinet`, `sys`,
`ufs`, `miscfs`, `nfs` and `isofs` was diffed (`diff -w`) against the
same file, or the one file of that name, in each donor; the figure is
the share of this tree's lines that survive. Machine-dependent code and
drivers are not in it. It measures text, not compatibility, so for the
two interfaces that matter most the operation names were counted too.

| Donor | source lines kept | header lines kept |
|---|---|---|
| OpenBSD 1997-06 | 81% | 85% |
| OpenBSD 1999-12 | 80% | 84% |
| NetBSD 1.3 | 78% | 83% |
| FreeBSD 2.0.5 | 76% | 81% |
| FreeBSD 2.2.8 | 75% | 81% |
| FreeBSD 3.0 | 70% | 78% |
| NetBSD 1.4 | 67% | 80% |
| OpenBSD 2002-01 | 66% | 72% |
| FreeBSD 4.0 | 65% | 72% |
| NetBSD 1.5 | 62% | 76% |
| FreeBSD 5.0 | 38% | 56% |

**NetBSD 1.3 and OpenBSD 1997-99 are the continuation; take from them
first.** The break is UVM, in NetBSD 1.4 and in OpenBSD by 2002
(`sys/vm` gone), and, in FreeBSD, 3.0 for the VM and VFS and 5.0 for
the rest.

- **VFS.** This tree has 42 vnode operations. OpenBSD 1997 has 41
  (no `revoke`), OpenBSD 1999 all 42 and `balloc`, NetBSD 1.3 40
  (`poll` for `select`), FreeBSD 2.0.5 39. FreeBSD 3.0 moves seven
  UFS operations out of the generic set (`blkatoff`, `seek`, `select`,
  `truncate`, `update`, `valloc`, `vfree`) and adds seven: the
  interface changes there.
- **pmap.** 26 entry points in `vm/pmap.h`. NetBSD 1.3 and OpenBSD
  1997 keep all 26; OpenBSD 1999 loses one; NetBSD 1.4 and 1.5 lose
  four to six (UVM); FreeBSD 2.2.8 loses five and adds fifteen. The
  pmap boundary is the seam under any VM.
- **VM.** FreeBSD's `sys/vm` keeps 39-66% of this tree's source lines;
  NetBSD 1.3 and OpenBSD 1997-99 keep 88-92%, being Lite2's VM with
  fixes. To keep the Lite2 VM interface, take from them.
- **Shrink-wrap set.** 149 of the 363 files (41,848 lines) keep 90%
  or more in all three of NetBSD 1.3, OpenBSD 1997 and OpenBSD 1999;
  215 keep it in at least one; 14 are gone from all three. By
  directory, 90% or more in all three: `sys` 54 of 92, `vm` 23 of 32,
  `net` 14 of 27, `netinet` 14 of 39, `miscfs` 12 of 39, `ufs` 11 of
  44, `kern` 9 of 57, `nfs` 7 of 21, `isofs` 5 of 12. The list, with
  every figure, is `docs/provenance/shrinkwrap-4.6.tsv`. A file in it is
  one whose donors' version is a few changes from this one, so the
  changes can be taken as bug fixes and not as an interface move.

**Additions that have no counterpart here, sized at the nearest
donor** (lines of `.c`, `.h`, `.s`):

| Item | Donor | Lines |
|---|---|---|
| kernel threads | FreeBSD 4.0 `kern_kthread.c` | 144 |
| zone allocator | FreeBSD 3.0 `vm_zone`; Mach 3.0 `zalloc`; NetBSD 1.4 `pool` | 599; 1,409; 1,543 |
| loadable modules | FreeBSD 2.2.8 `kern_lkm.c`, `lkm.h` | 1,329 |
| SMP, i386 | FreeBSD 3.0 `mp_machdep.c`, `mpapic.c`, `mpboot.s`, `simplelock.s`, APIC headers and vectors | 6,236 |
| autoconf, `device.h` | NetBSD 1.3 | 803 |
| PCI core, `pcidevs` | NetBSD 1.3 `dev/pci`, `arch/i386/pci` (core files) | 1,927 (all of `dev/pci`: 32,658) |
| `bus_space`, `bus_dma` | NetBSD 1.3 i386 | 1,016 |
| `config(8)` | NetBSD 1.3 `usr.sbin/config` | 6,039 |
| ATA, ATAPI, PCI IDE | NetBSD 1.4 | 8,189 |
| USB | NetBSD 1.4 / OpenBSD 1999-12 | 16,715 / 18,812 |
| IPv6 (KAME) | NetBSD 1.5 `netinet6` | 37,532 |
| amd64 | FreeBSD 5.3 `sys/amd64` | 39,920 |
| ACPI | FreeBSD 5.3 `dev/acpica`, `contrib/dev/acpica` | 138,901 |
| AHCI | FreeBSD 8.0 `dev/ahci` | 2,408 |
| NVMe | FreeBSD 10.0 `dev/nvme` | 5,866 |
| virtio | FreeBSD 10.0 `dev/virtio` | 12,115 |
| UEFI loader | FreeBSD 10.0 `boot/efi`; 12.0 `stand/efi` | 7,067; 18,274 |

For scale, this tree's `sys/kern` is 32.7k lines, `sys/vm` 13.8k,
`sys/netinet` 13.4k, `sys/i386` 36.8k and all of `sys` 579k. UVM
(NetBSD 1.4, 23,170 lines) and FreeBSD 3.0's VM (18,011) are larger than
this tree's 13.8k.

**SMP from FreeBSD, and what it asks of the VM.** FreeBSD 3.0's SMP is
a big kernel lock (`mp_lock`, taken on kernel entry and given up in
sleep), not a re-entrant VM: `sys/vm` has 16 lines that mention SMP or
a simple lock, in four files, and most of those are `vm_zone`.
`struct lock` and `simple_lock` are already in Lite2. The work is in
the machine-dependent code, by grep: `sys/i386/i386` 683 lines in 20
files (`mp_machdep.c` 314, `mpapic.c` 96, `pmap.c` 89, `globals.s`
76, `mplock.s` 48, `swtch.s` 34), `i386/include` 175, `i386/isa`
267 (`apic_vector.s` 67, `apic_ipl.s` 47), `kern` 110 in 17 files.
pmap's part is a private page table per CPU (`prv_CMAP`, `SMP_prvpt`)
and TLB shootdown by inter-processor interrupt. The difficulty is not
the VM but that those hooks are written against FreeBSD's i386 base,
and this tree's i386 files share little with it: against FreeBSD
2.2.8, 3.0 and 4.0 this tree's `pmap.c` keeps 36%, `trap.c` 27-28%,
`machdep.c` 33-35%, `locore.s` 9%, `isa/clock.c` 27-28%; against NetBSD
1.3 and OpenBSD 1997, `pmap.c` 49-51%, `trap.c` 34%, `locore.s`
22-23%. FreeBSD 3.0's interrupt and `spl` handling for SMP (`cpl`
under a spin lock, `ipl.s`, `ipl_funcs.c`) is the larger piece and
touches every driver's interrupt path. NetBSD 1.3 to 1.5 and OpenBSD
1997 to January 2002 have no APIC or MP file under `arch/i386`
(checked 2026-10-28), so FreeBSD is the only donor in the window.

## The continuation roadmap (recorded 2026-10-29)

The maintainer's direction: 4.6BSD is 4.4BSD-Lite2 modernized by
backporting the continuations of 4.4's own subsystems as the daughters
carried them on -- additions and fixes to the 4.4 design, not
redesigns -- searched in the order OpenBSD, NetBSD, FreeBSD, then MirOS
BSD, then DragonFly BSD, then other permissive BSDs. The evidence is in
`analysis-4.6-continuations.md` (each subsystem traced to its plateau,
and how they fit), `analysis-md-minimality.md` (machine-dependent code
per port) and `analysis-4.6-lineage.md` (the first pass, corrected by
the second). This section is the plan drawn from them, for later. Where
it and an older line above disagree, this section is the later.

### Decisions still open

1. **The VM.** The maintainer wants no translation layer and as few
   shims as possible (2026-10-29). So: (a) Lite2's Mach VM, continued to
   its plateau (NetBSD 1.3, OpenBSD to 2001-05, FreeBSD 2.0-2.1, and
   Mach 3.0's symmetric copy-on-write and collapse work), with the amd64
   and MP pmaps and PAE written for Lite2's pmap interface by us, and
   each later machine-dependent file's VM calls written in Lite2's form
   as it is taken; or (b) the Mach VM first, then UVM at the point
   OpenBSD switched (both builds kept until the switch, as OpenBSD did
   1999-12 to 2001-05). FreeBSD's VM after 1995-07-13 is not a
   candidate. The costs of (a) are in `analysis-4.6-continuations.md`
   section 11. Nothing below that touches the pmap, amd64, SMP or PAE
   starts before this is decided. OSF MK 7.3 (the maintainer's
   repository) is still to be read for it.
2. **Four FreeBSD VM files with Net/2 versions** (`vm_page.c`,
   `vm_pageout.c`, `vnode_pager.c`, i386 `pmap.c`, inherited through
   FreeBSD 1.x): tainted under the 386BSD rule or not. Until ruled,
   FreeBSD VM changes are taken only where they touch Lite-derived files.
3. ~~**x86**~~ **Decided 2026-10-30: i386 and amd64 are separate ports,**
   as in OpenBSD (one lineage; OpenBSD's own ACPI), not NetBSD 2.0-4.0's
   shared `x86`. `guide-4.6.md` section 2; `design-4.6.md` rule 5.
4. **The ports:** whether every Lite2 port (hp300, i386, luna68k,
   news3400, pmax, sparc, tahoe, vax) is kept up in source with each
   machine-independent change, or only the ports that can be built and
   boot-tested count as supported. tahoe has no daughter port at all.
5. **The order among the first three donors**, recorded as the
   maintainer wrote it: OpenBSD, NetBSD, FreeBSD.
6. **MirOS BSD** could not be read from the workspace; a copy of its
   `src` would let it be checked.
7. **Corrections from the first pass awaiting a decision**
   (`analysis-4.6-lineage.md` section 9): the network driver plateau
   ends 2014-06, not 2015; FreeBSD 3.0's SMP withdrawn as the SMP donor;
   POSIX.1-2024 itself requires a 64-bit `time_t` (Austin Group bug
   1462), on the i386 too; the slice layer and FreeBSD 2.1.0's
   `wd`/`atapi`/`wcd` leave with the driver Tier A.
8. Older open questions (`design-4.6.md`): ddb or kgdb, Kerberos, the
   licence for our own files, GCC or Clang.

### The backports, by subsystem, in order

Each item is the oldest donor that continues 4.4 for it. "Plateau" is
the furthest that line may be followed.

**Processes and threads** (plateau: OpenBSD 2017)

1. The kernel stack off its fixed address at `USRSTACK`, as NetBSD 1.1
   (1995); also gives ddb `trace/t`.
2. OpenBSD rthreads, 2005-12-03: threads as processes sharing a
   `vmspace`, `rfork(RFTHREAD)`, `thrsleep`.
3. OpenBSD's increments: the `struct process` split (2007-2010),
   `__thrsigdivert` (2009), the TCB syscalls and per-thread segment
   registers (2011), futex (2017).

**Time, signals, POSIX realtime** (plateau: NetBSD 4.0; FreeBSD 4.11 for
AIO)

1. `clock_gettime`, `nanosleep` (OpenBSD 1997), `CLOCK_MONOTONIC` (OpenBSD
   2003), timecounters (OpenBSD 2004); `struct timespec`'s `ts_sec` to
   `tv_sec`.
2. `sigset_t` widened to 128 bits (NetBSD 1.4).
3. `siginfo` (OpenBSD 1997); the queued `ksiginfo` and `sigtimedwait`
   (NetBSD 2.0); `sigqueue` and `SIGRTMIN`-`SIGRTMAX` (NetBSD 6, a few
   dozen lines, adapted to spl).
4. POSIX timers (NetBSD 2.0: `splclock`, callouts, per-process
   `p_timers`; Lite2's interval timers become slots 0-2); `SIGEV_THREAD`
   in libc (FreeBSD 7's `librt` the only BSD precedent).
5. `clock_nanosleep` (NetBSD 7 as reference; a few dozen lines over
   `tsleep`); `pthread_condattr_setclock` (OpenBSD 2012).
6. AIO: FreeBSD 3.0's `vfs_aio.c` (Dyson 1997, `splbio`, kernel
   processes); `aio_fsync` after FreeBSD 7.
7. Semaphores, barriers, spin locks, rwlocks: OpenBSD's userland (2011-12);
   option values to 202405L.
8. `pselect`/`ppoll` (OpenBSD 2013), `getentropy` (OpenBSD 2014), `waitid`
   (OpenBSD 2022), `posix_spawn` (OpenBSD 2012). SysV message queues and
   semaphores: OpenBSD's rewrites (2002, 2009); Boulet's 1993-94 code in
   NetBSD and FreeBSD is checked against the 386BSD rule before any use.

**Locking, SMP, scheduler** (plateau: OpenBSD 2014)

1. OpenBSD's `timeout(9)` (2000), the level-based `cpl` (2001-12),
   `splassert` (2002), rwlock (2003-11).
2. SMP after decision 1: OpenBSD 2004-06 (i386 and amd64; one recursive
   kernel lock, `simple_lock` left a no-op), `mtx` at IPL (2004-07),
   `sched_lock` as a mutex (2005).
3. The machine-independent `cpu_switchto` (2007), per-CPU run queues
   (2009), lockmgr on rwlocks with its interface kept (2013).
4. Not taken: witness, SMR, the MP network stack (other designs).

**VM** (plateau: OpenBSD 2001-05, NetBSD 1.3, FreeBSD 1995-07-12; then
Mach 3.0's own; `analysis-4.6-continuations.md` section 12)

1. NetBSD 1.3's and OpenBSD's fixes to Lite2's VM: collapse and bypass,
   Cranor's `vm_physseg` (1998), `vm_page_alloc_memory` for bus_dma,
   `swapctl` (NetBSD 1.3), `paddr_t` (OpenBSD's Mach VM).
2. FreeBSD 2.0-2.1's changes on the Lite design (fault-ahead and
   clustered page-in, collapse and dead-object fixes, hold/busy,
   valid/dirty bits, `vm_page_alloc_contig`, `pmap_growkernel`, page
   aging), Lite-derived files first, the rest subject to decision 2.
3. Mach 3.0's and OSF MK's pieces (OSF MK as MkLinux DR3 carried it,
   github.com/slp/osfmk-mklinux), adapted to Lite2's pagers: symmetric
   and delayed copy-on-write, protections in `vm_map_enter`, fault and
   pageout clustering, zones and map entries from zones, the contiguous
   allocator, throttling, real VM locks.
3a. UVM's pieces that do not depend on amaps and anons, where the Mach
   lines have none: the VM object embedded in the vnode (ends double
   caching and the per-file pager structures), loans of object pages to
   the kernel, the wiring technique, and the pmap additions
   (`pmap_kenter_pa`, `pmap_kremove`, `pmap_growkernel`,
   `pmap_update`). Section 13.
4. Then decision 1: the amd64 pmap, TLB shootdown and PAE in the pmap
   (our own, for the Mach VM), or UVM.
5. Remaining by design if the Mach VM is kept, after 3 and 3a: the
   shadow-chain model (its swap leaks reduced, not ended) and loans into
   anonymous memory.

**Files** (plateau: OpenBSD 2014, selectively later)

1. `O_NOFOLLOW` (OpenBSD 2000), dirhash (2003).
2. FFS2 (OpenBSD 2004-2006) with 64-bit block numbers (`daddr64_t`
   2006-10, `daddr_t` 2011-03, cleanup 2013-06).
3. `st_atim` (2009-01); the name cache on red-black trees (2009).
4. The `*at()` calls, `utimensat`/`futimens`, `O_CLOEXEC`, `O_DIRECTORY`
   (OpenBSD 2011-07).
5. `O_SEARCH`, `O_EXEC` (NetBSD or FreeBSD 8; not in OpenBSD); `O_CLOFORK`
   (OpenBSD 2025, checked against POSIX's exec semantics; NetBSD trunk's
   follows POSIX exactly).
6. Vnode locks on rwlocks (OpenBSD 2016). The 2010 `struct vops` table is
   a translation boundary to cross once.

**Sockets, network, terminals** (plateau: OpenBSD 2014-06; tty 2026)

1. `poll` (OpenBSD 1996), `socklen_t`, `sockaddr_storage` (1999), mbufs on
   pool (2001; on the Mach VM, pool's page back end, about 40 lines),
   `MSG_NOSIGNAL` (2011), `accept4`/`SOCK_CLOEXEC`/`pipe2`/`dup3`
   (2014-08-31), `sockatmark` (NetBSD 2001).
2. Removal of ISO, XNS and X.25 (OpenBSD 2004-06 precedent).
3. KAME IPv6 (OpenBSD 1999-12), later and separately; `getaddrinfo` in
   libc first.
4. tty: keep Lite2's `tty.c` on FreeBSD 2.0's cblocks; take OpenBSD's
   ptm (2004) and `posix_openpt` (2012) through a `firstc`/`clalloc`
   shim; `tcgetwinsize` (NetBSD 2017).

**exec and the ABI** (plateau: OpenBSD 2017)

1. The exec framework (NetBSD 1.0-1.1; the encumbrance fix in progress).
2. ELF with the auxiliary vector and `PT_INTERP` (NetBSD 1.1).
3. OpenBSD's ld.so (2000); TLS (2011 registers and syscalls; 2016 ld.so).
4. The 64-bit ABI in one flag day, OpenBSD's of 2013-08-13 (`time_t`,
   `ino_t`, `getdents`, `st_atim`), before 4.6BSD binaries exist.

**Machine layer, drivers, boot** (plateau: OpenBSD 2014; UEFI 2015)

1. Torek autoconf with `cfattach` on the i386 (NetBSD 1.0-1.3 / OpenBSD
   1997), `bus_space`/`bus_dma`, PCI; the driver interface at OpenBSD's
   1996-2014 plateau.
2. Storage: OpenBSD wdc/pciide (1999), the SCSI midlayer, AHCI (2006),
   NVMe (2014), GPT (2014); the FreeBSD slice layer and `wd`/`atapi`/
   `wcd` out. wscons; USB.
3. After decisions 1 and 3: amd64 (OpenBSD 2004, NetBSD's code of 2003;
   or NetBSD 2.0-4.0 with shared `x86`), APIC and MP, ACPI.
4. Boot: OpenBSD's `bootarg` (1997) extending Lite2's arguments; efiboot
   and the framebuffer console (2015).

### Machine-dependent code: the measure for every port

From `analysis-md-minimality.md`: for every Lite2 port the smallest
machine-dependent core of any release is Lite2's own (i386 10,076 lines,
1,704 of them assembly); for amd64 it is OpenBSD's 2004 port (28,741).
The daughters grew mostly in C, not assembly, and sharing between ports
did not by itself shrink them. So, for each port:

- its core is measured against Lite2's figure, and each patch states
  its growth (this tree's i386 core is 15,253 lines today);
- only what touches the processor or its privileged state is
  machine-dependent; drivers are in `sys/dev` behind `bus_space` and
  `bus_dma`;
- assembly only where C cannot do it (trap and interrupt entry, context
  switch, the first instructions after boot, the syscall stub,
  `setjmp`/`longjmp`); C17 `<stdatomic.h>` and compiler builtins
  instead of per-port atomic and lock assembly; libc's string functions
  in portable C unless a measurement shows a port needs assembly;
- code is shared between ports only where it is truly common.

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
