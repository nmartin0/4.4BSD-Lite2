# The continuations of 4.4BSD, subsystem by subsystem, and how they fit

Written 2026-10-29 at the maintainer's request, as a second and deeper
pass over `analysis-4.6-lineage.md`, which it corrects (section 9). The
task as set: for each major subsystem, find the implementation that is
the most minimal and canonical continuation of 4.4BSD -- additions to
the 4.4 design, not a redesign -- tracing each daughter as far forward
as it continues 4.4, so that the plateaus are known; then see how well
the continuations fit together. 4.6BSD is to be 4.4BSD-Lite2 modernized
by backporting those continuations, and nothing else.

**Donor order** (the maintainer's rule for 4.6BSD, 2026-10-29): OpenBSD,
NetBSD, FreeBSD; then MirOS BSD; then DragonFly BSD; then any other
permissive BSD (Bitrig is the one examined here). Lite2 itself and the
clearance rules come before all of them, as before.

**Sources read.** OpenBSD's full history (1995-2026); NetBSD's branch
tips 1.0 to 1.6 and 2 to 8 and trunk (and its history to 1.6); FreeBSD's
releases 2.0 to 9.0 in full and its history; DragonFly's full history
(2003-2026); Bitrig's (2012-2017). **MirOS BSD could not be read**: its
source is offered only by anonymous CVS or rsync over SSH from
mirbsd.org, which this workspace cannot reach. What is said of it here
is from the web and is marked so; a copy of its `src` would let it be
checked properly.

Marks used: **C**, a continuation of 4.4's design (fixes and additions to
it); **R**, a redesign (the 4.4 structure replaced); **UNVERIFIED**, not
checked against source.

## 1. In one page

| Subsystem | Most canonical continuation | Its plateau | Redesigns that end the other lines |
|---|---|---|---|
| VM | Lite2's Mach VM as NetBSD 1.3 and OpenBSD to 2001-05 fixed it, plus FreeBSD's changes before 1995-07-13 | OpenBSD 2001-05; NetBSD 1.3; FreeBSD 1995-07-12 | UVM (NetBSD 1.4, OpenBSD 2001); FreeBSD's object/pager overhaul (1995-07-13); DragonFly starts after it |
| Locking, SMP, scheduler | OpenBSD's: spl kept, `__mp_lock` big lock (2004), `mtx` at IPL (2004), rwlock (2003), 4.4 scheduler with per-CPU queues (2009) | about 2014 | NetBSD 2.0 (LWP/SA, new x86 interrupts) and 5.0 (newlock2); FreeBSD 5.0 (SMPng); DragonFly 2003 (LWKT) |
| Threads | OpenBSD rthreads (2005; threads are processes) | futex 2017 | NetBSD LWP/SA 2.0, 1:1 5.0; FreeBSD KSE 5.x; DragonFly LWKT |
| POSIX realtime | timers: NetBSD 2.0; realtime signals: NetBSD 1.4 and 2.0; AIO: FreeBSD 3.0; rest: OpenBSD | NetBSD 4.0; FreeBSD 4.11 | NetBSD 5.0; FreeBSD 5.0; DragonFly removed AIO (2011) |
| VFS, buffer cache, UFS, NFS | OpenBSD's | about 2014; selectively to 2026 | NetBSD UBC (1.6) and 5.0; FreeBSD VMIO (2.0-2.2) and newnfs; DragonFly nlookup (2004) |
| Sockets, network | OpenBSD's | 2014-06 | OpenBSD's own MP stack (2014-07 on); NetBSD 5.0; FreeBSD 5; DragonFly netmsg (2004) |
| tty | Lite2's `tty.c` on FreeBSD 2.0's cblocks (what this tree has), with OpenBSD's additions | 2026 (OpenBSD never redesigned it) | FreeBSD 8.0 (new tty layer); NetBSD 5.0 (locking) |
| exec, ELF, user ABI | NetBSD 1.0-1.1's exec framework and ELF; OpenBSD's ld.so (2000), TLS (2011), 64-bit ABI (2013) | about 2017 | -- |
| x86 machine layer | OpenBSD's separate i386 and amd64 (amd64 is NetBSD's of 2003); or NetBSD 2-4's shared x86 | OpenBSD 2014; NetBSD 4.0 | NetBSD 5.0 (`device_t` split, locking); FreeBSD newbus (3.0); DragonFly |
| Drivers | OpenBSD's 1996-2014 plateau | 2014 | as decided |
| Boot | OpenBSD `bootarg` (1997) and `efiboot` (2015) | 2015 | -- |

**The decisive finding stands, corrected:** every subsystem has a 4.4
continuation that reaches the 4.6BSD targets *except the VM*, which no
daughter continued past 2001. The other continuations fit one another
with small shims, because most are OpenBSD's. Where they meet the VM,
they all speak UVM's names (OpenBSD's machine layers, pmap, pool,
bus_dma). Whether that makes UVM necessary is now a narrower question
than the first pass said (section 3).

**What the first pass got wrong, in brief** (section 9 has the list):
OpenBSD has parts of the four realtime options, not none of them; their
missing parts have older, spl-era donors (NetBSD 2.0, FreeBSD 3.0), not
only post-2009 ones; FreeBSD's VM is a continuation until a dated
commit of July 1995, and is a smaller departure from Lite2 than UVM;
and amd64, SMP and PAE need a pmap, not UVM as such.

## 2. The retention curve (from the first pass, still valid)

How much of Lite2's machine-independent kernel (`kern`, `vm`, `ufs`,
`miscfs`, `nfs`, `net`, `netinet`, `sys`) each daughter kept, by
`diff -w` at the same path:

| Donor | Kept |
|---|---|
| OpenBSD 1997-06 | 84% |
| NetBSD 1.3 (1997) | 80% |
| FreeBSD 2.2 (1997) | 77% |
| OpenBSD 2001-12 (after UVM) | 68% |
| FreeBSD 4.0 (2000) | 67% |
| NetBSD 1.5 (2000) | 66% |
| OpenBSD 2004-06 (UVM, SMP, amd64) | 63% |
| OpenBSD 2014-06 | 42% |
| FreeBSD 5.0 (2003) | 41% (low: some files moved) |
| NetBSD 7 (2015) | 40% |
| NetBSD trunk | 37% |
| OpenBSD 2026-10 | 35% |

## 3. The VM

### 3.1 Each lineage, and where it stops continuing 4.4

**OpenBSD: C until 2001-05.** Lite2's VM with fixes: 165 of Lite2's 181
VM functions kept in 1996, 160 in 2001; every field of `vm_object`,
`vm_page`, `vm_map` and `vm_map_entry` kept; 89% of the lines of the
nine main files. Additions: Cranor's physical segments (`vm_physseg`,
1998-03-01), `vm_page_alloc_memory` for `bus_dma` (1999). UVM became the
i386 default on 1999-12-01 (`d7f71f22e60`) and compulsory on 2001-05-05
(`318eb79dd09`).

**NetBSD: C from 1.0 to 1.3.** 179 of 181 functions in 1.0, 163 in 1.3;
all fields kept; 91% of the lines; `vm_page_alloc_memory` and
`vm_physseg` in 1.3. 1.4 is UVM.

**FreeBSD: C until 1995-07-12, then R.** FreeBSD 2.0 (1994) and the 2.1
branch are Lite2's design worked on by John Dyson and David Greenman:
`hold_count`, `act_count` and `busy` on pages; `vm_page_alloc_contig`
(1994-09-27, `d3c2cf7a8`); collapse fixes, `pmap_growkernel` and the
merged VMIO buffer cache (1995-01-09, `0d94caffc`); per-page `valid` and
`dirty` bits. FreeBSD 2.1.0 keeps 152 of the 181 functions and the
object's `pager`, `shadow` and `copy` fields. The redesign is one
commit, checked: `24a1cce34fc1`, David Greenman, 1995-07-13, "Much
needed overhaul of the VM system ... the idea of a pager structure has
essentially been eliminated. Objects now have types". Built on it, and
so R for this project: copy objects removed (1995-07-16), pages named by
`pindex` (1995-12), page colouring (1996), zones (1997), Dillon's swap
pager (1999), `vm_page_t` in `pmap_enter` (2000-01), UMA (2002), PAE's
`vm_paddr_t` (2003), amd64 (2003). FreeBSD 2.1.0 to 2.1.7 shipped from
the branch made before the overhaul (whether 2.1.5-2.1.7 took later code
is **UNVERIFIED**).

**MirOS:** forked from OpenBSD in 2002, after UVM; so UVM by inheritance
(web; **UNVERIFIED**).

**DragonFly: no continuation of 4.4's VM.** It began from FreeBSD 4.8
(2003-06-17), years after FreeBSD's overhaul; its first release keeps
38% of Lite2's VM lines and 7 of `vm_object`'s 14 fields. It continued
FreeBSD 4's design (MP tokens 2010) and redesigned it again in 2019.

**Bitrig:** UVM, as OpenBSD.

### 3.2 A provenance hazard in FreeBSD's VM, for the maintainer's ruling

Checked in FreeBSD 2.0: `vm_page.c` and `vm_pageout.c` carry "from:
@(#)... 7.4 (Berkeley) 5/7/91", `vnode_pager.c` "7.5 (Berkeley)
4/20/91", and i386 `pmap.c` "7.7 (Berkeley) 5/12/91" -- Net/2 versions,
carried through FreeBSD 1.x, which grew from 386BSD. FreeBSD 2.1's
`vm_map.c`, `vm_object.c` and `vm_fault.c` carry Lite's 8.x versions,
as do all of NetBSD 1.3's and OpenBSD 2001's VM files. The rule excludes
what the daughters held derived from 386BSD's files; these four files
may be such. Whether they are tainted is **not decided here**; until it
is, FreeBSD's VM changes should be taken only where they touch the
Lite-derived files, or rewritten from their description.

### 3.3 UVM against FreeBSD's overhaul: which departs less from 4.4

| | FreeBSD 2.2 / 4.0 | UVM (NetBSD 1.4) |
|---|---|---|
| Lite2 VM function names kept (of 181) | 125 / 111 | 2 exact; 27 after renaming `vm_` to `uvm_` |
| `vm_object` fields kept (of 14) | 9 / 7; shadow chains kept (`backing_object`) | about 4 in meaning (`uvm_object`) |
| `vm_page` (of 8) | 6 / 5 | 7 |
| `vm_map` (of 13) | 11 / 8 | 12 |
| pmap interface names kept (of 29) | 24 / 24 | 25 |
| Lines | about 16,500 | about 23,200 |

UVM replaces 4.4's model of anonymous memory -- objects, shadow and copy
chains, pagers -- with amaps and anons; it keeps the map, the page and
the pmap seam a little better. FreeBSD keeps the objects and the fault
model and folds the pager into the object. By structure, **FreeBSD's
overhaul is the smaller departure.** By this project's standard both
are redesigns.

### 3.4 What the targets need from the VM, and what is really forced

- **SMP with a big lock is pmap work, not VM work.** OpenBSD's `uvm/` of
  2004 has no SMP code at all; its i386 `pmap.c` has 115 lines of it
  (TLB shootdown). FreeBSD 3.0's `vm/` has 7 lines, all in the zone
  allocator. Lite2's Mach VM can run under a big lock with a shootdown
  in its pmap.
- **PAE and more than 4 GB** need a physical-address type split from
  `vm_offset_t` (UVM's `paddr_t` 1998; FreeBSD's `vm_paddr_t` 2003): a
  mechanical change on any base.
- **amd64 needs a pmap and a machine layer.** Every donor's is written to
  a VM other than Lite2's: OpenBSD's 2004 amd64 (from NetBSD's of 2003)
  to UVM, about 30 `uvm_`/`pmap_kenter_pa` uses in the machine layer and
  the pmap; DragonFly's 2008 x86_64 (from FreeBSD's amd64) to FreeBSD
  4's post-overhaul VM, with `vm_page_t` in `pmap_enter`, and a small
  DragonFly shim (no tokens; 18 `crit_enter`). Neither runs on Lite2's
  VM as it stands; each Mach equivalent exists (`vm_page_alloc`,
  `kmem_alloc`, `vm_map_find`), so a graft is mechanical but has no
  precedent.
- **bus_dma's contiguous memory**: NetBSD 1.3's `vm_page_alloc_memory` or
  FreeBSD's `vm_page_alloc_contig` (1994), both on the Lite design.
- **pool(9)**, which OpenBSD's mbufs and many later subsystems use: the
  first import (OpenBSD 1999-02-26, from NetBSD) had a Mach-VM page
  back end; it was removed in 1999-11 ("Pool will not work with old
  vm"). About 40 lines.

So **UVM is not forced by design.** It is forced by *donor availability*:
the continuations chosen for every other layer were written, after
2001, against UVM's names.

## 4. Locking, SMP, interrupts, scheduler

**OpenBSD: C to about 2014.** spl kept throughout (on i386 a global
`cpl`, made level-based in 2001-12, `4c0bd79e7ea`; on MP the APIC's task
priority register); rwlock 2003-11; SMP 2004-06-13 (`012ea299774`, i386)
and 2004-06-25 (amd64) with one recursive kernel lock (`__mp_lock`) and
Lite2's `simple_lock` left a no-op; `mtx` "spl plus spin" 2004-07;
4.4's scheduler kept (`schedcpu`, `roundrobin`), its switch made
machine-independent in 2007 and given per-CPU queues in 2009; lockmgr
put on recursive rwlocks in 2013 with its interface kept. After 2014:
witness from FreeBSD (2017), SMR, the MP network stack -- additions of
other designs.

**NetBSD: locking C to 4.0 (2007), but with two redesigns underneath
from 2.0.** 2.0-4.0 keep lockmgr, a kernel lock built on it, real
`simple_lock` interlocks, `ltsleep` and the 4.4 scheduler. But 2.0 also
brought `struct lwp` with scheduler activations (abandoned in 5.0), and
a new x86 interrupt model (per-CPU software levels, `struct intrsource`:
the 2002 x86_64 design, carried to i386). 5.0 replaced the locking.

**NetBSD 2-4 against OpenBSD 2004.** The MP machine code is the same
(OpenBSD's came from Sommerfeld's NetBSD branch of 2000 and NetBSD's
x86_64 of 2003). NetBSD keeps 4.4's lock primitives more literally;
OpenBSD changes less of 4.4 overall: it keeps `struct proc`, `tsleep`,
the global `cpl` on i386 and one lock, and later adds 1:1 threads, which
NetBSD's SA base could not grow into. **OpenBSD's is the purer and the
smaller continuation.**

**FreeBSD: C to 4.11 in model** (a global `cpl` mask under a spin lock,
the `mp_lock` giant lock, simplelocks, lockmgr) -- but on FreeBSD's own
VM and i386, and a second x86 MP codebase if mixed with the others. 5.0
(SMPng) is R.

**DragonFly: R from 2003-06-20** (light-weight kernel threads,
interrupt threads, tokens). **Bitrig** adds its own machine-independent
mutexes and lock (2012-13): R, and nothing older than OpenBSD's.
**MirOS** forked before OpenBSD's SMP (web).

## 5. Threads and the POSIX realtime options

**What OpenBSD has and lacks (corrected).** Of POSIX.1-2024's fourteen
mandatory options, OpenBSD has ten (threads, semaphores, barriers, spin
locks, reader/writer locks, monotonic clock, timeouts, mapped files,
memory protection, thread-safe functions), at the 2001 value. Of the
other four it has parts:

| Option | OpenBSD has | OpenBSD lacks |
|---|---|---|
| Clock selection | `pthread_condattr_setclock` (2012), monotonic condition-variable timeouts | `clock_nanosleep` |
| Realtime signals | `SA_SIGINFO`, `sigwaitinfo`, `sigtimedwait` (over `__thrsigdivert`, 2009) | `sigqueue`, `SIGRTMIN`-`SIGRTMAX`, queued values, a `sigset_t` wider than 32 bits |
| Timers | -- (`timer_create` is a libc stub returning `ENOSYS`; syscalls 235-239 unimplemented since 1997) | all |
| Asynchronous I/O | -- | all |

**Threads: OpenBSD rthreads, C.** 2005-12-03 (`802bb6ac573`): threads are
processes sharing a `vmspace`, made by `rfork(RFTHREAD)`; 16 files,
+356/-30; then `thrsleep`, the `struct process` split (2007-2010), the
TCB syscalls (2011), futex (2017). NetBSD's LWPs (SA in 2.0, 1:1 in
5.0), FreeBSD's KSE/`struct thread` and DragonFly's LWPs are R.
Prerequisite: the kernel stack off its fixed address (NetBSD 1.1, 1995).

**The missing realtime pieces, oldest spl-era donor first:**

- **Realtime signals.** Widen `sigset_t` to 128 bits: NetBSD **1.4**
  (checked: `__bits[4]` in 1.4, `unsigned int` in 1.3), on an spl
  kernel; FreeBSD 4.0 did the same. Then the per-process queue of
  `ksiginfo` and `sigtimedwait`: NetBSD **2.0** (spl). `sigqueue` itself
  and the exposed `SIGRTMIN` came only in NetBSD 6 (2012, on 5.0
  locking) and FreeBSD 7 (SMPng); it is a few dozen lines over the 2.0
  queue and is ported to spl. **C**, with that one small adaptation.
- **Timers.** NetBSD **2.0** (shipped 2004; checked: `sys_timer_create`,
  `splclock`, `callout_reset`, a per-process `p_timers` array): it
  generalises Lite2's own `p_realtimer`/`realitexpire`, the interval
  timers becoming timer slots 0-2. **C**. The scheduler-activations
  upcall path and the `struct lwp` argument are taken out. Same shape
  in 3.0 and 4.0; R in 5.0. `SIGEV_THREAD` is libc's job: FreeBSD 7's
  `librt` is the only BSD implementation.
- **Clock selection.** `clock_nanosleep` first appears in NetBSD 7,
  FreeBSD 11.1 and DragonFly 6.0 (2021); it is `nanosleep` with a clock
  and `TIMER_ABSTIME`, a few dozen lines over `tsleep`. **(b)**: small,
  later donor, adapted.
- **Asynchronous I/O.** FreeBSD **3.0** (1998; checked: John Dyson's
  `vfs_aio.c`, 1997, on `splbio` and `tsleep`, not in 2.2.8), 2,049
  lines, with kernel processes that borrow the user's `vmspace` and a
  raw-device path through `physio`. **C** through FreeBSD 4.11; R under
  SMPng. DragonFly kept it until 2011-02-15 ("Eliminate AIO",
  `91bcbcd6be`; checked: ENOSYS stubs now). NetBSD's (2007) needs 5.0's
  LWPs and locks. No BSD does AIO in userland over threads. `aio_fsync`
  is a later FreeBSD addition (7.0).
- **Semaphores, barriers, spin locks, rwlocks:** OpenBSD's, in userland
  over `thrsleep`/futex. Only the option values change (to 202405L).
- **Smaller items:** `waitid` (OpenBSD 2022), `posix_spawn` (OpenBSD
  2012, from FreeBSD's), `pselect`/`ppoll` (OpenBSD 2013), `getentropy`
  (OpenBSD 2014). SysV message queues and semaphores: OpenBSD's
  `sysv_sem.c` (2002) and `sysv_msg.c` (2009) are rewrites; NetBSD's and
  FreeBSD's are Daniel Boulet's code of 1993-94 from the 386BSD/FreeBSD
  1 era, to be checked against the exclusion rule before any use.

So **the realtime options need a second lineage**, NetBSD's 1.4-2.0 and
FreeBSD's 3.0, and every piece of it is a continuation of 4.4 on spl and
`tsleep`. It is a graft in one area, to be recorded as such, not a
redesign.

## 6. Files, network, terminals, exec and the ABI

**VFS, buffer cache, UFS, NFS: OpenBSD's, C to about 2014.** Dirhash
(2003, from FreeBSD), FFS2 (2004-2006, keeping the ufs/ffs split),
64-bit block numbers in three steps (`daddr64_t` 2006-10, `daddr_t`
64-bit 2011-03, `daddr64_t` gone 2013-06), `st_atim` (2009-01), the name
cache on red-black trees (2009), the `*at()` calls, `utimensat`,
`O_CLOEXEC`, `O_DIRECTORY` (2011-07), lockmgr vnode locks on rwlocks
(2016). One translation boundary: in 2010-11 OpenBSD replaced the
generated `vnode_if` vectors with a hand-written table; the operations
are the same. The VFS touches the VM only through `setsize` and
`uncache`, so it fits any VM. `O_SEARCH` and `O_EXEC` are not in OpenBSD:
NetBSD (6 header; trunk kernel) or FreeBSD 8 (`O_EXEC`), small.
NetBSD's UBC (1.6) and 5.0 locking, FreeBSD's VMIO and newnfs, and
DragonFly's `nlookup` (2004) are R.

**Sockets and network: OpenBSD's, C to 2014-06** (`poll` 1996,
`socklen_t` and `sockaddr_storage` 1999, `timeout(9)` 2000, mbufs on
pool 2001, `MSG_NOSIGNAL` 2011); the 2014-08-31 `accept4`/`SOCK_CLOEXEC`/
`pipe2`/`dup3` commit is syscall-layer and can be taken after the
plateau. R from 2014-07 (`if_rxr`, `mbuf_list`, ifq, `NET_LOCK`). NetBSD
is C to 4.0, FreeBSD to 4.x, DragonFly R from 2003-06 (threaded
interrupts, network message threads). `sockatmark`: NetBSD 2001, 54
lines of libc over Lite2's `SIOCATMARK`.

**Terminals.** The clists: this tree's `tty_subr.c` is FreeBSD 2.0's
cblocks, and **that is the purer continuation of 4.4's interface**:
FreeBSD 2.0's `tty.c` calls the clist exactly as Lite2's does (11
`putc`, 5 `unputc`, 3 `nextc`, the same `c_cf`/`c_cl` uses), while de
Raadt's ring clist (NetBSD 0.9 1993, OpenBSD to 2026) changed `tty.c`
itself (`firstc`, `clalloc`, `clfree`). But the forward line is
OpenBSD's: its `tty.c` still keeps 74% of Lite2's lines in 2026 and was
never redesigned (FreeBSD's tty ends at 7.x). So: keep the cblocks, and
take OpenBSD's tty and pty changes through a small shim (`firstc` as a
4.4 `nextc` start; `clalloc`/`clfree` as no-ops). ptm: OpenBSD 2004
(+632/-88; no cloning device needed); `posix_openpt` and friends:
OpenBSD 2012 (105 lines of libc); `tcgetwinsize`: NetBSD 2017 (48
lines over Lite2's `TIOCGWINSZ`).

**exec, ELF, ABI.** The exec framework: NetBSD 1.0's (the encumbrance fix
in progress uses NetBSD 1.1's), then 1.1's ELF with the auxiliary
vector and `PT_INTERP`. ld.so: OpenBSD's own (2000). TLS: OpenBSD's
per-thread segment registers (2011-03 i386, 2011-04 amd64), TCB
syscalls (2011-10), ld.so ELF TLS (2016-05, not 2012). The 64-bit ABI:
OpenBSD's flag day (2013-08-13). Bitrig's 2012 TLS is FreeBSD's rtld,
a graft; not taken.

## 7. The x86 machine layer, drivers and boot

**The shared x86 directory.** NetBSD created `sys/arch/x86` on
2003-02-26 and used it for i386 and amd64 through 2.0, 3.0 and 4.0
(14,563 lines in 2.0, 23,054 in 4.0). It is mostly factoring of i386
code (`bus_space`, `bus_dma`, `isa_machdep`, `pci_machdep`), but carries
one redesign: the 2002 interrupt dispatch. It is **the only precedent
in the Torek lineage** for sharing i386 and amd64 code (FreeBSD's
`sys/x86` of 9.0 is newbus and SMPng; DragonFly shares nothing between
its pc32 and pc64).

**OpenBSD's 2004 amd64 is NetBSD's.** 98 of its 115 files carry NetBSD
revision IDs: the 2003 amd64 port and the `x86` files copied flat
(`intr.c`, `apic.c`, `lapic.c`, `ioapic.c`, `mpbios.c`, `bus_dma.c`).
OpenBSD's i386 kept Lite2's `cpl`/`imask` model, which is why OpenBSD
never shared i386 and amd64 code (checked: no cross-references in 2004,
2014 or 2026).

**Choices that keep one lineage per layer:**

1. **OpenBSD's separate i386 and amd64** (C to 2014): the i386 keeps
   Lite2's interrupt model; amd64 is NetBSD's code of 2003. With
   OpenBSD's own ACPI (2005), drivers, bootarg (1997) and efiboot
   (2015). Consistent with every other choice above. Cost: no shared
   x86 code.
2. **NetBSD 2.0-4.0's shared x86** (plateau 4.0, 2007): the shared
   directory, i386 on the new interrupt model. Costs: a process shim
   (`struct lwp` to `struct proc`, the SA code removed) or OpenBSD's
   `struct proc` versions of `locore`, `trap`, `vm_machdep` (the same
   files a year apart); and its ACPI glue calls ACPICA, which conflicts
   with OpenBSD's own ACPI. OpenBSD's drivers attach to it with two
   small shims (`CFATTACH_DECL`; `pci_intr_establish`'s name argument).

**Drivers:** OpenBSD's 1996-2014 plateau, unchanged. **Boot:** Lite2's
`howto, bootdev, ..., esym` extended by OpenBSD's `bootarg` (1997) or
NetBSD's `bootinfo` (1997), both continuations; UEFI only from
OpenBSD's efiboot (2015) or NetBSD 8. **Storage:** OpenBSD's wdc and
pciide (1999), AHCI (2006), NVMe (2014), GPT (2014, from Bitrig), all
machine-independent.

## 8. How the continuations fit together

Each layer's chosen continuation, and what it needs from its neighbours.
"Native" means the neighbour already speaks its interface.

| Seam | Needs | With the Mach VM kept | With UVM at OpenBSD's 2001 point |
|---|---|---|---|
| amd64 and i386 MP pmap ↔ VM | `pmap_kenter_pa`, `uvm_km_*`, `uvm_pagealloc`, page-table pages in an object (about 30 names) | a project-owned shim, or the pmap rewritten to Mach names; no precedent | native |
| bus_dma ↔ VM | contiguous page allocation | native (NetBSD 1.3's `vm_page_alloc_memory`) | native |
| pool(9), mbufs ↔ VM | a page back end | about 40 lines (the 1999 Mach back end) | native |
| drivers ↔ VM | `uvm_km_valloc`, `pmap_kenter_pa`, `pmap_extract` | about ten names shimmed (OpenBSD 1999 shows each Mach equivalent) | native |
| VFS ↔ VM | `setsize`, `uncache` | native (Lite2's names) | native (renamed) |
| AIO (FreeBSD 3.0) ↔ VM, processes | kernel processes borrowing a `vmspace`, `vmapbuf` | close (Lite2 has `vmspace` reference counts and `vmapbuf`); written to FreeBSD's post-1995 VM, so (b) | (b) |
| timers, signals (NetBSD 2.0) ↔ processes | `struct proc` timers, `ksiginfo` | (b): the `lwp` argument and SA paths removed | (b) |
| rthreads ↔ SMP ↔ machine | `struct proc`, `cpu_fork`, per-process kernel stack | native within OpenBSD | native |
| network ↔ callouts | `timeout(9)` | OpenBSD 2000's `timeout(9)` taken first | same |
| tty ↔ clists | `firstc`, `clalloc` | shim of a few lines | same |
| x86 ↔ ACPI | OpenBSD acpi(4) or ACPICA | OpenBSD's with OpenBSD's x86; ACPICA with NetBSD's | same |

**Reading the table.** Outside the VM row, every seam is native or a
shim of a few lines, because the chosen continuations are mostly one
lineage (OpenBSD's) plus a bounded realtime graft (NetBSD 1.4-2.0,
FreeBSD 3.0). The VM rows are where the choice shows:

- **Mach VM kept:** the most 4.4 in `vm/`; every 4.4 VM continuation
  available (NetBSD 1.3, OpenBSD to 2001, FreeBSD to 1995-07, subject to
  3.2). Costs one project-owned seam between a Mach VM and machine
  layers written for UVM -- the amd64 and MP pmaps above all -- and that
  seam grows with every later OpenBSD machine-dependent change taken.
- **UVM at OpenBSD's point:** every later layer native, and the history
  repeats OpenBSD's own (both builds kept for 18 months). Costs `vm/`:
  UVM is a redesign by this project's standard, and the larger of the
  two.
- **FreeBSD's VM after 1995** fits nothing else chosen here (no machine
  layer or driver donor of the plateau speaks it) and carries the
  provenance question of 3.2. Not a candidate.

**How far 4.6BSD can go by continuations alone.** With the Mach VM and
no graft: the i386, uniprocessor, the driver plateau with a few
renamed calls, threads, the 64-bit ABI, FFS2, the `*at()` calls,
sockets, terminals, and all fourteen mandatory POSIX options (the
realtime ones from NetBSD 1.4-2.0 and FreeBSD 3.0). That is all of
POSIX.1-2024. SMP on the i386 then needs OpenBSD's 2004 MP code with its
few VM calls renamed, and a TLB shootdown written into Lite2's pmap.
amd64 and PAE are where a decision is needed: a graft onto the Mach VM,
or UVM.

**Plateaus, the furthest each subsystem can be followed safely:**

| Subsystem | Furthest safe point |
|---|---|
| VM (Mach) | OpenBSD 2001-05 / NetBSD 1.3 / FreeBSD 1995-07-12 |
| VM (UVM, if chosen) | OpenBSD to about 2014 |
| Locking, SMP, scheduler | OpenBSD 2014 |
| Threads | OpenBSD 2017 (futex) |
| Timers, realtime signals | NetBSD 4.0 (2007) |
| AIO | FreeBSD 4.11 (2005) |
| VFS, UFS, NFS | OpenBSD 2014, selectively 2026 |
| Network | OpenBSD 2014-06 (socket calls to 2014-08) |
| tty | OpenBSD 2026 |
| exec, ABI, ld.so | OpenBSD 2017 |
| x86 machine layer | OpenBSD 2014, or NetBSD 4.0 for shared x86 |
| Drivers | OpenBSD 2014 |
| Boot, UEFI | OpenBSD 2015 |

## 9. Corrections to `analysis-4.6-lineage.md`

1. "OpenBSD has none of [timers, realtime signals, AIO, clock
   selection]": wrong. It has `sigwaitinfo`/`sigtimedwait`,
   `SA_SIGINFO` and the condition-variable clock; it lacks timers, AIO,
   `sigqueue`/queued signals/`SIGRTMIN`, and `clock_nanosleep`.
2. "Only NetBSD (post-5.0) and FreeBSD (SMPng) have them; adapt NetBSD's
   code to OpenBSD's lock primitives": wrong. NetBSD 2.0's timers and
   signal queue and NetBSD 1.4's `sigset_t` are spl-era; FreeBSD 3.0's
   AIO is spl-era. Only `sigqueue` and `clock_nanosleep` need small
   adaptation from later donors.
3. "FreeBSD's VM becomes a redesign between 2.1.0 and 2.2": the
   redesign is one commit, 1995-07-13; FreeBSD 2.0-2.1 are continuation
   donors for the Mach VM (subject to 3.2).
4. "Option C (FreeBSD's VM): not recommended": it stands for FreeBSD
   after 1995, but by structure FreeBSD's overhaul departs less from
   Lite2 than UVM does.
5. "amd64, SMP and PAE all sit on the far side [of UVM]": overstated.
   They need a pmap and a physical-address type; UVM is forced only by
   which donors exist, not by design.
6. NetBSD 2.0-4.0 were not examined in the first pass. They are the only
   Torek-lineage precedent for shared i386/amd64 code, and the source of
   the oldest spl-era POSIX timers and signal queue; their locking is
   not a closer continuation than OpenBSD's.
7. The clists: FreeBSD 2.0's cblocks (this tree's) are the purer
   continuation of 4.4's interface; OpenBSD's is the forward line, taken
   through a shim.
8. OpenBSD's ld.so gained ELF TLS in 2016, not 2012; ptm needs no
   cloning device.
9. MirOS BSD and DragonFly BSD were not in the first pass. DragonFly
   continues no 4.4 subsystem examined (it redesigned from 2003 and
   started after FreeBSD's VM overhaul); MirOS could not be read.

## 10. Checked again for this pass

Against the sources: OpenBSD master's `timer_create` stub, syscalls
235-239, `sigwait.c` (`sigwaitinfo`, `sigtimedwait` over
`__thrsigdivert`), `pthread_condattr_setclock`, `unistd.h`; DragonFly's
syscalls (timers unimplemented, `clock_nanosleep` 241) and its AIO
removal (`91bcbcd6be`, 2011-02-15); NetBSD 2's `kern_time.c`
(`sys_timer_create`, `splclock`, `p_timers`); NetBSD 1.3 and 1.4
`sigset_t`; FreeBSD 3.0's `vfs_aio.c` (Dyson 1997, `splbio`, `tsleep`)
and its absence from 2.2.8; FreeBSD commit `24a1cce34fc1`; the SCCS
versions of FreeBSD 2.0's and 2.1's, NetBSD 1.3's and OpenBSD 2001's VM
files. The rest is from the six surveys, with their **UNVERIFIED**
marks kept above.

## 11. Keeping the 4.4 Mach VM: its flaws, the Mach lines, portability

Added 2026-10-29, at the maintainer's question: how bad would it be to
keep Lite2's Mach VM and add improvements until the donors plateau?
The maintainer does not want a translation layer, and wants as few
shims as possible.

**Where 4.4's VM came from, and its two later Mach lines.** 4.4BSD took
Mach's VM of about 1990 (the Mach 2.5 generation; Lite2's files carry
CMU's 1987-91 notices). Mach went on in two lines. CMU's: Mach 3.0
(MK83, 1993; read here from `donors/mach3`), Utah's Mach 4 (1993-96),
and xMach (about 2000, Mach 4 with the Lites server; read here from
github.com/neozeed/xMach, whose VM is Mach 4's). OSF's: OSF MK through
7.3 (MkLinux DR3, 1998). OSF MK 7.3 is in the maintainer's repository
github.com/nmartin0/osfmk7.3, which this workspace cannot read until the
maintainer's GitHub account is linked; nothing is said of it here.
Apple's later descendant (XNU) is under the APSL and is not a donor.

| | Lite2 | Mach 3.0 (CMU) | Mach 4 / xMach (Utah) |
|---|---|---|---|
| VM lines (`vm/`) | 13,765 | 20,800 | 18,113 |
| copy strategies (symmetric copy-on-write) | none | yes (1992, rpd from dlb) | yes |
| `vm_map_copy` (for IPC) | 13 uses | 332 | 256 |
| external memory-object interface | none | 137 uses | 82 |
| shadow-chain bypass in collapse | yes (3 uses) | yes, switchable (7) | yes (6) |
| clustered paging | yes (swap pager, 41 uses) | 1 use | 1 use |

So the Mach lines continued the same VM, but most of their growth is the
microkernel's: memory objects reached by ports, and map copies for
message passing. Those are of no use in a monolithic kernel, which is
why 4.4 left them out. What carries over is small and real: Mach 3.0's
symmetric copy-on-write (which avoids most copy objects) and its
tunable collapse and object-cache limits. Each would be adapted to
Lite2's pager interface: a continuation, with the adaptation recorded.
4.4 also has things the Mach lines do not, such as clustered swap
paging.

**The flaws of 4.4's VM, as measured.** Chuck Cranor's UVM paper (USENIX
1999) measured NetBSD 1.3's VM -- Lite2's with fixes -- against UVM on
the i386:

- page faults: `ls /` 59 against 33, `cc` of hello-world 1,086 against
  590, `man csh` 114 against 64;
- map, fault and unmap one page: private file read 48 against 22
  microseconds, zero-fill 60 against 48;
- map entries: multi-user boot 400 against 242 (fragmentation from
  wiring; the kernel's map-entry pool is fixed, and running out panics);
- under memory pressure BSD VM became unresponsive where UVM slowed (by
  user report);
- structural: shadow chains keep unreachable pages until swap fills
  ("swap memory leaks"), and collapse can repair them only after the
  fact; at most 100 unreferenced objects are cached, whatever memory is
  free; mapping a file allocates an object, a pager and a vnode pager;
  page-in is one page at a time.

Which of these can be fixed inside 4.4's design, by continuations:

- **Fault-ahead** (UVM maps four pages ahead and three behind on each
  fault, which accounts for most of the fault counts above): an
  algorithm in `vm_fault`, addable to Lite2's.
- **Clustered page-in, the object cache limit, collapse and bypass,
  map-entry fragmentation from wiring:** FreeBSD 2.0-2.1 (before the
  1995-07-13 overhaul) and Mach 3.0 worked on each within the old
  design.
- **Copy objects:** Mach 3.0's symmetric copy-on-write removes most of
  them.
- **Not fixable without a redesign:** the shadow-chain model itself, and
  so its swap leaks and its cost on deep fork chains. That model is what
  UVM (amaps and anons) and FreeBSD's 1995 overhaul (typed objects, no
  copy objects) each replaced.

These are performance and robustness costs under heavy fork, mmap and
paging loads. None of them affects what POSIX requires.

**Is UVM more portable than the Mach VM?** No. Both put everything
machine-dependent behind the pmap interface, which Mach's VM was
designed around (Rashid and others, 1987). UVM kept that interface and
reused the BSD and Mach pmaps unchanged, and Cranor credits that reuse
for its easy porting to Alpha, i386, m68k, MIPS, SPARC and VAX. UVM's
pmap interface adds a few calls (`pmap_kenter_pa`, `pmap_kremove`,
`pmap_growkernel`, `pmap_update`), which help MP and performance, not
portability. For Lite2's ports, Mach-VM pmaps exist in NetBSD 1.3 and
OpenBSD 1996-2001 for hp300, sparc, vax and pmax; UVM pmaps exist for
the same ports from NetBSD 1.4. tahoe has neither.

**What keeping the Mach VM costs, with no translation layer.**

1. The fixes above are taken as continuations (FreeBSD 2.0-2.1, NetBSD
   1.3, OpenBSD to 2001-05, Mach 3.0): bounded work, no shim.
2. **The amd64 pmap, the MP pmap (TLB shootdown) and PAE are written for
   Lite2's pmap interface by us**, following OpenBSD's and NetBSD's as
   the reference. No donor's can be taken as it is, and without a
   translation layer each UVM call in them is rewritten to its Mach
   form (page-table pages from a kernel object, wired `pmap_enter` for
   `pmap_kenter_pa`, `vm_map_find` for `uvm_map`). That is own writing,
   kind C, about the size of one pmap per architecture.
3. The same holds for each later machine-dependent file or driver taken
   from OpenBSD that calls the VM: its few VM calls are written in Lite2's
   form as it is taken (`bus_dma`'s allocation, `pool`'s page back end,
   ACPI's mappings).
4. After 2001 no donor fixes this VM; the project maintains it alone.

**How bad, then.** For POSIX conformance, the i386 and the Lite2 ports:
not bad. Every piece has a donor, and the VM is the most 4.4 it can be.
The costs are (a) the measured performance gap under load, partly closed
by the in-design fixes and never wholly; and (b) one pmap per new
architecture written by us, plus the VM calls of each later
machine-dependent file adapted as it comes in, with no upstream after
2001. The alternative remains UVM where OpenBSD took it, which removes
(b) and most of (a) at the cost of a redesigned `vm/`.

Sources: Cranor and Parulkar, "The UVM Virtual Memory System", USENIX
1999 (usenix.org/legacy/event/usenix99/full_papers/cranor); Rashid et
al., "Machine-Independent Virtual Memory Management for Paged
Uniprocessor and Multiprocessor Architectures", 1987 (not re-read).

## 12. How far the 4.4 VM can be upgraded canonically, and what remains

Added 2026-10-29, at the maintainer's question: apply the fixes the BSDs
made to the Mach VM, then fill what is left from the Mach kernels' own
VM. How much of 4.4's VM is upgraded so, and what holes remain?

Read for it: Lite2's `vm/` (13,765 lines), NetBSD 1.3's (16,772),
OpenBSD's of 2001-05 (17,890, the last before UVM became compulsory),
FreeBSD 2.1's (16,729, the branch before the 1995-07-13 overhaul), CMU
Mach 3.0's (20,800) and Utah Mach 4's as xMach carries it (18,113).
OSF MK 7.3 could not be read (section 11). "Found" means the code is
there; how well it works was not tested.

| Weakness or need | BSD continuation (first) | Mach (then) | Remains |
|---|---|---|---|
| Too many page faults (no fault-ahead) | FreeBSD 2.1 `vm_fault_additional_pages`, in its Lite-derived `vm_fault.c`, plus a `pmap_prefault` call | -- | the pmap half written for Lite2's pmap |
| Page-in one page at a time | FreeBSD 2.1 getpages clustering (16 uses against Lite2's 12) | -- | -- |
| Copy objects in copy-on-write | -- | Mach 3.0 symmetric copy-on-write (1992), also in Mach 4 | adapted from Mach's memory-object attributes to Lite2's pagers |
| Shadow chains: collapse and bypass | NetBSD 1.3 and OpenBSD 2001 (bypass 9-11 uses against Lite2's 3); FreeBSD 2.1 dead-object handling | Mach 3.0's switchable collapse and bypass | **the shadow-chain model itself**: its swap leaks are reduced, not ended |
| Object cache fixed at 100 | tunable in FreeBSD 2.1 | Mach 3.0 `vm_object_cached_max` | the double caching of vnodes and objects, and their recycling conflict |
| Mapping with a protection is two steps (a race) | -- | Mach 3.0 `vm_map_enter` takes current and maximum protection and inheritance | -- |
| Physical memory in several segments | OpenBSD 2001 `vm_physseg` (Cranor, 1998) | -- | -- |
| Contiguous memory for `bus_dma` | NetBSD 1.3 `vm_page_alloc_memory` | -- | -- |
| Physical-address type for PAE | OpenBSD 2001 `paddr_t` (25 uses in its Mach VM) | -- | PAE in the pmap, written by us |
| Swap added and removed at run time | NetBSD 1.3 `vm_swap.c` and `swapctl` (Matthew Green, 1995-97) | -- | -- |
| Pageout aging | FreeBSD 2.1 `act_count` (53 uses) | Mach 3.0 (22) | FreeBSD's pageout file carries a Net/2 version (section 3.2) |
| Kernel memory zones | -- | Mach 3.0 `kern/zalloc.c` | the `pool(9)` interface OpenBSD's code calls |
| Multiprocessor VM locking | Lite2's locks are there but empty without MP | Mach 3.0's VM locks are real and ran on multiprocessors (Sequent, iPSC) | the pmap's TLB shootdown, written by us |
| 64-bit | NetBSD 1.1-1.3 ran the Mach VM on the Alpha | Mach 3.0 has an Alpha port | an amd64 pmap, written by us |
| Page loanout and transfer (zero-copy I/O) | none | Mach 3.0's map-copy page lists, built for message passing | **a hole**: no canonical source for a monolithic kernel |
| Pager structures per mapped file (object, pager, vnode pager, hash) | none before FreeBSD's overhaul | Mach's memory objects are heavier still | **structural; remains** |
| Map-entry fragmentation from wiring; the fixed kernel map-entry pool | not found in any donor before UVM | not found | **remains**; a bounded fix would be our own |
| Unified buffer and page cache | none before FreeBSD's VMIO and NetBSD's UBC | none | **remains**; POSIX does not need it (section 5) |

**The answer.** Of the weaknesses measured or listed against 4.4's VM,
about two-thirds have a canonical filler: the BSDs' Mach-VM work (NetBSD
1.3, OpenBSD to 2001, FreeBSD 2.0-2.1) and, after it, Mach 3.0's own
(symmetric copy-on-write, protections in `vm_map_enter`, zones, real
MP locks). Everything the targets *need* from the VM has a source,
except the machine-dependent parts that were never written for this VM:
the amd64 pmap, the TLB shootdown and PAE in the pmap. What remains
are the parts of the design itself: the shadow-chain model (its swap
leaks reduced, not ended), the per-file pager structures, the double
caching of objects and vnodes, map-entry fragmentation from wiring, and
zero-copy page loanout. Those are the parts that UVM and FreeBSD's
overhaul replaced, and keeping them is the cost of keeping 4.4's design.

**Order, if the Mach VM is chosen:** NetBSD 1.3's and OpenBSD's fixes
first (they apply to Lite2's files most directly), then FreeBSD 2.1's
changes in its Lite-derived files (`vm_fault.c`, `vm_object.c`,
`vm_map.c`; the Net/2-headed files only after the ruling of section
3.2), then Mach 3.0's pieces adapted to Lite2's pager interface. Each
Mach piece is taken from a sibling line, not a descendant of Lite2, so
each is recorded as an adaptation (kind B or D, as `imports.md` sets).

## 13. OSF MK's VM, and the parts of UVM that fit a Mach VM

Added 2026-10-29, at the maintainer's request: read OSF MK's VM as
MkLinux carried it, and find how much of UVM could fill the holes left
in section 12 without taking UVM's redesign.

**OSF MK, read.** Source: github.com/slp/osfmk-mklinux, "OSF Mach and
MkLinux conservation project" (Sergio Lopez), whose first commit
(`0f6f7e6`, 2011-01-04) is the MkLinux DR3 distribution as issued; later
commits there are his own and were not used. Its `vm/` is 32,379 lines.
Every file carries OSF's permissive notice ("Copyright 1991-1998 by Open
Software Foundation, Inc. Permission to use, copy, modify, and
distribute this software..."), with CMU's. The maintainer's own
`osfmk7.3` repository still cannot be read from here; whether DR3's
kernel is exactly OSF MK 7.3 was not checked against it.

Counted with the same patterns as section 12 (uses in `vm/`):

| | Lite2 | NetBSD 1.3 | OpenBSD 2001 | FreeBSD 2.1 | Mach 3.0 | Mach 4 | OSF MK |
|---|---|---|---|---|---|---|---|
| fault clustering, read-ahead | 0 | 0 | 0 | 3 | 0 | 0 | **63** |
| pageout clustering | 4 | 4 | 4 | 1 | 0 | 0 | **30** |
| swap existence maps (`vm_external`) | 0 | 0 | 0 | 0 | 71 | 61 | 110 |
| delayed and symmetric copy | 0 | 0 | 0 | 0 | 10 | 9 | **43** |
| collapse and bypass | 16 | 29 | 31 | 16 | 31 | 25 | 42 |
| contiguous physical allocator | 4 | 5 | 12 | 9 | 10 | 7 | **91** (`cpm_allocate`) |
| zones (map entries from a zone) | 0 | 0 | 0 | 0 | 38 | 37 | 24 (`vm_map_entry_zone`, `vm_map_kentry_zone`) |
| page-shortage throttling, free reserve | 0 | 2 | 2 | 19 | 14 | 14 | **49** |
| lock operations | 94 | 113 | 107 | 99 | 196 | 196 | **321** |

OSF MK is the furthest the Mach VM itself went before Apple's line. What
it adds, beyond Mach 3.0:

- **fault clustering**: page-in of a cluster of pages around a fault,
  sized per object (`cluster_size`);
- **clustered pageout**;
- **delayed copy**: copy-on-write deferred further than Mach 3.0's
  symmetric strategy;
- **a contiguous physical-memory allocator** (`cpm_allocate`);
- **map entries from zones**, which removes the fixed kernel map-entry
  pool;
- **throttling** under page shortage;
- **the most thorough VM locking of any line**, built for multiprocessors.

As with Mach 3.0, much of the remainder is the microkernel's (memory
objects reached through ports, map copies for messages); the items above
are what a monolithic kernel can use, each adapted to Lite2's pager
interface.

**UVM, file by file.** NetBSD 1.4's `uvm/` is 19,693 lines of C. Counting
each file's uses of the anonymous-memory redesign (`amap`, `anon`):

- **The redesign itself**: `uvm_amap.c` (1,073 lines, 328 uses),
  `uvm_anon.c` (363, 79) and most of `uvm_fault.c` (1,776, 215). These
  replace shadow chains and are not separable from them.
- **Partly tied to it**: `uvm_loan.c` (751, 89), `uvm_pdaemon.c` (1,111,
  53), `uvm_map.c` (2,956, 75), `uvm_page.c` (1,162, 18), `uvm_pager.c`
  (761, 11).
- **Nearly free of it** (four uses or fewer): `uvm_vnode.c` (2,077, 0),
  `uvm_swap.c` (1,968, 4), `uvm_aobj.c` (1,112, 4), `uvm_km.c` (1,070, 3),
  `uvm_device.c` (522, 3), `uvm_pglist.c` (292, 1), and the files that
  are Lite2's own under new names (`uvm_glue.c`, `uvm_unix.c`,
  `uvm_user.c`, `uvm_io.c`, `uvm_meter.c`, `uvm_init.c`, `uvm_mmap.c`).

So a little under three-fifths of UVM by lines does not depend on amaps
and anons. But much of that is Lite2's own code renamed, and NetBSD
1.3's swap and contiguous-allocation code that is already on the Mach
VM. What UVM adds, as pieces a Mach VM could take:

| Hole (section 12) | UVM piece | How it fits a Mach VM |
|---|---|---|
| Double caching of objects and vnodes; per-file pager structures | `uvm_vnode.c`: the VM object embedded in the vnode, no separate pager or hash (no amap use; it keeps Berkeley's 1991-93 and Utah's 1990 notices, being Lite2's vnode pager reworked) | the largest gain; a rework of Lite2's `vnode_pager.c` and the vnode's VM hook, adapted from `uvm_object` to `vm_object`. FreeBSD's overhaul made the same move (`v_object`) |
| Fixed kernel map-entry pool | `uvm_mapent_alloc`: kernel map entries allocated as needed | small; OSF MK's zones do the same, and come first in the Mach line |
| Map-entry fragmentation from wiring | wiring kept out of map entries where it need not be there (Cranor: four of five cases) | a technique in `vm_map`/`vslock`, our own writing following it |
| Too many faults | per-entry advice tables, `madvise`-driven read-ahead and read-behind (`nback`, `nforw`) | an algorithm; FreeBSD 2.1 and OSF MK have their own forms, which come first |
| Page-out efficiency | `uvm_pager.c` clustering of dirty pages for put | adaptable (11 amap uses); OSF MK's clustered pageout comes first |
| Zero-copy loanout | `uvm_loan.c`: loaning object pages to the kernel uses only a per-page loan count | **partly**: loans from objects to the kernel can be adapted; loans into anonymous memory need anons |
| The pmap interface the machine layers call | `pmap_kenter_pa`, `pmap_kremove`, `pmap_growkernel`, `pmap_update` | pmap-level, not VM-level: OpenBSD's 1999 `vm/pmap.h` already offered them beside Lite2's under `PMAP_NEW`. Adding them to Lite2's pmap interface removes part of the adaptation of every later OpenBSD machine file; it is the newer pmap interface, not a translation layer |
| The shadow-chain model | amaps and anons | **not separable**: taking them is taking UVM |

**What remains, after the BSDs, the Mach lines and UVM's separable
pieces.** Only two holes stay open:

1. **the shadow-chain model**, with its swap leaks reduced by collapse,
   bypass, delayed copy and existence maps, but not ended; and
2. **loans into anonymous memory**, which need anons.

Two things stay our own writing: the machine-dependent pmaps (amd64,
the TLB shootdown, PAE); and the adaptation of each piece taken from a
sibling line to Lite2's pager interface. The pmap work is smaller if the
pmap interface gains UVM's four additions.

**Order of taking**, by the donor rule and this project's preference for
the nearest line: the BSDs' Mach-VM work first (section 12); then OSF MK
and Mach 3.0 for what the BSDs lack (fault and pageout clustering,
delayed copy, zones and map-entry allocation, contiguous allocation,
throttling, locking); then UVM's separable pieces where the Mach lines
have none (the vnode-embedded object, loans to the kernel, the wiring
technique, the pmap additions). Each is recorded as an adaptation from a
sibling line.
