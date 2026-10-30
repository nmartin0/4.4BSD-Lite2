# 4.6BSD: what it is for, and how it is built

Decided by the maintainer, 2026-10-29. This file records the goal and
the design rules; `docs/roadmap-4.6.md` holds the measured work items,
`docs/roadmap-audit.md` the audit that comes before the cut. Where this
file and an older line of roadmap-4.6.md disagree, this file is the
later decision. `docs/plan-4.6.md` (2026-10-30) is the plan of work from
here, in stages, with the decisions each stage waits on.
`docs/guide-4.6.md` (2026-10-30) is the design guide:
the base, the decisions, where each piece comes from, the route to each
feature modern systems share, the research worth using, and what of
4.4BSD's traditional way stays. `docs/analysis-4.6-lineage.md` (2026-10-29) traces each
layer's lineage to the goal and finds where lineages cannot be
reconciled; its section 9 lists corrections to this file that await
the maintainer's decision.

## 1. The goal

OpenBSD is known for security, NetBSD for portability, FreeBSD for its
i386 depth. 4.6BSD's one defining goal is **POSIX**: to be a premier
system for developing the POSIX standard and for hosting POSIX
applications, continuing an aim 4.4BSD itself had. It conforms to:

- **POSIX.1-2024** (Issue 8), base and XSI, with **C17** (the C
  standard POSIX.1-2024 requires);
- the **System V ABI** (the generic ABI and the processor supplements
  for each port);
- the **errata** published after those (Austin Group interpretations
  and technical corrigenda).

It is to be a fully usable system, as the other BSDs are: it boots on
real, current hardware, builds itself, runs networks, and hosts
ordinary software.

## 2. The rules every change follows

1. **Deviate from 4.4BSD-Lite2 as little as possible.** Lite2's code
   and design are the default. Code is changed only where it needs
   upgrading, and then by the smallest canonical piece of a donor,
   shrink-wrapped on (the same rules as 4.5BSD: Lite2 first, then the
   clear files of 386BSD 0.0/0.1, then the clear parts of the other
   BSDs, then Mach, then Minix, then our own writing; the clearance
   ledger; settlement-encumbered text never used, nor what the other
   BSDs carried forward from it). **The other BSDs are searched in this
   order, always (decided 2026-10-29): OpenBSD, NetBSD, FreeBSD; then
   MirOS BSD; then DragonFly BSD; then any other permissive BSD.** What
   is taken is a continuation of a 4.4BSD subsystem as a daughter
   carried it on -- an addition or fix to the 4.4 design -- and not a
   redesign; `analysis-4.6-continuations.md` traces each subsystem's
   continuation to its plateau.
1a. **The most canonical fit is the best fit, not the earliest
   precedent** (decided by the maintainer, 2026-10-30). Until then
   donors were chosen by which BSD first fixed a thing, on the
   assumption that the first fix would change 4.4BSD-Lite2's code least;
   that was a proxy. What is chosen is what preserves Lite2's code best.

   **This is 4.6BSD's rule** (decided by the maintainer, later on
   2026-10-30). 4.5BSD, a restoration, keeps the first canonical fit:
   the earliest continuation that closes the gap, in the source order,
   with its own line's later fixes for its holes (`plan-4.5.md`, *The
   two rules*). 4.6BSD judges 4.5BSD's choices again by this rule when
   it touches their files.

   **What best fit means.** For any candidate, from any donor at any
   date:

   - **Smallest change to Lite2's code:** fewest lines touched, most of
     Lite2's text and structure left intact.
   - **Same semantics as 4.4:** fits how 4.4 already does things (its
     idioms, its interfaces, its locking model) rather than just
     compiling against it. A small diff that quietly changes behaviour
     is a worse fit than a slightly larger one that doesn't.
   - **Complete:** includes the donor's later corrections to the same
     code.
   - **Fewest foreign dependencies:** counted as interfaces it needs
     that Lite2 and our chosen pieces don't have.
   - **Clean provenance:** passes the clearance ledger and the 386BSD
     rule.
   - **Still a continuation, not a redesign:** that test stays exactly as
     it is.

   **How this changes the method.** The old rule becomes a search order,
   not a selection rule:

   - Start looking at the oldest continuation, since that's where good
     fits usually are.
   - Then check the same code forward through every donor, up to its
     plateau.
   - Pick by fit.
   - Donor order (OpenBSD, NetBSD, FreeBSD, MirOS, DragonFly, others)
     only breaks ties.

   **Breaking a tie.** Totally equivalent candidates are unlikely; when
   two or more fit about equally, the tie goes first to **the candidate
   whose maintained code is closest to modern before reaching its
   lineage's redesign plateau** -- the one whose line was carried
   furthest forward, with the most later corrections, while still a
   continuation of 4.4. Only if that too is equal does donor order
   decide.

   Lite2 itself still comes first in every search, and the source order
   above (Lite2, the clear 386BSD files, the clear parts of the other
   BSDs, Mach, Minix, our own writing) and the clearance rules are
   unchanged: the best-fit rule chooses among the candidates they
   admit. Every commit states the candidates measured, how each scored
   against the six points, and why the chosen one won.
2. **What gets in:**
   - what POSIX.1-2024, C17, the System V ABI or the errata require;
   - what makes the system faster, or able to run on current
     hardware, *without changing what a POSIX program sees*: SMP and
     its locking, drivers, modern buses and firmware, scheduler and
     VM improvements. These serve POSIX by making it fast and usable;
   - what Lite2 already has, including its BSD features outside POSIX
     (sockets and BSD networking, the VM system, FFS).
3. **What stays out:** new user-visible features that are neither POSIX
   nor Lite2's (the other BSDs' security frameworks, jails, new
   filesystems as features, package systems, feature layers), unless
   the maintainer decides one in.
4. **Removal is allowed.** Kernel functionality and tools whose
   underlying technology is no longer used may be deprecated and
   removed (candidates for a first look: the ISO/OSI, XNS and CCITT
   X.25 protocol families; Kerberos IV; tools for hardware or services
   that no longer exist). Each removal has a recorded reason and is
   checked against how Lite2 uses the code first.
5. **Portable, with as few machine-dependent files as can be had
   without contriving it.** Machine-dependent code exists only where
   the hardware forces it. Drivers are machine-independent and reach
   hardware through `bus_space`/`bus_dma`; libc prefers portable C to
   per-port assembly; the compiler's own facilities (C17 atomics and
   the like) are used rather than per-port code. i386 and amd64 are
   separate ports, as in OpenBSD (decided 2026-10-30, `guide-4.6.md`
   section 2); this replaces "share their common x86 code, as NetBSD's
   `sys/arch/x86` does".
6. **Generic C, with as few compiler flags as possible** (section 7).

## 3. Before the cut: the 4.5BSD floor

4.5BSD is not completed as its own release plan. Before 4.6BSD is
cut from it (decided 2026-10-29):

- the encumbrance check of `vfs_bio.c`, `execve`, `kern_acct.c` and
  `ptrace`, and a clean-source fix where it holds
  (`roadmap-audit.md`, A2);
- the missing donor holders and licences (A3);
- the maintainer's copyright line on our own files (A4);
- a record of where 4.5BSD was left off: what is done, what is known to
  be incomplete, and the audit findings not acted on.

The rest of the audit's replay (Part B) becomes a standing rule in
4.6BSD: when a file is touched, its earlier decisions are replayed with
the worksheet. The 4.5BSD items that were still to come move to
4.6BSD, among them the tests that keep the system stable (the boot
test, the system building itself) and the ports tree.

## 4. Test-driven conformance

The maintainer's own suite (posixcheck, github.com/nmartin0/
conformance_suite, about 8,600 assertions over XBD, XSH and XCU, plus
about 3,350 generated header checks) is the guide: run it, group the
failures by cause, fix each cause with the minimal canonical fit, run
it again. A System V ABI suite and a suite for what Unix-like systems
standardly have beyond POSIX are to run the same way.

- **The suite is proprietary and is never put in this tree**, not even
  under `contrib/`. The test harness copies it into the guest's disk
  image for a run and removes it afterwards.
- One command builds the system, puts the suite on the guest, boots
  QEMU, runs it, brings `run.log` back, and diffs it with the last run
  (`analysis/analyze.py diff`). A `known/` list for 4.6BSD holds what
  it fails today and is the backlog.
- Every patch states the score it moved and that nothing regressed.
- The standard is the oracle: the system is fixed to the text, never
  to a test; a wrong test is fixed in the suite, separately.
- The usable-system checks run beside the suite: boot, multi-user,
  network, the system building itself.

First blockers, measured from the suite's driver: it needs a C17
compiler (GCC 2.95.3 cannot build the harness or most tests), the
system's `sh` must pass the suite's 17 trust checks or the run aborts,
and many tests need pthreads.

## 5. Booting on modern hardware (research, 2026-10-29)

Lite2's kernel framework is not the obstacle. Current NetBSD and
OpenBSD still carry Lite2's `subr_autoconf.c` (Torek's, LBL, header
unchanged: "subr_autoconf.c,v 1.12 93/02/01 torek"), OpenBSD's
`vfs_bio.c` still has the Berkeley 8.6 line, OpenBSD keeps the `spl`
model on multiprocessors, and FreeBSD's VM is Lite2's Mach VM evolved.
What is missing is machine-level layers and drivers, which the donors
added in stages.

**The i386 cannot reach modern hardware.** Intel set its platforms to
UEFI Class 3 (no legacy BIOS or CSM) by 2020; current PCs boot only
through 64-bit UEFI. OpenBSD never gave the i386 a UEFI loader; NetBSD's
`bootia32` is for rare 32-bit firmware. (Whether any BSD's 64-bit UEFI
loader can start an i386 kernel is not verified.) Intel's x86S plan to
drop 32-bit modes was cancelled in December 2024, so the processors keep
them. So: **amd64 is the modern port**; the i386 stays for legacy
machines and QEMU. amd64 also brings a 64-bit `time_t` (the i386's
32-bit one ends in 2038).

The path, in the order the donors took it, oldest canonical version of
each:

1. Torek's autoconf on the i386 (NetBSD 1.0, 1994, had done it; already
   the planned item).
2. `bus_space`/`bus_dma` (NetBSD, merged into OpenBSD April 1996).
3. PCI (`pci_mapreg_*` 1999 in OpenBSD, from NetBSD).
4. amd64 (NetBSD about 2001-02; OpenBSD from NetBSD, January 2004), with
   shared x86 code.
5. SMP, APIC and ACPI (FreeBSD SMP by 3.0, 1998; OpenBSD SMP June 2004
   and its own small ACPI June 2005, the likely minimal fit beside
   Intel's ACPICA).
6. UEFI loader and framebuffer console (OpenBSD, August-September 2015;
   FreeBSD amd64 UEFI by 10.0, 2014). Under UEFI there is no VGA text
   mode, so Lite2's `pccons` shows nothing.
7. AHCI, NVMe, xHCI (OpenBSD 2013-14; FreeBSD by 8.0 and 10.0).

Each step can be tested first in QEMU's `q35` machine with OVMF (UEFI)
and emulated NVMe and xHCI.

## 6. The driver interface: OpenBSD's drivers, at the plateau

Modern drivers come from OpenBSD, adapted; its driver interface stayed
closest to Lite2's. Measured 2026-10-29 on five current OpenBSD
drivers (`com`, `ahci`, `nvme`, `xhci`, `em`): 300 distinct kernel
names used 2,246 times. A count of names, not of behaviour.

- 38% of the uses are already Lite2's interfaces. Lite2's `struct
  device`, `cfdata` and `cfdriver` match OpenBSD's nearly field for
  field; OpenBSD (from NetBSD, April 1996) moved match/attach into a
  `cfattach` and added detach and activate.
- OpenBSD's driver interface stayed nearly fixed from about 2000 to
  2014 (`em` in 2003, 2010 and 2014 uses the same set:
  `bus_space`/`bus_dma`, the PCI calls, `timeout(9)`,
  `IFQ_DEQUEUE`/`IFQ_POLL`, `if_start`, `splnet`). The multiprocessor
  network interface (`ifq_*`, `if_rxr_*`, `mbuf_list`, `KERNEL_LOCK`)
  came in 2015-16, `kstat` in 2020.

**Decision: 4.6BSD's driver interface is the plateau, and not beyond
it**, to stay as close to 4.4BSD's designs as feasible:

- **Tier A, adopted:** the `cfattach` split with detach/activate;
  `bus_space`/`bus_dma`; the PCI core; `endian.h`; `timeout(9)`; the
  small kernel-library additions (`memcpy`, `memset`, `strlcpy`,
  `snprintf`, `KASSERT`, the C17 integer types). With it, about 80% of
  the sample's uses are met.
- **Tier B, with each subsystem:** OpenBSD's SCSI midlayer (AHCI, NVMe
  and USB disks attach as SCSI; Lite2's SCSI is Torek's sparc-only
  design), the USB core and its keyboard and mass-storage drivers,
  `ifmedia`. About 90%.
- **Tier C, with the SMP decision:** mutexes, reader/writer locks, the
  kernel lock, task queues. About 94%.
- **Not adopted:** the 2015+ multiprocessor network interface, `kstat`
  and sensors. A modern network driver is ported back to the
  plateau's `IF_DEQUEUE`/`if_start` form (about 40 lines of `em` today);
  statistics and sensor code is taken out of a driver as it is ported.
  Revisit only if the network stack goes multiprocessor.

Prerequisite under all of it: the i386 moved from Lite2's `isa_device`
tables to Torek's framework.

## 7. Generic C, few flags

The system is to compile as plain, standard C17 with as few compiler
flags as possible. A flag is one of two kinds:

- **Essential:** what an operating system needs and the standard does
  not cover: building freestanding, keeping floating point and vector
  registers out of the kernel, the kernel's code model and address,
  the ABI the System V supplement sets.
- **A crutch:** a flag that is there because the code is not standard
  enough: switching off strict aliasing, defining signed overflow,
  allowing common symbols, an older or GNU dialect, silenced warnings,
  switched-off optimizations that only non-conforming code needs.

The work: survey the kernel and userland flags of FreeBSD, NetBSD and
OpenBSD (and Lite2's own), classify each one, keep the essential ones,
and remove each crutch by fixing the code it covers for, so that the
system's behaviour does not depend on the compiler being told to
tolerate it. Behaviour a flag only makes explicit is written in the
code instead. Already known: the 101 kernel symbols defined in more
than one object that `-fno-common` would reject (roadmap-4.6.md).

## 8. From C89 to C17

The kernel and the shipped system tools are to be C17, not C89 (decided
2026-10-29). Third-party code in `contrib/` stays as it is.

C89 code is mostly valid C17 already. C99 removed implicit `int` and
calls to undeclared functions; old-style (K&R) function definitions
are still legal in C17 but obsolescent, and C23 removes them. So the
change is a few large mechanical passes and a smaller residue of real
fixes. Measured 2026-10-29: the kernel alone has 4,360 uses of
`__P(())` and 759 `.c` files with K&R definitions; userland is several
times that.

**The tools, all on the build host:**

- **The compiler as the inspector:** `-std=c17 -pedantic-errors` with
  the warnings for old-style definitions and missing prototypes; a
  script turns its output into a tally by file and kind of error.
- **`cproto`** converts K&R definitions to prototypes in place (GCC's
  old `protoize` as a fallback).
- **`sed`** removes `__P` (`__P((int, char *))` to `(int, char *)`);
  `<sys/cdefs.h>` keeps the macro for code outside the tree.
- **Coccinelle (`spatch`)** for changes that need the C to be
  understood (`bcopy`/`bzero` to `memcpy`/`memset`, GNU extensions to
  standard C).
- **`unifdef`** removes the dead `#ifdef __STDC__` branches.

**The safety check: compare the object code.** A purely syntactic pass
must give byte-identical object files with the same compiler. The
pass script compiles every file before and after and compares the
machine code; identical files need no review. Those that differ are
the ones to look at, usually a parameter declared `char`, `short` or
`float` that K&R widened and a prototype does not.

**The order:**

1. The C17 compiler first: it is the target and the checker.
2. The inventory: everything built with `-std=c17 -pedantic-errors`,
   and the tally.
3. The mechanical passes, each its own patch and each object-compared:
   `__P` removal; K&R to prototypes; missing `#include`s and
   declarations, as the compiler lists them.
4. The residue by hand, grouped by kind: type punning, signed
   overflow, GNU-only syntax (zero-length arrays, statement
   expressions), inline-assembly syntax. This is the same work as
   removing the crutch flags (section 7).
5. After every pass, the boot test and the system building itself;
   once the harness exists, the conformance suite.

The heavy work runs in the shell, not in Claude's context: scripts,
summaries, and the residue by hand. The K&R pass touches nearly every
function, which widens the diff against Lite2; it is recorded as one
reproducible sweep, with its script in the tree, so the result can be
regenerated from Lite2. The other BSDs made the same sweep (each
removed `__P`, by memory in the early 2000s; to be checked in their
histories before it is cited).

## 9. Order of work

1. The 4.5BSD floor, the record of where 4.5BSD was left, the cut.
2. A C17 compiler, first as a cross-compiler on the build host.
3. The C17 sweep (section 8), the kernel first, then the system tools.
4. `sh` passing the suite's trust checks.
5. The conformance harness and the first baseline.
6. The failures, grouped by cause, cheapest and widest first; threads
   and SMP decided together after a survey of the precedents back to
   the earliest.
7. Alongside: the compiler-flag survey and its removals; Torek's
   autoconf on the i386 and driver Tier A; then amd64, ACPI/APIC/SMP,
   UEFI and the framebuffer console, and Tier B drivers, each tested
   in QEMU `q35` with OVMF.

## Open questions

- Whether a kernel debugger belongs in 4.6BSD, or Lite2's `kgdb` stub
  is revived instead.
- The Kerberos policy.
- The licence for our own files.
- Which compiler: a modern GCC (GPLv3) or Clang/LLVM (Apache 2.0 with
  the LLVM exception).
