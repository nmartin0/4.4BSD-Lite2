# 4.6BSD design guide

Recorded 2026-10-30 at the maintainer's request: everything the
research of 2026-10-29 and 2026-10-30 found, gathered as the design
guide for building 4.6BSD. With `design-4.6.md` (the goal and the rules)
and `roadmap-4.6.md` (the measured work items, *The continuation
roadmap*), it is the plan.

The evidence for it is in:

- `analysis-4.6-lineage.md`: the first lineage pass, corrected by the
  second;
- `analysis-4.6-continuations.md`: each subsystem traced to its plateau,
  how the pieces fit, the VM in sections 11-15, and section 16;
- `analysis-md-minimality.md`: machine-dependent code per port.

Claims checked against source are stated plainly. What rests on general
knowledge of the systems is marked **(not re-checked)**.

**Note on the sources named here.** They were chosen by the method in
use until 2026-10-30, the oldest donor that continues 4.4. That
method is replaced by the best-fit rule (`design-4.6.md` rule 1a):
each source named below is a starting point for the search, to be
confirmed or replaced by fit when its work is taken up.

## 1. The base: 4.4BSD-Lite2, not a daughter

Decided 2026-10-30. 4.6BSD starts from 4.4BSD-Lite2, as 4.5BSD carried
it. It does not start from an OpenBSD of 2004-2014 with features added,
even though more of the work would then be done.

Starting from OpenBSD 2004-2014 would decide, before anything else:

- **The VM, as UVM.** OpenBSD's i386 went UVM-only on 2001-05-05, which
  closes the Mach-VM path.
- **The i386, as NetBSD's of 386BSD and Net/2 lineage.** It would not be
  Berkeley's own 4.4 port (section 16 of the continuations analysis: 20
  of OpenBSD 2.6's Berkeley-versioned i386 files at Net/2's 7.x, against
  Lite2's 76 at 8.x).
- **OpenBSD's own agenda, to be subtracted.** This includes pf, IPsec,
  the mandatory mitigations, privilege separation, its emulations and its
  removals. OpenBSD in 2004 kept 63% of Lite2's machine-independent
  lines, and in 2014 42%.

**What makes 4.6BSD its own system**, against OpenBSD of 2004-2014, in
the kernel:

1. 4.4BSD's trunk kept wherever nothing forces a change: process
   management, signals, VFS, FFS, NFS, sockets, TCP/IP, the tty layer.
2. The VM: 4.4's Mach VM continued (if the VM decision keeps it), not
   UVM.
3. Berkeley's i386, not the 386BSD/Net/2 one.
4. Complete POSIX.1-2024: timers, queued realtime signals over a 128-bit
   signal set, kernel AIO, `clock_nanosleep`, `O_SEARCH`, `O_EXEC`,
   `tcgetwinsize`. None of these is in OpenBSD.
5. Lines other than OpenBSD's where they are the continuation:
   - NetBSD's for signals, timers, exec;
   - FreeBSD's for AIO and the tty's cblocks;
   - Mach's and OSF's for the VM.
6. None of OpenBSD's own agenda, unless decided in.
7. Lite2's ports: hp300, luna68k, news3400, pmax, sparc, tahoe, vax,
   with the i386, and amd64 added.
8. C17 with few compiler flags; the System V ABI; machine-dependent code
   held to Lite2's minimum per port.
9. Possibly 4.4's own LFS for crash safety.

Where they will look alike, said plainly: SMP and locking, threads, the
VFS and network additions, drivers, ACPI and UEFI. That is the layer
only OpenBSD continued without redesigning, and 4.6BSD takes from it
what is needed, not all of it.

## 2. Decisions recorded 2026-10-30

- **i386 and amd64 are separate ports**, not sharing an `x86` directory,
  as in OpenBSD. Supersedes `design-4.6.md` rule 5's "as NetBSD's
  `sys/arch/x86` does". It drops NetBSD's 2002 interrupt redesign and
  the ACPICA conflict from the x86 question. amd64's continuation is
  OpenBSD's of 2004, which is NetBSD's amd64 code of 2003.
- **The boot blocks.** FreeBSD 4.1's boot1, boot2 and BTX stay for BIOS
  booting until UEFI booting is taken up. Then OpenBSD's BIOS loader and
  `efiboot` (2015), which share code and both pass `bootarg` (OpenBSD,
  1997), replace them. That gives one boot lineage and one argument
  protocol. The `bootarg` list continues Lite2's `howto, bootdev, ...,
  esym` and carries what UEFI needs: the memory map, the framebuffer,
  the ACPI pointer.
- **`pool(9)` for the kernel's object allocation, not Mach's `zalloc`.**
  Every OpenBSD and NetBSD subsystem and driver taken calls `pool_get`
  and `pool_put`; `zalloc` would need those calls rewritten or a shim.
  The VM may still allocate its own map entries from Mach-style zones
  inside itself, as OSF MK does.
- **4.4BSD-Lite2 is the base** (section 1).
- **The best-fit rule** replaces the earliest-donor method (`design-4.6.md`
  rule 1a), with its tie-breaker: closest to modern before the plateau,
  then donor order.

## 3. Where each piece comes from, by system

"Direct" is taken from that system itself; "via" reaches 4.6BSD through
another tree. Each is a starting point for the best-fit search.

**4.4BSD-Lite2 (the trunk):**

- process management, signals, credentials, the 4.4 scheduler's algorithm;
- VFS, FFS, NFSv3, specfs, fifofs, the buffer cache;
- sockets, TCP/IP, BPF;
- the tty layer;
- the Mach-derived VM (if kept), LFS;
- Berkeley's i386 and the hp300, sparc, vax, pmax, luna68k, news3400 and
  tahoe ports;
- kgdb, ktrace.

**CMU Mach and OSF MK** (direct, if the Mach VM is kept; permissive
CMU and OSF notices):

- symmetric and delayed copy-on-write, `vm_map_enter` with protections;
- fault and pageout clustering, the contiguous allocator, throttling, zones;
- real VM locks;
- ddb, which is CMU's (reaching us via NetBSD).

**NetBSD** (direct):

- the exec framework and ELF with the auxiliary vector (1.0-1.1);
- `sigset_t` widened to 128 bits (1.4);
- the `ksiginfo` queue and POSIX timers (2.0), `sigqueue` (6);
- `clock_nanosleep` (7, as reference);
- Mach-VM fixes, `vm_physseg`, `swapctl` (1.3);
- `sockatmark` (2001), `tcgetwinsize` (2017), `O_SEARCH`;
- Mach-VM machine code for hp300, sparc, vax and pmax (1.3);
- LFS's continuation (1.5), if chosen.

**FreeBSD** (direct):

- the tty's cblocks (2.0);
- AIO (3.0, Dyson);
- `SIGEV_THREAD` in librt (7);
- the VM's changes before 1995-07-13 (2.0-2.1), subject to the ruling
  on its Net/2-headed files;
- `O_EXEC` (8), if not NetBSD's;
- the boot blocks until UEFI;
- library pieces already in 4.5BSD.

**Third parties and research** (direct, or via the BSD that carries
them):

- KAME IPv6 (WIDE Project, BSD licence);
- SACK, NewReno and FACK (the Daedalus group, UC Berkeley; OpenBSD
  1998-11-17 "adapted from code for BSDI by Hari Balakrishnan, Tom
  Henderson and Venkat Padmanabhan");
- soft updates (McKusick, from Ganger and Patt, University of Michigan),
  if chosen;
- superpage reservations (Rice), as a design;
- NTP clock discipline (David Mills), optional.

**Via OpenBSD, written elsewhere:**

- the amd64 port, the MP code, `bus_dma` and `pool`: NetBSD's;
- kqueue (Jonathan Lemon) and timecounters (Poul-Henning Kamp): FreeBSD
  authors;
- dirhash: FreeBSD's.

**OpenBSD's own:**

- the SMP big lock and `mtx` at IPL; the per-CPU form of the 4.4
  scheduler;
- rthreads, futex;
- FFS2, the `*at()` calls, the 2013 64-bit ABI change;
- the network stack's fixes to 2014-06;
- the driver interface and modern drivers, its own ACPI;
- `efiboot`/`bootarg`, ld.so, TLS;
- `pselect`/`ppoll`, `pipe2`/`dup3`/`accept4`, `getentropy`, `waitid`.

**DragonFly, MirOS, Bitrig:** nothing. DragonFly redesigned from 2003;
Bitrig added nothing beyond OpenBSD; MirOS could not be read from the
workspace.

## 4. What every modern system has, and 4.6BSD's route to it

Across FreeBSD, NetBSD, OpenBSD, DragonFly, Linux, illumos and macOS
**(not re-checked)**. Each line names the route to the feature that
sits on 4.4's semantics; licences and origins were checked in the donor
histories.

**In all of them; needed:**

| Feature | 4.6BSD's route | On 4.4's semantics because |
|---|---|---|
| SMP, per-CPU scheduling | OpenBSD 2004 big lock; per-CPU run queues 2009 | spl, `tsleep`, 4.4's scheduler kept |
| 1:1 threads, a futex | OpenBSD rthreads (2005); `futex` (Pieuchot, 2016-17, ISC) | threads are processes; futex is `tsleep`/`wakeup` keyed by user address |
| Demand paging, copy-on-write, `mmap`, NX, large memory | the VM decision; NX by honouring `VM_PROT_EXECUTE` in the pmap; `paddr_t` (OpenBSD's Mach VM) | 4.4's VM already carries execute permission |
| Slab-style allocator with per-CPU caches | `pool(9)` (NetBSD 1997, Thorpe; OpenBSD 1999); per-CPU caches with SMP | a kernel-internal allocator beside `malloc(9)` |
| Precise monotonic clocks | timecounters (Kamp, "beer-ware" licence; OpenBSD 2004); user-space clock reading (OpenBSD 2020-07, Irofti) | `gettimeofday`/`clock_gettime` semantics kept |
| Scalable events | kqueue (Lemon 1999-2000, BSD licence; OpenBSD 2000-11-16, 23 files, +1,433) | knotes hang where `select` records waiters. **A decision**: outside POSIX |
| Modern hardware | OpenBSD's ACPI, APIC, PCIe, AHCI, NVMe, xHCI, UEFI | the driver plateau |
| IPv6, TCP with SACK | KAME; Berkeley's Daedalus SACK | Berkeley TCP extended |
| 64-bit time and offsets | OpenBSD's 2013 flag day | -- |
| Shared libraries, TLS | OpenBSD's ld.so (2000), TLS (2011, 2016) | -- |
| Kernel debugger | ddb (CMU) or Lite2's kgdb | open question |
| Dynamic tracing | OpenBSD `dt` (Pieuchot, 2020-01-21, ISC, 25 files, +1,658) beside `ktrace` | optional |
| Exploit mitigations | W^X from `VM_PROT_EXECUTE` and NX; ASLR by a random hint in `vm_map_findspace` (OpenBSD's from 2003) | 4.4's own VM semantics. **A decision**: OpenBSD's agenda |

**In most of them, but shown optional by one:**

- **A crash-consistent file system: needed in practice.** All but
  OpenBSD have one. Two in-lineage answers, a decision:
  - **Soft updates** (McKusick, from Ganger and Patt). OpenBSD carried
    them on the same Lite2-shaped FFS from 1998 to 2024 and removed
    them as too costly to maintain. Their 1998 licence carried a
    source-availability clause, which had gone by 2002 (plain BSD).
  - **LFS**, already in Lite2 (4,805 lines), crash-safe by construction.
    NetBSD's continuation (Konrad Schroder) reached 8,465 lines in
    NetBSD 1.5 with 8 references to the newer VM; by NetBSD 4 it was
    15,088 lines, tightly tied to UBC.
- **Not needed:**
  - a unified page cache (POSIX does not need it);
  - loadable modules (4.4 had none; OpenBSD removed them in 2014);
  - RCU or SMR (only with fine-grained locking; OpenBSD's SMR of 2019
    then);
  - tickless timers (an idea for later).

## 5. Research that 4.6BSD can use

**(Not re-checked)** except where noted. "Code" is a candidate to take;
"idea" is written by us following it.

- **Superpage reservations** (Navarro, Iyer, Druschel, Cox; Rice; OSDI
  2002). FreeBSD's `vm_reserv.c` (checked: "Copyright (c) 2002-2006 Rice
  University", BSD) is written to FreeBSD's VM; for us an idea, in our
  pmap and physical allocator.
  - The processor's TLB caches a few hundred to a few thousand address
    translations. With 4 KB pages, large programs miss constantly; a
    2 MB page covers 512 times as much.
  - On the first fault in a 2 MB-aligned region, the kernel reserves
    the whole physical block, fills it as it is touched, and promotes
    the mapping to one large page when all of it is present and alike.
  - It demotes on partial unmapping or reprotection, and breaks unused
    reservations under memory pressure.
- **Page colouring** (Kessler and Hill, 1992): an idea, small.
- **Page loanout** (Cranor, 1998): object-to-kernel loans fit a Mach VM.
- **RadixVM** (Clements, Kaashoek, Zeldovich; MIT; EuroSys 2013). Checked:
  MIT licence in sv6, about 4,060 lines of C++. An idea, only if the VM
  goes fine-grained for many processors.
- **The slab allocator** (Bonwick, 1994) and **magazines and vmem**
  (Bonwick and Adams, 2001): behind `pool` and per-CPU caches.
- **Hashed and hierarchical timing wheels** (Varghese and Lauck, 1987)
  and **timecounters** (Kamp, 2002): on the path already.
- **ULE** (Roberson, 2003): an idea; OpenBSD's per-CPU 4.4 scheduler is
  the continuation taken.
- **Lottery and stride scheduling** (Waldspurger, 1994-95): an idea, if
  POSIX's optional priority scheduling is ever done properly.
- **Scheduler activations** (Anderson and others, 1991): the lesson, not
  the method; NetBSD abandoned them in 5.0.
- **Journaled soft updates** (McKusick and Roberson, 2010) and **WAPBL**
  (NetBSD, 2008; checked: TNF licence, 67 lock calls of NetBSD 5's
  kind): ideas for crash safety.
- **SYN cache and cookies** (Lemon, 2002): inside the network plateau.
- **Receive livelock** (Mogul and Ramakrishnan, 1996-97): interrupt
  mitigation, partly inside the plateau.
- **CUBIC** (Ha, Rhee, Xu, 2008): an idea.
- **Event delivery** (Banga, Mogul, Druschel, 1999): kqueue's precursor.
- **The scalable commutativity rule** (Clements and others, SOSP 2013), a
  method for finding which POSIX interfaces can be implemented to scale,
  and its Commuter tool. For 4.6BSD: a test method beside the
  conformance suite, so that the implementation does not serialize
  where the standard does not require it.
- **Userland allocators:** jemalloc (Evans, 2006), phkmalloc (1995),
  OpenBSD's malloc.
- **Outside the rules unless decided in:**
  - Capsicum (Watson and others, 2010);
  - jails (Kamp and Watson, 2000);
  - netmap (Rizzo, 2012);
  - DTrace (CDDL, excluded by licence);
  - GEOM (a redesign);
  - `pledge`/`unveil`.

## 6. Three things explained

**LFS.** The log-structured file system: Rosenblum and Ousterhout's
1991 idea in Sprite, made into BSD-LFS by Margo Seltzer with Bostic,
McKusick and Staelin (USENIX 1993), shipped in 4.4BSD.

- Every change, data and metadata alike, is written sequentially into
  large segments at the end of a log; nothing is updated in place.
- Recovery after a crash finds the last checkpoint and rolls forward,
  so it is crash-safe by construction.
- Writes are fast. A cleaner must compact half-empty segments in the
  background, and files written piecemeal may read slowly.
- It has always been less mature than FFS; NetBSD is the daughter known
  to have continued it **(not re-checked for the others)**.

**`zalloc` and `pool`.**

- Both give fast allocation of fixed-size kernel objects.
- `zalloc` (Mach): a zone per type, simple, in 4.4's Mach heritage.
- `pool` (NetBSD 1997): safe from interrupts at a set priority level,
  with water marks and later per-CPU caches of constructed objects.
- The decision for `pool` is in section 2.

**Superpages:** section 5.

## 7. Traditional, and still valid

Sorted from Lite2's own source inventory. Limits checked in the tree;
the rest is general knowledge **(not re-checked)**.

**Kept as they are:**

- **Installation by hand.** Boot a miniroot, `disklabel`, `newfs`,
  unpack the sets, edit `/etc/fstab`, `MAKEDEV`, install the boot
  blocks, reboot, as "Installing and Operating 4.4BSD UNIX" (the SMM
  setup document) describes. At most, later, a small shell script that
  does the same steps in the open.
- **Startup and services:**
  - one `/etc/rc` with `/etc/rc.local` and `/etc/netstart`;
  - `/etc/ttys`, `init`, `getty`;
  - `syslogd`, `cron`, `inetd`;
  - single-user mode, `securelevel`, `shutdown`, `reboot`, `fastboot`.
- **Devices and disks:**
  - static device nodes made by `MAKEDEV`;
  - `disklabel` partitions, over MBR or GPT reading;
  - swap on partitions.
- **The kernel:**
  - a kernel built by `config(8)` from `GENERIC` or one's own file, with
    no loadable modules;
  - `sysctl`, `savecore`, `ktrace` and `kdump`;
  - the kvm-based `ps`, `vmstat`, `pstat`, `iostat`, `systat`.
- **File systems:** FFS with `fsck`, `dump` and `restore`, `tunefs`,
  quotas, `mtree`; NFSv3 with `amd` and `portmap`.
- **Building:** the whole system from `/usr/src` with BSD `make` and
  `share/mk`.
- **Documentation:** man pages in mdoc; the SMM, USD and PSD document
  sets in troff.
- **Accounts and accounting:** `master.passwd` with `pwd_mkdb`, `vipw`,
  `chpass`; process accounting (`accton`, `sa`, `lastcomm`).
- **Tools:**
  - `sh`, `csh`, `vi`/`ex`, `ed`;
  - `mail` with `mail.local` and `vacation`;
  - `calendar`, `leave`, `look`;
  - `lpd` and `lpr`, `tip`;
  - `ping`, `traceroute`, `route`, `routed`, `ifconfig`;
  - `termcap` and `curses`.

**Kept, with 1993's numbers raised.** Checked in Lite2:

- `UT_NAMESIZE` 8, `UT_LINESIZE` 8, `UT_HOSTSIZE` 16 (`<utmp.h>`);
- `MAXLOGNAME` 12;
- `ARG_MAX` 20,480 bytes;
- `CHILD_MAX` 40.

Each meets POSIX's minimum and is too small for modern names or modern
builds; the mechanism stays and the size grows. Also:

- `crypt(3)` keeps its interface, with a modern hash in place of Lite2's
  DES-based one (bcrypt, OpenBSD 1997, the BSD choice);
- `termcap` gains entries for modern terminals;
- 32-bit time, FFS1's timestamps and the 1 TB disk limit are already
  planned to change.

**Retired or replaced, because they conflict with use today:**

- Cleartext remote access: `rlogin`/`rlogind`, `rsh`/`rshd`, `rexecd`,
  `telnet`/`telnetd`, `ftpd`. SSH replaces them: OpenSSH, third-party
  under a BSD licence, in `contrib/`. The `ftp` client may stay.
- Kerberos IV (`kpasswdd`, its `libtelnet` parts): the open question.
- UUCP (`uucp`, `uucpd`); the XNS, ISO and X.25 families (`XNSrouted`,
  `trsp`); `tn3270`; SLIP (`slattach`, `sliplogin`, `startslip`).
- `timed`, replaced by an NTP client.
- Mail transport: the 1995 `sendmail`, to be replaced by a current one
  or a smaller one; local delivery stays.
- Legacy disk tools: `bad144`, `badsect`, `scsiformat`. Retired with
  care.
- Kept but off by default in `inetd.conf`: `fingerd`, `talkd`,
  `comsat`/`biff`, `rwhod`/`rwho`/`ruptime`.
- `portal`, `umap`, `union`, `kernfs`, `fdesc`, `procfs`: kept or dropped
  on use (`ps` needs none of them).

**Added beside the tradition, replacing nothing:** DHCP, NTP, SSH, IPv6,
shared libraries, a framebuffer console under UEFI, the ports tree.

## 8. Open decisions, gathered

From `roadmap-4.6.md` *Decisions still open*, updated:

1. The VM: Lite2's Mach VM continued, or UVM where OpenBSD took it.
2. The four Net/2-headed FreeBSD VM files.
3. ~~x86 layout~~: decided, separate (section 2).
4. The ports: kept up in source, or only tested ports supported.
5. Crash safety: soft updates on FFS2, or LFS brought forward.
6. kqueue.
7. Exploit mitigations (W^X, ASLR).
8. The mail transport; the NTP client; SSH in `contrib/`.
9. MirOS BSD's source, if the maintainer can supply it.
10. From `design-4.6.md`: ddb or kgdb, Kerberos, the licence for our own
    files, GCC or Clang.
