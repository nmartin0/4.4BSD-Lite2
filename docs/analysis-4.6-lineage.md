# How far the 4.4BSD lineage reaches: a roadblock analysis for 4.6BSD

Written 2026-10-29 at the maintainer's request: look at the kernel as it
stands and at every change 4.6BSD intends, and find where following the
code lineages stops working -- above all, where two lineages cannot be
reconciled -- so that 4.6BSD ends as one logical system and not a
patchwork. It also answers the maintainer's question: will 4.6BSD be a
distinct BSD that continues 4.4, or a patchy OpenBSD?

This is analysis, not a decision. Nothing in `design-4.6.md` or
`roadmap-4.6.md` is changed by it; the corrections it finds are listed
in section 9 for the maintainer to decide.

Method: five read-only surveys of the donor histories, one per area
(VM; VFS and file systems; processes, threads and time; machine-
dependent code, drivers and boot; networking, tty and the user ABI),
plus a measurement of how much of Lite2 each donor kept over time. The
claims the conclusions rest on were checked again against the sources
(section 10). Where a figure is a count of lines it measures text, not
behaviour. **UNVERIFIED** marks what was not checked.

Classes used throughout:

- **(a)** a clean lineage backport: a donor's change onto code that is
  still Lite2-shaped, taken as it was made;
- **(b)** reconcilable: the right lineage, but it needs adapting to this
  tree (a shim, a rename, a part written against a neighbouring
  subsystem the donor had and we do not);
- **(c)** irreconcilable: the feature exists only in a lineage built on
  a base this tree does not have, so taking it means taking that base,
  writing it ourselves, or mixing two lineages in one layer.

## 1. The short answer

- **Most of what 4.6BSD needs is (a) or (b).** POSIX.1-2024 in the
  kernel (threads, time, signals, the `*at()` calls, the 64-bit ABI,
  sockets, terminals), FFS2, the driver interface at the plateau, ACPI,
  storage, USB and the console all have a lineage that continues
  Lite2-shaped code. Nearly all of it is OpenBSD's, which is the
  daughter that kept Lite2's design (section 2).
- **One choice decides everything else: the VM.** Every amd64 port, every
  x86 SMP, and PAE in the NetBSD/OpenBSD lineage was written on UVM. The
  only amd64 written on a Mach-derived VM is FreeBSD's, and it sits on
  FreeBSD's redesigned VM, newbus and SMPng, a different lineage in
  every layer. **No donor has amd64 + Lite2's Mach VM + the plateau
  driver interface.** Staying on the Mach VM past about 2001 means
  writing that combination ourselves (section 4).
- **There are four other true (c) points**, each with a clean way out
  (section 7): FreeBSD 3.0's SMP must not be mixed with the NetBSD-
  derived x86 MP; OpenBSD never shared code between i386 and amd64;
  the four mandatory POSIX realtime options exist only in NetBSD's
  post-5.0 code or FreeBSD's SMPng; the FreeBSD-only parts already in
  the tree (slices, `wd`/`atapi`, BTX) have no forward path.
- **Verdict on the second question:** 4.6BSD will be a distinct BSD, but
  only if the rules are held. Its changing layers will be mostly
  OpenBSD's code of 1996-2014, because that is where 4.4's code lived
  on. What makes it more than a patchy OpenBSD is what it does *not*
  take (OpenBSD's own redesigns and agenda, which account for most of
  its drift from 4.4) and what it adds that no daughter has (complete
  POSIX.1-2024 on a 4.4 base). Section 8 sets out the risks.

## 2. Where the tree stands, and where the heritage went

**The tree today.** Against the Lite2 import (`b3d2855f`), with
whitespace ignored, the machine-independent kernel is still nearly all
Lite2:

| Directory | Lite2 lines | Now | Lines added / removed |
|---|---|---|---|
| vm | 13,765 | 13,765 | +1 / -1 |
| net | 9,602 | 9,602 | 0 |
| nfs | 17,956 | 17,956 | +2 / -2 |
| netinet | 13,388 | 13,401 | +16 / -3 |
| miscfs | 15,315 | 15,329 | +17 / -3 |
| ufs | 17,481 | 17,597 | +123 / -7 |
| sys | 12,490 | 13,624 | +1,145 / -11 |
| kern | 28,810 | 32,746 | +4,266 / -268 |
| i386 | 23,587 | 37,345 | +14,932 / -1,737 |
| ddb, boot | 0 | 7,838 | new |

So the starting point is clean: whatever 4.6BSD becomes, it starts as
Lite2 plus an i386 port.

**How much of Lite2 each daughter kept, over time.** For every `.c` and
`.h` file of Lite2's `kern`, `vm`, `ufs`, `miscfs`, `nfs`, `net`,
`netinet` and `sys`, the share of its lines that survive (`diff -w`) in
the file at the same path in the donor:

| Donor | All | kern | vm | ufs | nfs | net | netinet | sys |
|---|---|---|---|---|---|---|---|---|
| OpenBSD 1997-06 | 84% | 78 | 93 | 81 | 87 | 91 | 85 | 90 |
| NetBSD 1.3 (1997) | 80% | 77 | 91 | 78 | 83 | 90 | 75 | 88 |
| FreeBSD 2.2 (1997) | 77% | 72 | 52 | 76 | 94 | 91 | 85 | 83 |
| OpenBSD 2001-12 | 68% | 74 | 0 | 77 | 71 | 86 | 80 | 82 |
| FreeBSD 4.0 (2000) | 67% | 65 | 45 | 55 | 79 | 80 | 72 | 72 |
| NetBSD 1.5 (2000) | 66% | 69 | 8 | 72 | 78 | 83 | 69 | 82 |
| OpenBSD 2004-06 | 63% | 69 | 0 | 74 | 70 | 78 | 72 | 74 |
| OpenBSD 2008-01 | 50% | 59 | 0 | 43 | 64 | 72 | 70 | 70 |
| OpenBSD 2014-06 | 42% | 51 | 0 | 41 | 47 | 64 | 65 | 60 |
| FreeBSD 5.0 (2003) | 41% | 52 | 40 | 42 | (moved) | 74 | 67 | 61 |
| NetBSD 7 (2015) | 40% | 38 | 0 | 44 | 47 | 59 | 53 | 60 |
| NetBSD trunk (2026) | 37% | 36 | 0 | 39 | 46 | 53 | 48 | 57 |
| OpenBSD 2026-10 | 35% | 44 | 0 | 37 | 43 | 42 | 53 | 54 |

A file the donor moved counts as lost (FreeBSD 5.0 moved `nfs` and
`miscfs`), so later FreeBSD figures are low by that much. The
measurement script is `ret.py` in the session scratchpad; it is to go
into `build/` with the next measurement patch if the maintainer wants
it kept.

What it shows:

- **1994 to 2001 is the continuation window.** All three daughters kept
  77-84% of Lite2 in 1997. That is the era the shrink-wrap rule draws
  from first.
- **The VM is the first and sharpest break.** OpenBSD and NetBSD drop
  Lite2's `vm/` entirely when they take UVM (NetBSD 1.4, 1999; OpenBSD
  from December 1999, compulsory May-June 2001). FreeBSD keeps about
  half of it, but redesigned (section 4).
- **After 2001 the drift is mostly each project's own agenda,** not
  anything POSIX or hardware forced: OpenBSD's security work, the 2008
  buffer-cache rework, the 2013 ABI break, the 2014-16 multiprocessor
  network stack; NetBSD's 5.0 locking; FreeBSD's SMPng. 4.6BSD's rules
  exclude most of that, which is why it can stay closer to Lite2 than
  any daughter does today.
- **OpenBSD in mid-2004, with UVM, i386 SMP and amd64 already in,
  still kept 63% of Lite2.** That is the nearest measured reference for
  a 4.4 system that has taken the structural changes 4.6BSD needs.

## 3. Layer by layer: which lineage, how far, what class

| Layer | Lineage to follow | Clean as far as | Class | Notes |
|---|---|---|---|---|
| VM | Lite2 Mach VM → NetBSD 1.3 / OpenBSD to 2001-05, then the choice in §4 | 2001-05 | (a) then the fork | §4 |
| pmap interface | Lite2's 26 entry points; OpenBSD 1999's dual `PMAP_NEW` `pmap.h` | 2001 | (a) | the seam under any VM |
| Buffer cache | Lite2/OpenBSD (`vfs_bio.c` 8.6, still in OpenBSD 2026) | 2008 (fixed `nbuf`) | (a); the 2008 dynamic cache (b) | no unified page cache is needed (§5) |
| UFS/FFS, FFS2 | OpenBSD (FFS2 2004-07; 64-bit `daddr_t` 2006-13) | 2014 | (a/b) | VM-neutral: touches the VM only through `uvm_vnp_setsize`/`uncache`, renames of Lite2's `vnode_pager_*` |
| VFS, `*at()` | OpenBSD 2011-07-07 (`8ef87afd565`, +494/-61) | 2014 | (a) | same `vfs_lookup.c` site as Lite2's |
| NFS | Berkeley 8.16 as OpenBSD kept it | 2026 | (a) | |
| Threads | NetBSD 1.1 kernel-stack move, then OpenBSD rthreads 2005-12-03 (`802bb6ac573`, +356/-30, no VM change) | 2017 (futex) | (a) | the fixed `kstack` at `USRSTACK` must go first |
| Scheduler | 4.4's, as OpenBSD kept it (`schedcpu`, still in 2026) | 2009 (per-CPU queues) | (a) | |
| Time | OpenBSD: `clock_gettime` 1997, timecounters 2004, 64-bit `time_t` 2013 | 2013 | (a) | |
| POSIX realtime (timers, RT signals, AIO, clock selection) | none in OpenBSD; NetBSD 2.0+/5.0 | -- | (b) or (c) | §7.3 |
| Locking/SMP core | OpenBSD's (spl kept, `KERNEL_LOCK`, `mtx` at IPL, rwlock) | 2014 (plateau) | (a) | already decided |
| x86 MP, APIC | NetBSD (Sommerfeld 2000, fvdl 2002-03) via OpenBSD 2004-06-13 | 2014 | (a) on UVM; (b) on Mach VM | §4, §7.1 |
| Autoconf, bus, PCI | NetBSD 1.0-1.3 / OpenBSD 1997 | 2014 | (a) | interrupt model is Lite2's `cpl` mask with an `imask[IPL]` table |
| Driver VM calls | OpenBSD's | 2014 | (a) on UVM; (b) on Mach VM | about ten names to shim |
| amd64 | OpenBSD 2004-01-28 (`f5df1827e93`, from NetBSD x86_64) | 2014 | (a) on UVM; (b)/(c) on Mach VM | §4 |
| ACPI | OpenBSD acpi(4), 2005 | 2014 | (a) | small VM coupling |
| UEFI, boot arguments | OpenBSD efiboot 2015, `bootarg` | 2015 | (a) | extends Lite2's `howto, bootdev, ..., esym` |
| Storage | OpenBSD wdc/pciide, SCSI midlayer, AHCI 2006, NVMe 2014, GPT 2014 | 2014 | (a) | replaces the FreeBSD slice layer |
| Console, USB | OpenBSD wscons/rasops/efifb, USB core | 2014 | (a) | required under UEFI |
| Network stack | Lite2 → OpenBSD | 2014-06 | (a) | the MP rework starts 2014-07, not 2015 |
| Socket API | OpenBSD: `poll` 1996, `socklen_t` 1999, `MSG_NOSIGNAL` 2011, `accept4`/`SOCK_CLOEXEC`/`pipe2`/`dup3` 2014-08-31 | 2014 | (a) | |
| IPv6 | KAME, OpenBSD 1999-12 | 2014 | (b), large | optional in POSIX (`_POSIX_IPV6`) |
| tty | Lite2 tty with the FreeBSD 2.0 cblocks we have; OpenBSD ptm 2004 | 2014 | (b) | see §7.4 |
| User ABI | OpenBSD's 2013 flag day (`91a535ff42f`) | 2013 | (a) | no compat layer needed: no installed base |
| ELF auxv, TLS, ld.so | NetBSD 1.1 `elf_copyargs`; OpenBSD `__set_tcb`, ld.so | 2012 | (a)/(b) | |
| Removal: ISO, XNS, X.25 | OpenBSD 2004-06 | -- | (a) | every use is behind an option |

## 4. The VM: the decision under every other one

**What the Mach VM can reach by following its lineages.** NetBSD 1.3
and OpenBSD 1997-2001 are Lite2's VM with fixes: 88-98% of the lines of
each main file and 159-162 of Lite2's 180 VM functions survive. Their
last additions are the ones a driver interface needs: Cranor's
physical-segment code (`vm_physseg`, OpenBSD 1998-03-01) and
`vm_page_alloc_memory` for `bus_dma` (NetBSD 1.3, OpenBSD 1999). That is
the plateau. Its end is dated:

- 1999-12-01, `d7f71f22e60`, "uvm on the i386": OpenBSD's i386 GENERIC
  switches to UVM. For 18 months the tree builds both ways, every VM
  call written under `#if defined(UVM)`.
- 2001-05-05, `318eb79dd09`, "PMAP_NEW and UVM are no longer optional
  on i386"; `sys/vm` is gone from OpenBSD by January 2002.

**Why amd64, SMP and PAE all sit on the far side.**

- *amd64.* NetBSD x86_64 (2001) and OpenBSD amd64 (2004, "hacked by art@
  from netbsd sources"; its `pmap.c` is NetBSD's 1.3 of 2003-05-08, 32
  `uvm_` references) are UVM. FreeBSD's amd64 (5.x) is on FreeBSD's own
  VM (53 `vm_page_t`, queue locks), newbus (25 `DEVMETHOD`s in
  `nexus.c`) and SMPng. Nothing else exists.
- *SMP.* OpenBSD's i386 SMP (2004-06-13, `012ea299774`, +10,647) is Bill
  Sommerfeld's NetBSD MP branch of 2000 (`lapic.c 1.1.2.8 2000/02/23
  sommerfeld`) on NetBSD's UVM pmap 1.91. FreeBSD 3.0's SMP is the only
  pre-UVM x86 SMP, and it is written on FreeBSD's i386 base, which this
  tree shares little of.
- *PAE.* OpenBSD `pmapae.c` (2006) is UVM; it needs a `paddr_t` split
  that UVM made in 1998 and the Mach VM never did.

**FreeBSD's VM is not the way round.** It is Lite2's Mach VM continued,
but it becomes a redesign between 2.1.0 and 2.2: `vm_object` loses the
pager, shadow and copy fields; 3.0 drops 65 Lite2 VM functions (the
whole `vm_pager_get/put` layer) and adds 128; by 4.11 `pmap_enter`
takes a `vm_page_t`. Its line survival in `vm/` (52% in 2.2, 40-45% by
4.0-5.0) is no better than UVM's, and every piece that rests on it --
the VMIO buffer cache, the amd64 pmap, zones -- brings FreeBSD's other
layers with it.

**The three options.**

- **A. Mach VM to the 2001 plateau, and no further.** The most
  faithful to 4.4 (about 90% of `vm/` kept). Everything in section 3
  that is VM-neutral can be done on it, which is nearly all of the
  POSIX work and every i386 driver. But amd64, SMP and PAE then have no
  donor: the NetBSD/OpenBSD amd64 pmap would have to be grafted onto
  the Mach VM (about 40 call sites, each with a Mach equivalent, but a
  graft nobody made), and maintained alone. Every later driver needs a
  VM shim the project owns.
- **B. Mach VM first, then UVM where OpenBSD took it.** 4.6BSD repeats
  OpenBSD's own history: the plateau Mach VM, then the UVM transition
  as OpenBSD made it, with both builds kept under `#if UVM` until the
  switch, as OpenBSD did for 18 months. After it, amd64 (2004), SMP
  (2004), PAE (2006) and `bus_dma` are clean (a) backports. The cost is
  that `vm/` becomes UVM: 31-51% of Lite2's VM lines survive into
  NetBSD 1.4's UVM, and UVM is about 23,000 lines against Lite2's
  13,800. UVM keeps `vm_map`, `vm_page` and the pmap seam; it replaces
  objects and pagers with amaps, anons and `uvm_object`.
- **C. FreeBSD's VM.** Not recommended: no OpenBSD-compatible driver
  surface, and its amd64 needs FreeBSD 5.x in every layer.

**Assessment.** UVM is not a foreign system grafted on: it is Chuck
Cranor's redesign of the 4.4 VM inside NetBSD (1998), and two of the
three daughters chose it. B is the canonical continuation if 4.6BSD
keeps amd64, SMP and PAE as goals. A is only consistent if those are
dropped or a project-written VM graft is accepted. **This is the
maintainer's decision, and it should be made before Tier A drivers or
SMP are begun,** because it decides whether those come with a shim or
without one. Lite2's other ports would go through the same transition;
NetBSD 1.4 ran its old-interface pmaps under UVM (`pmap.c !pmap_new` in
its `files.i386`), but how each Lite2 port's NetBSD descendant
converted was not checked (**UNVERIFIED**).

## 5. File systems and VFS

The VM choice does not constrain the file systems, provided they are
taken from OpenBSD:

- **No unified page cache is needed.** Lite2 keeps `write()` in the
  buffer cache and mapped pages in the VM, and does not invalidate one
  from the other while a file is mapped. OpenBSD does the same today
  (`uvm_vnode.c`: "this function should DIE once we merge the VM and
  buffer cache"). POSIX.1-2024's `mmap()` page puts synchronisation
  between `mmap()` and `read()`/`write()` on the application; the other
  pages were not checked (**UNVERIFIED**). NetBSD's UBC (needs UVM) and
  FreeBSD's VMIO (needs FreeBSD's VM) are both (c) and both avoidable.
- **2038 and large disks.** FFS1's inode has 32-bit seconds (it already
  stores nanoseconds). OpenBSD's FFS2 (2004-2007) keeps Lite2's
  `ufs`/`ffs` split with a dinode union; it needs the 64-bit `daddr_t`
  first (OpenBSD 2006-2013), which is mechanical but wide (`buf.h`,
  `bread`/`getblk`, every strategy routine). Today `b_blkno` is a 32-bit
  count of sectors: 1 TiB per disk. NetBSD's and FreeBSD's UFS2 are tied
  to UBC and VMIO: (c), not needed.
- **Soft updates:** OpenBSD took them in 1998 and removed them in 2024.
  Not taken.
- **SMP:** OpenBSD's VFS still runs under the kernel lock in 2026, with
  Lite2's `lockmgr` vnode locks turned into a compatible rwlock. That is
  consistent with a big-lock 4.6BSD. NetBSD's fine-grained VFS (its
  vnode interlock *is* the UVM object lock) and FreeBSD's MPSAFE VFS
  are (c).

## 6. Processes, threads, time, signals

- **The prerequisite is already known:** Lite2's kernel stack sits at
  `USRSTACK` (`locore.s:175`), the same address in every process. Two
  threads, or two CPUs, cannot share that. NetBSD moved it between 1.0
  and 1.1 (1995), on Lite VM; OpenBSD's first import already has it.
  (a), and the same change gives ddb its `trace/t`.
- **Threads: OpenBSD rthreads,** in its 2005 form: threads are
  processes sharing a vmspace, 16 files, no VM change; then
  `thrsleep`, the `struct process` split (2007-2010) and futex (2017),
  each an increment. NetBSD's LWPs (tied to its 5.0 locking) and
  FreeBSD's `struct thread`/KSE (tied to SMPng) are (c) as lineages.
- **Time:** all (a) from OpenBSD.
- **Mandatory POSIX options OpenBSD does not have.** POSIX.1-2024
  `<unistd.h>` (checked) makes 14 options mandatory, 202405L. OpenBSD's
  `unistd.h` today sets four of them to -1:
  `_POSIX_ASYNCHRONOUS_IO`, `_POSIX_CLOCK_SELECTION`,
  `_POSIX_REALTIME_SIGNALS`, `_POSIX_TIMERS`. So the OpenBSD line alone
  cannot reach Issue 8 (§7.3).
- **Smaller gaps,** each (a): SysV message queues and semaphores (Lite2
  has only shared memory; NetBSD 1.3 and OpenBSD 1997 have all three),
  `siginfo` (OpenBSD 1997), `ppoll`/`pselect`, `getentropy`, `waitid`,
  and `struct timespec`'s `ts_sec`/`ts_nsec` renamed to POSIX's
  `tv_sec`/`tv_nsec` (`sys/time.h:54`).

## 7. The irreconcilables, and the way out of each

**7.1 amd64 + the Mach VM + the plateau drivers.** No donor. Ways out:
option B of section 4 (UVM, as OpenBSD did), or option A with a
project-written graft of the OpenBSD/NetBSD amd64 pmap onto the Mach
VM. B keeps one lineage per layer; A does not.

**7.2 Two SMP lineages.** `roadmap-4.6.md` names FreeBSD 3.0's SMP as the
only donor of its era (6,236 lines). Taken with an OpenBSD/NetBSD amd64,
it would give one x86 two SMP codebases and two APIC/MP-table
implementations, and FreeBSD 3.0's `cpl`/`ipl.s` interrupt model
conflicts with the OpenBSD `spl`/IPL model already decided. Way out:
drop FreeBSD 3.0 SMP; take x86 MP from NetBSD via OpenBSD 2004, which
needs the VM decision of section 4 (on the Mach VM its TLB shootdown has
to be ported into Lite2's pmap: (b)).

**7.3 POSIX realtime: timers, realtime signals, AIO, clock selection.**
Only NetBSD (`timer_create` from 2.0, `clock_nanosleep`, `sigqueue`,
`sys_aio.c`, all on its 5.0 mutexes and condition variables) and
FreeBSD (SMPng) have them. They are mandatory in Issue 8. Ways out:

- adapt NetBSD's code to OpenBSD's primitives (`mutex_enter` to
  `mtx_enter`, `cv_wait` to `msleep`), as the roadmap already plans for
  NetBSD 5 drivers: (b), and the clearest place where 4.6BSD goes
  beyond any daughter;
- AIO may instead be provided in libc over threads, since POSIX does not
  say it is in the kernel; which donor's userland AIO would fit is
  **UNVERIFIED**.

This is a graft from a second lineage in one area, and should be
recorded as such. It is the price of conformance; leaving it out costs
four mandatory options.

**7.4 The FreeBSD-only pieces already in the tree.**

- *The slice layer* (`subr_diskslice.c`, `subr_dkbad.c`,
  `diskslice_machdep.c`, `diskslice.h`) exists only because FreeBSD
  2.1.0's `wd.c` needs it. FreeBSD dropped it for GEOM in 2003; it reads
  only the MBR. Way out: when the i386 moves to Tier A, `wd`, `atapi`
  and `wcd` give way to OpenBSD's `wdc`/`pciide`/`atapiscsi`, and MBR
  and GPT are read in `readdisklabel` as OpenBSD does. Its only consumer
  is `wd.c`, so it leaves cleanly.
- *The clists:* `tty_subr.c` is FreeBSD 2.0's cblocks (with `ndqb`
  still a stub). OpenBSD's `tty.c` uses de Raadt's ring-buffer clists
  (`clalloc`, `firstc`). Following OpenBSD's tty means a small shim or
  taking de Raadt's clist as well: (b). FreeBSD 8's `ttyq` would be a
  fork: (c), not taken.
- *BTX and boot2* stay as the BIOS path; the UEFI path is OpenBSD's
  efiboot, a separate program. Not a roadblock, but two boot lineages
  for one architecture, as OpenBSD itself has (biosboot and efiboot).

**7.5 i386 and amd64 sharing x86 code.** `design-4.6.md` rule 5 says
"i386 and amd64 share their common x86 code, as NetBSD's `sys/arch/x86`
does". OpenBSD never did this (it has no `sys/arch/x86`; its i386 uses
one amd64 file), and NetBSD's sharing began about February-April 2003
(inferred from RCS dates) on its own line, which went on past the
plateau. Ways out: keep separate i386 and amd64 trees as OpenBSD does
(one lineage), or make the shared directory ourselves and record it as
4.6BSD's own work (kind C). It cannot be had as a backport while the
drivers and amd64 come from OpenBSD.

**Not roadblocks, and stated so they are not re-examined:** the unified
buffer cache, soft updates, fine-grained SMP in the VM, VFS and network
(all beyond the plateau and outside the rules), NetBSD's syscall
versioning and FreeBSD's ino64 compatibility (no installed base to keep
working), and IPv6 (large but compatible, and optional in POSIX).

## 8. Distinct BSD, or patchy OpenBSD?

**What the code will come from.** Where 4.6BSD changes Lite2, the
donor will most often be OpenBSD, because the measurements and the
earlier decisions agree that OpenBSD kept Lite2's design: its
scheduler is still 4.4's (`kern_synch.c`'s NetBSD 1996 header on
`sched_bsd.c`), its `vfs_bio.c` still carries Berkeley's 8.6 line, its
`subr_autoconf.c` is still Torek's, and its VFS still runs under
`lockmgr`-style locks and one kernel lock. The x86 machine layer will
be NetBSD's code as OpenBSD carried it. Someone reading the changed
files will see much OpenBSD.

**Why that is not the same as being OpenBSD.**

- *It takes much less.* OpenBSD's kernel today keeps 35% of Lite2's
  lines; most of what it replaced was its own choice: security
  hardening, the 2014-16 multiprocessor network stack, the dynamic
  buffer cache, `pledge`/`unveil`, removed interfaces, constant
  restyling. 4.6BSD's rules take only what POSIX, the ABI or hardware
  forces. OpenBSD in 2004, with UVM, i386 SMP and amd64, kept 63%. A
  4.6BSD that takes the same structural changes and little else should
  keep more than that. That is a reasoned expectation, not a
  measurement, and should be measured at each stage with the same
  script.
- *It goes where no daughter went.* No BSD has complete POSIX.1-2024:
  OpenBSD lacks four mandatory options, `tcgetwinsize` and `O_SEARCH`/
  `O_EXEC`; NetBSD has them on a locking model far from 4.4's. 4.6BSD
  is the system that completes Issue 8 on a 4.4 kernel.
- *It keeps what OpenBSD dropped.* Lite2's own ports, Lite2's kgdb or
  ddb question, Lite2's interfaces where they still serve.
- *It stops where OpenBSD kept going.* The driver interface at the
  plateau; big-lock SMP.

**Where it could become a patchwork, and the rule that prevents it.**

1. Grafts from a second lineage in one layer: FreeBSD 3.0's SMP beside
   NetBSD's x86 MP, or NetBSD's realtime code beside OpenBSD's. Rule:
   one lineage per layer; where a second is unavoidable (7.3), it is
   recorded as a graft, adapted to the first lineage's primitives, and
   kept to one subsystem.
2. Shims the project owns, which grow: a Mach-VM shim under OpenBSD
   drivers is the largest of these. Rule: avoid by the VM decision, or
   write the shim once, record it, and measure it.
3. The FreeBSD-only pieces already in the tree, each with a planned way
   out (7.4).
4. Own writing that looks like a backport. Rule as now: kind C and
   AI-ONLY NOTEs.

**How far 4.5BSD can be extended by heritage alone.** On the Mach VM,
with no graft: the i386, uniprocessor, PCI and ISA, Tier A and Tier B
drivers on a small VM shim, AHCI and NVMe on the i386, USB, wscons,
OpenBSD's ACPI, the thread model, the 64-bit ABI, FFS2, the `*at()`
calls, sockets, terminals: that is nearly all of POSIX.1-2024 except the
four realtime options. **The hard wall is amd64, SMP and PAE**, which no
lineage provides on the Mach VM, and UEFI, which only matters with
amd64. Past that wall the lineage continues cleanly through OpenBSD's
UVM-era code to its 2014 plateau.

## 9. Corrections to the existing records (for the maintainer)

1. `design-4.6.md` §2 rule 5, "as NetBSD's `sys/arch/x86` does":
   conflicts with an OpenBSD lineage for amd64 and drivers (7.5).
2. `design-4.6.md` §6: the multiprocessor network interface begins
   2014-07/08 (`if_rxr`, `mbuf_list`), not 2015-16. The plateau ends at
   about June 2014 for network drivers.
3. `roadmap-4.6.md`, *SMP from FreeBSD*: FreeBSD 3.0 SMP should be
   withdrawn as the SMP donor (7.2).
4. 64-bit `time_t` is required by POSIX.1-2024 itself (Austin Group
   bug 1462, applied for Issue 8), not only by amd64. A 32-bit `time_t`
   is allowed only in a legacy environment beside a conforming one.
   The i386 therefore needs OpenBSD's 2013 change, or the i386 is a
   legacy environment and conformance is claimed on amd64.
5. `roadmap-4.6.md`, *Modern hardware*: the slice layer and FreeBSD
   2.1.0's `wd`/`atapi`/`wcd` are to leave with Tier A, and the plan
   should say so (7.4).

## 10. What was checked again

Checked against the OpenBSD history: `d7f71f22e60` (1999-12-01, UVM on
the i386), `318eb79dd09` (2001-05-05, UVM no longer optional),
`f5df1827e93` (2004-01-28, amd64 from NetBSD; its `pmap.c` NetBSD 1.3
of 2003-05-08, 32 `uvm_` uses), `012ea299774` (2004-06-13, SMP;
`lapic.c` Sommerfeld 2000; i386 `pmap.c` NetBSD 1.91 of 2000),
`802bb6ac573` (2005-12-03, rthreads), `8ef87afd565` (2011-07-07,
`*at()`), `91a535ff42f` (2013-08-13, 64-bit `time_t`), `4c0bd79e7ea`
(2001-12-04, level-based IRQs); OpenBSD's master `sys/arch` (no `x86`)
and `unistd.h` (the four -1 options); `sched_bsd.c`, `vfs_bio.c`,
`subr_autoconf.c` headers. POSIX.1-2024 `<unistd.h>` for the mandatory
options, and Austin Group bug 1462. In this tree: `kstack` at
`USRSTACK`, `ts_sec`, and NetBSD 1.1's `elf_copyargs`.

Taken from the surveys and not checked again: the FreeBSD figures
(its clone was partly unreadable while the disk was full), the NetBSD
2.0 and 5.0 details, the line counts of donor subsystems, and the
dates marked as inferred. The donor clones are partial; `git grep`
over whole trees on them fetches from the network and was avoided.
