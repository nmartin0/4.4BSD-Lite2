# The plan of work: from here to 4.6BSD

Written 2026-10-30 at the maintainer's request, after the best-fit
sweep: one ordered plan from the tree as it stands (patch 239) to a
usable 4.6BSD. It draws together what the other documents decided and
found, and points to them for detail:

- `design-4.6.md`: the goal and the rules, including rule 1a, the best-fit rule;
- `guide-4.6.md`: the design guide (the base, the decisions, the sources
  by system, the route to each shared feature, the tradition kept);
- `roadmap-4.6.md`, *The continuation roadmap*: each subsystem's backports
  and plateaus;
- `roadmap-audit.md`: Part A and the replay worksheet;
- `audit-bestfit.md`: the sweep's findings;
- the four analyses (`analysis-*.md`).

Where this file and an older one disagree about order, this file is the
later decision. Nothing here overrides a rule.

## How every patch is made

Unchanged from 4.5BSD, with the best-fit rule added:

1. **The worksheet** (`roadmap-audit.md`, steps 1-8). Lite2 first. Then
   every candidate from the oldest continuation forward through each
   donor to its plateau, scored on the six points of rule 1a. Ties go
   first to the maintained code closest to modern before its plateau,
   then to donor order (OpenBSD, NetBSD, FreeBSD, MirOS, DragonFly,
   others). Own writing last.
2. **Clearance** before taking anything: the ledger row (stage 0.2), the
   386BSD rule, the encumbered text never used.
3. **The commit states** the candidates measured, how each scored, why
   the chosen one won, its kind (A-G, `provenance/imports.md`), and the
   licence and holders it carries.
4. **Tested** before it is sent: the build, the boot test in QEMU, and
   once stage 2 exists, the conformance suite (score before and after,
   no regressions).
5. **Measured** where it applies:
   - Lite2 retained in the machine-independent kernel (`build/` script,
     to be added in stage 2);
   - the machine-dependent core of each port against Lite2's figure;
   - compiler flags in use.
6. **Delivered** as a numbered patch with its copy-and-paste apply
   script, or a bundle when several go together.

## Decisions, and what each one holds back

| Decision | Holds back |
|---|---|
| **D1** the VM: Lite2's Mach VM continued, or UVM where OpenBSD took it (`analysis-4.6-continuations.md` 3, 11-15) | stage 5 and everything after it |
| **D2** the four FreeBSD VM files with Net/2 versions: tainted or not | FreeBSD 2.0-2.1 VM changes (5.1) |
| **D3** the compiler: a modern GCC or Clang/LLVM | stage 1 |
| **D4** the ports: all of Lite2's kept up in source, or only those tested | stage 8; the cost of every machine-independent change |
| **D5** crash safety: soft updates on FFS2, or LFS brought forward | 7.1 |
| **D6** kqueue | 7.5, and the ports tree |
| **D7** exploit mitigations (W^X, ASLR) | 5.2, 7.5 |
| **D8** mail transport, NTP client, SSH in `contrib/` | 7.2 |
| **D9** ddb or kgdb; Kerberos; the licence of our own files | 0.5 (licence), 7.5 |
| **D10** the sweep's twelve (`audit-bestfit.md`): `sh` getpwd; `expr`; the boot blocks; the FreeBSD disk driver and slice layer; `ptrace`; the stack limit; `ps_strings`; `microtime`; `_C_LABEL`; the ELF start-up files; `<stdint.h>` types | 0.4, and the stage that touches each file |
| **D11** MirOS BSD's source, if it can be supplied | nothing; it widens the search |

Decided already: 4.4BSD-Lite2 is the base; i386 and amd64 are separate
ports; FreeBSD's boot blocks stay until UEFI, then OpenBSD's loaders
and `bootarg`; `pool(9)`; the best-fit rule; the driver interface at
OpenBSD's 1996-2014 plateau; big-lock SMP first.

## Stage 0: the 4.5BSD floor, then the cut

The defects and the legal exposure first. Nothing in stage 1 begins
until 0.1-0.3 are done.

**0.1 The sweep's defects** (`audit-bestfit.md`), each by best fit, in
this order:

1. `copyout` with no user-address check, and the 386 path's two bugs.
   The fit to measure first is NetBSD 1.0's `copyout`; Lite2's own C
   `copyout` is the Lite2-first candidate for the 386. `copyin` is
   checked at the same time.
2. The ELF loader's short read (NetBSD 1.2's one line).
3. `bwrite`'s race, with NetBSD 1.1's corrections to `getblk` and
   `getnewbuf`.
4. `compress`'s decode overrun (CVE-2011-2895).
5. The rest, in order:
   - `deroff`; `if_ed` (FreeBSD 2.1.7); `uucpd`; `at`; `test`;
     `mkdir -p`; `sa`;
   - `re_comp`; `pstat`; ddb's ELF checks; `wcd`; the ELF address
     arithmetic; `strtoimax`; `uucplock`;
   - the two man pages.
6. My own two known bugs from 4.5BSD: the stray-interrupt counter of
   patch 222 and the `%ss:` overrides for `INTR_SPUR`.

**0.2 The clearance ledger (A1).** `provenance/clearance.md`, one row per
donor file in use: origin, lineage, version stamps, licence, holders,
verdict. It answers the sweep's open provenance questions: the floppy
DMA code (#373), FreeBSD 2.1.0's `wd.c` (#318), and decision D2's
FreeBSD VM files. `precedent.md` is brought to the donor order and the
best-fit rule.

**0.3 The encumbrance (A2).**

- **`execve`:** replaced by NetBSD's exec framework. The candidate is
  chosen by fit: NetBSD 1.0, 1.1, 1.2 and later, and OpenBSD's copies,
  each measured against Lite2's `kern_exec.c` stub and this tree.
  - It takes out the `getxfile` lines quoted from 4.4 (#211).
  - It brings the auxiliary vector with it.
  - Tested: init, ELF static binaries, `#!` scripts, set-user-ID,
    argument limits, `ps` strings, saved IDs.
- **Ledger rows** for `kern_acct.c`, `subr_rmap.c`, `kern_physio.c`,
  `sys_process.c` and `vfs_bio.c`, recording whether each donor is
  clear.

**0.4 The sweep's decisions (D10) that touch the floor:**

- the stack limit (`MAXSSIZ`);
- `ps_strings`;
- `ptrace`;
- `sh`'s `getpwd`;
- `expr`.

The others wait for the stage that touches their files.

**0.5 Holders, licences and our own files (A3, A4).**

- The missing donor notices (Hannum, the NetBSD Foundation, Nordier,
  Demetriou, the Regents/USL lines).
- The SPDX line on every file of ours.
- `COPYRIGHT` listing every holder.
- The six attribution corrections from the sweep, recorded in
  `imports.md`.

**0.6 The record of where 4.5BSD was left**:

- what is done, what is known to be incomplete;
- the audit findings not acted on;
- the 4.5BSD roadmap items carried into 4.6BSD.

**0.7 The cut.** A tag for 4.5BSD's last state, and 4.6BSD begins from
it (the branch name the maintainer's to choose).

Exit: no known defect from the sweep open, the ledger complete for
every donor file, `execve` clean, the boot test passing.

## Stage 1: the compiler and C17

1. **D3**, then a C17 cross-compiler on the build host, beside GCC
   2.95.3 until the switch.
2. **The flag survey:** Lite2's and the three donors' kernel and userland
   flags, each classed essential or crutch (`design-4.6.md` 7). Then
   `-fno-common`: the 101 symbols defined in more than one object.
3. **The System V i386 psABI's stack alignment,** read in the document,
   before the kernel's `-mpreferred-stack-boundary=2` is kept or
   dropped.
4. **The inventory:** everything built with `-std=c17 -pedantic-errors`
   and the old-style warnings, tallied by file and kind.
5. **The mechanical passes,** each its own patch, each proved by
   comparing object code:
   - `__P` removed (4,360 uses in the kernel);
   - K&R definitions to prototypes with `cproto` (759 kernel files);
   - missing includes and declarations.
6. **The residue by hand,** grouped: type punning, signed overflow,
   GNU-only syntax, inline assembly. Each crutch flag goes when the
   code it covered is fixed.

Exit: the kernel and the system tools build as C17 with only the
essential flags; the boot test and the system building itself pass.

## Stage 2: the tests that keep it stable

1. **`sh` passes the suite's 17 trust checks.** Each fix is by best
   fit: FreeBSD's and NetBSD's `sh` lines (the 4.5BSD notes in
   `deferred.md` list the first).
2. **The harness:** one command builds the system, puts the suite on the
   guest's disk image, boots QEMU, runs it, brings `run.log` back, and
   diffs it with the last run. The suite is never committed.
3. **The first baseline,** and a `known/` list of its failures as the
   backlog.
4. **The stability tests carried from 4.5BSD:**
   - the boot test, multi-user;
   - the system building itself;
   - the network.
5. **The measuring scripts in `build/`:** Lite2 retention,
   machine-dependent core per port, flags in use.

Exit: every later patch states its score and that nothing regressed.

## Stage 3: POSIX.1-2024 on the i386

Work that does not depend on the VM decision. Order within it follows
the suite's failures, cheapest and widest first, but these dependencies
hold:

1. **The ABI, once, before 4.6BSD binaries exist** (OpenBSD's 2013 flag
   day as the starting candidate):
   - 64-bit `time_t` (required: Austin Group bug 1462), `ino_t`;
   - `st_atim` and its fellows; `struct timespec`'s `tv_sec` and
     `tv_nsec`;
   - `getdents` and `d_off`;
   - the `utmp` sizes and `MAXLOGNAME`;
   - `ARG_MAX` and `CHILD_MAX` raised;
   - bcrypt in `crypt(3)`.
2. **The kernel stack off its fixed address** (NetBSD 1.1). It is the
   prerequisite of threads, and gives ddb `trace/t`.
3. **Threads:** OpenBSD rthreads (2005); `thrsleep`; the `struct process`
   split; the TCB calls; futex. Also libc's semaphores, barriers, spin
   locks and rwlocks.
4. **Time:** `clock_gettime` and `nanosleep`, `CLOCK_MONOTONIC`,
   timecounters, `timeout(9)`.
5. **Signals:**
   - the 128-bit `sigset_t`;
   - `siginfo`;
   - the `ksiginfo` queue and `sigtimedwait`;
   - `sigqueue` and `SIGRTMIN`-`SIGRTMAX`.
6. **POSIX timers** (NetBSD 2.0 to 4.0, by fit), `clock_nanosleep`,
   `SIGEV_THREAD` in libc.
7. **AIO** (FreeBSD 3.0 to 4.11, by fit), with `aio_fsync`.
8. **Files:**
   - `O_NOFOLLOW`;
   - the `*at()` calls, `utimensat`, `O_CLOEXEC`, `O_DIRECTORY`,
     `O_SEARCH`, `O_EXEC`, `O_CLOFORK`;
   - dirhash;
   - FFS2 with 64-bit block numbers (after item 1).
9. **Sockets:**
   - `poll`, `socklen_t`, `sockaddr_storage`;
   - `MSG_NOSIGNAL`;
   - `accept4`, `SOCK_CLOEXEC`, `pipe2`, `dup3`;
   - `sockatmark`.
10. **Terminals:** ptm, `posix_openpt` and its fellows, `tcgetwinsize`,
    through the cblock shim.
11. **The rest:** `pselect`/`ppoll`, `getentropy`, `waitid`,
    `posix_spawn`, SysV message queues and semaphores.
12. **The System V ABI:** ELF auxiliary vector, ld.so, TLS, shared
    libraries.
13. **libc and the utilities,** by the suite's failures.
14. **The option values** set to 202405L as each option is met.

Exit: all fourteen mandatory options met; the suite's remaining
failures are understood and listed.

## Stage 4: the machine layer and drivers on the i386

Still independent of the VM decision, except where a driver allocates
memory (written in Lite2's VM form).

1. **Torek's autoconfiguration with `cfattach` on the i386,** and
   run-time interrupt establishment in place of `config`'s generated
   `vector.s`.
2. **Driver Tier A:** `bus_space`, `bus_dma`, PCI, the small library
   additions.
3. **Storage:**
   - OpenBSD's `wdc`/`pciide`/`atapiscsi`, then the SCSI midlayer
     (Tier B), AHCI, NVMe, GPT in `readdisklabel`;
   - the FreeBSD slice layer and `wd`/`atapi`/`wcd` removed (D10's disk
     driver item).
4. **Network drivers at the 2014 plateau.** `if_ed` replaced or kept by
   fit.
5. **Console and USB:** wscons and rasops; the USB core, keyboard and
   mass storage.
6. **ACPI:** OpenBSD's own acpi(4).

Exit: the i386 boots QEMU's `q35` machine from AHCI and NVMe, with a
USB keyboard.

## Stage 5: the VM, and what rests on it

Begins with **D1** and **D2**.

1. **If the Mach VM is kept:**
   - the BSDs' fixes first (NetBSD 1.3, OpenBSD to 2001-05, FreeBSD
     2.0-2.1 under D2);
   - then Mach 3.0's and OSF MK's pieces;
   - then UVM's separable ones (the vnode-embedded object, loans to the
     kernel, the wiring technique, the pmap additions);
   - each rewritten in Lite2's short-function style.

   **If UVM:** the transition as OpenBSD made it, both builds kept until
   the switch.
2. **`paddr_t`, PAE, the no-execute bit,** and W^X and ASLR under D7.
3. **SMP on the i386:**
   - the local and I/O APICs and the MP table;
   - the kernel lock, `mtx` at IPL, rwlocks;
   - the per-CPU 4.4 scheduler;
   - TLB shootdown in the pmap.
4. **Per-CPU `pool` caches.** Superpage reservations as a design in the
   pmap and the physical allocator.

Exit: the i386 boots multiprocessor in QEMU and passes the suite under
SMP.

## Stage 6: amd64 and UEFI

1. **The amd64 port,** separate from the i386: OpenBSD's of 2004
   (NetBSD's code of 2003). Its pmap is written to the VM chosen in D1.
2. **amd64 SMP.**
3. **UEFI:** OpenBSD's `efiboot` and BIOS loader with `bootarg`, replacing
   BTX; the framebuffer console.
4. **The suite on amd64,** where 64-bit time is native.

Exit: amd64 boots QEMU `q35` with OVMF, multiprocessor, and passes the
suite; then real hardware.

## Stage 7: a usable system

1. **Crash safety (D5).**
2. **Network services:**
   - SSH (OpenSSH in `contrib/`), an NTP client, DHCP;
   - IPv6 (KAME), with `getaddrinfo`;
   - the mail transport (D8).
3. **The retirements** (`guide-4.6.md` 7), each with its recorded reason:
   - the cleartext remote services;
   - UUCP; the ISO, XNS and X.25 families; `tn3270`; SLIP; `timed`;
     Kerberos IV (D9);
   - the legacy disk tools;
   - the `inetd.conf` defaults.
4. **Event notification (D6), tracing, the debugger (D9).**
5. **The ports tree** (carried from 4.5BSD).
6. **Installation:** the manual procedure in the tradition of "Installing
   and Operating 4.4BSD UNIX", with install media for BIOS and UEFI.

Exit: 4.6BSD installs on real hardware, hosts itself, runs a network,
and passes the conformance suites (POSIX, then the System V ABI and the
beyond-POSIX suite).

## Stage 8: Lite2's other ports

Under D4: hp300, sparc, vax, pmax, luna68k, news3400 and tahoe brought
along with the machine-independent changes. Each uses NetBSD 1.3's
Mach-VM machine code where the Mach VM is kept, and is held to Lite2's
machine-dependent minimum. tahoe has no daughter port and is
the project's own throughout.

## What runs alongside every stage

- The replay of 4.5BSD's earlier decisions when a file is touched
  (`roadmap-audit.md` Part B), by the worksheet and the best-fit rule.
- The C17 and generic-flag rules for new code.
- The record: each finding, decision and measurement written into the
  tree as it is made.
