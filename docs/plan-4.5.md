# The plan of work: finishing 4.5BSD

Decided by the maintainer, 2026-10-30: 4.5BSD and 4.6BSD have
different goals, so they choose their sources by different rules,
and each has its own plan. This file is 4.5BSD's: what is left to
bring it to a stable release, and in what order. `plan-4.6.md` starts
from the 4.5BSD release.

## The two rules

**4.5BSD takes the first canonical fit.** 4.5BSD is a restoration: 4.4BSD-Lite2
as it should have been, completed the way its own descendants
completed it (`roadmap.md`). A donor is chosen as follows.

1. **Follow the source order**, `roadmap-audit.md`'s order applied
   strictly:
   - Lite2 itself;
   - the clear 386BSD 0.0/0.1 files;
   - the BSDs;
   - Mach and OSF;
   - Minix;
   - our own writing.

   A candidate from an earlier group always wins over one from a
   later group.
2. **Within the BSDs, survey every release of every donor.** That
   means OpenBSD, NetBSD and FreeBSD, then MirOS and DragonFly
   (`method.md` 3). The earliest release that closes the gap wins:
   - it must close the gap with 4.4's semantics;
   - it must be a continuation of 4.4's own code, not a redesign;
   - it must have clean provenance.
3. **Then take that same line's later fixes**, but only for holes it
   is shown to have. Each hole gets the first fix made for that hole.
   cron and at (patch 181) are the model. Fixing a defect is not a
   change of fit, so a hole is fixed whatever the date of its fix.
4. **Break a tie of date** by the smallest change to Lite2, then by
   donor order.

This is the rule 4.5BSD has always followed (`provenance/precedent.md`,
"stopping at the earliest release that has the thing"). It is now
stated as 4.5BSD's rule alone.

**4.6BSD takes the best canonical fit** (`design-4.6.md` rule 1a).
4.6BSD modernizes and continues the code, so it chooses on the six
points, with any donor at any date eligible. When 4.6BSD touches a
file 4.5BSD chose by first fit, 4.6BSD judges it again by best fit
(`plan-4.6.md`, stage 0).

The best-fit sweep (`audit-bestfit.md`) was made under 4.6BSD's rule.
Its findings are read again below under 4.5BSD's rule. Some stay
defects under either rule; some are 4.5BSD corrections; and some are
right for 4.5BSD and are left for 4.6BSD to judge again.

## What "stable" means here

4.5BSD is released when all of these hold:

- it installs from its CD on QEMU's `pc` machine;
- it boots multi-user;
- it builds itself;
- it runs its network services;
- it passes its regression tests;
- it carries no known security defect;
- it carries no text that cannot be used;
- every donor file has a clearance row;
- every holder is named.

The open work items in `roadmap.md` are divided as follows.

| Item in `roadmap.md` | Where |
|---|---|
| 5, the test framework | 4.5BSD (stage 5) |
| 6, the kernel's loose ends | 4.5BSD (stage 6) |
| 8, the release | 4.5BSD (stage 7) |
| 4, ports | standing item, for later (2026-10-30) |
| 7, Torek's autoconfiguration and PCI | standing item, for later (2026-10-30) |
| 9, X11R5 | standing item, for later (2026-10-30) |
| shared libraries | 4.6BSD |

The three standing items are not done in 4.5BSD, and are not tied to
either release's schedule. The maintainer takes them up when ready.

## Stage 1: the rules, recorded

Docs only. Rule 1a is marked as 4.6BSD's, and `roadmap-audit.md`'s
worksheet step 5 is split by release. `precedent.md` names its order
as 4.5BSD's rule (done with this plan, patch 241).

## Stage 2: the defects

Each defect is fixed by the first fit for its hole. The candidates are
listed in the order they are measured. The sweep's numbers are patch
numbers.

1. **`copyout` has no user-address check, and its 386 path has faults**
   (#126, #145): patch 243 (`provenance/copyout.md`; sealed only when
   the commit says so). `copyin` too. This comes first: a program can
   write kernel memory through `read()`.
   - The Lite2-first candidate is Lite2's own C `copyout` with its
     `USRSTACK` check and its 386 `user_write_fault`. Patch 126
     deleted it, and why is to be read in that commit.
   - The next candidate is NetBSD 1.0's `locore.s` `copyout`, with
     its `VM_MAXUSER_ADDRESS` check and 386 path.
   - `copyin` and the other user-copy routines are checked in the
     same patch.
   - Tested with a program that passes a kernel address to `read()`,
     on the 486 path and with the 386 path forced.
2. **The ELF loader accepts a short read** (#207): NetBSD 1.2's line.
3. **`bwrite` touches the buffer after an asynchronous write** (#252):
   NetBSD 1.1's corrections to `bwrite`, `getblk` and `getnewbuf`.
4. **`compress` overruns its decode stack** (#526): the first fix of
   CVE-2011-2895 among the three BSDs.
5. **The utilities**, each with the first fix of its hole:
   - `if_ed` (FreeBSD 2.1.7);
   - `test` with three arguments (FreeBSD 2.2);
   - `mkdir -p` (the first FreeBSD release with `if (!last)`);
   - `sa` (NetBSD 1.3);
   - `at` (OpenBSD's 1999 Y2K fix to `parsetime.c`, then the 2000
     `eqp` fix);
   - `uucpd` (OpenBSD 1996);
   - `deroff` (OpenBSD's own later fixes, each the first for its
     hole).
6. **The smaller ones:**
   - `re_comp` (`const` from NetBSD 1.1; the empty-string fix from
     FreeBSD 2.1 if it is earlier than NetBSD 1.4);
   - `pstat` (NetBSD 1.3);
   - ddb's ELF checks (NetBSD rev 1.16);
   - `wcd` (FreeBSD 2.1.7);
   - the ELF address arithmetic (NetBSD 1.3);
   - `strtoimax`'s "0x";
   - `uucplock`'s `snprintf`;
   - the `m4` and `deroff` man pages.
7. **My own two from 4.5BSD:** the stray-interrupt counter of patch
   222, and the `%ss:` overrides for `INTR_SPUR`.

## Stage 3: provenance

1. **The clearance ledger** (`provenance/clearance.md`, A1): one row
   per donor file in use, and the 386BSD rule applied to each.
2. **`execve`** (A2; #209, #211). The `getxfile` lines quoted from 4.4
   come out. Candidates in source order:
   - 386BSD 0.1's own `execve`, if the ledger finds it clear and it
     closes the gap with the ELF loader this tree uses;
   - otherwise the first release of NetBSD's exec framework that does,
     NetBSD 1.0, with 1.1 for ELF.

   Tested: init, ELF static binaries, `#!` scripts, set-user-ID,
   argument limits, `ps` strings, saved IDs.
3. **The other A2 files:** `kern_acct.c`, `subr_rmap.c`,
   `kern_physio.c`, `sys_process.c`. Each is measured, given a
   ledger row, and re-derived if it is not clear.
4. **The IDE disk driver** (#318, #319): **settled, 2026-10-30**
   (`provenance/disk-wd.md`). FreeBSD 2.1.0's `wd.c`, `wdreg.h`, the ATAPI
   and `wcd` files and the slice layer carry no Net/2-era text that
   Berkeley changed before Lite2, except the copyright notices. 7% of
   `wd.c` is from 386BSD 0.1 and the rest is FreeBSD's own. They stay.
   Work left in 4.5BSD:
   - the ledger row, naming the residue;
   - `wcd`'s bounds checks (FreeBSD 2.1.7), the first fix of that hole;
   - nothing else. The driver is not rewritten; 4.6BSD replaces it
     (`plan-4.6.md`, stage 4), so work on it now would be done twice.
5. **The floppy's DMA code** (#373). It is credited to a 1992 patchkit
   contribution. Measured with `build/lineage.py` as `wd.c` was, with
   Lite2's own `isa.c` DMA routines the first candidate; then the
   earliest clear donor if the test finds a residue that matters.
6. **Holders and licences (A3)** and **our own files' SPDX lines and
   `COPYRIGHT` (A4)**.

## Stage 4: the replay, by first fit

`roadmap-audit.md` Part B, with worksheet step 5 as first fit.

1. **Three of the sweep's decisions become corrections** under 4.5BSD's
   rule:
   - **`ptrace`** (#380): NetBSD 1.0's Lite-based `sys_process.c`
     (1994) comes before FreeBSD 2.2's (1997). Its unchecked
     `PT_WRITE_U` is a hole, fixed by the first fix of it. This
     coincides with the A2 item.
   - **The stack limit** (#442): FreeBSD 2.0, the source of
     `MAXDSIZ`, sets `MAXSSIZ` to 64 MB in the same file. Taking its
     line completes the patch.
   - **`microtime`** (#686): NetBSD 1.0's `microtime.s` is a donor
     text that closes the gap. Under the source order it comes before
     our C. `resettodr`'s donor (NetBSD 1.0's) is measured the same
     way.
2. **One decision must be measured first:**
   - **`sh`'s `getpwd`** (#264): NetBSD 1.1's four-line `getcwd` form
     is earlier than the loop we took. The loop is kept only if it
     fixes a hole the 1.1 form has.
3. **Six of the sweep's decisions are right for 4.5BSD and are kept.**
   Each was the first fit. 4.6BSD judges them again:
   - the ELF start-up files from NetBSD 1.6 (#20), right for GCC
     2.95.3;
   - `_C_LABEL` (#106);
   - `expr` from NetBSD 1.0 (#273);
   - `<stdint.h>`'s types (#342);
   - `ps_strings` (#474);
   - the boot blocks (#296), as decided on 2026-10-29 and unchanged.
4. **The attribution corrections** from the sweep: #9, #12, #31, #48,
   #55, #56, #66, #109, #629, #630, #683. #54 (`tip`) also moves to
   OpenBSD's 1997 form, which is earlier than NetBSD 1.4's.
5. **A first-fit pass over the 354 patches the sweep kept.** A best-fit
   KEEP can still be a later donor where an earlier one had the same
   fix. The sweep's measurements (each candidate's snapshot, and
   whether it holds our added lines) are read again for the earliest
   candidate holding the change. Where that is not the source taken,
   the patch is replayed. Most are expected to agree, because 4.5BSD's
   method was already earliest-first (UNVERIFIED until run).
6. **The audit's remaining flagged commits** (Part B stations), by the
   same worksheet.

## Stage 5: the tests

1. **Item 5:** NetBSD 1.0's `regress/` around Berkeley's own tests,
   the first fit for a test framework.
2. **The stability tests,** each run before the release and kept
   runnable:
   - the CD install on a blank disk;
   - multi-user boot;
   - `make build` on the installed system;
   - the network services (NFS, mail, cron);
   - `fsck` after a hard reset.
3. **The conformance suite, run once at the release** as a measurement.
   The baseline is recorded for 4.6BSD's first score. Nothing is
   changed in 4.5BSD to raise it.

## Stage 6: the kernel's loose ends (item 6)

- **The kernel's warnings audited.** Defects are fixed by first fit;
  the rest are recorded.
- **fd's debugging `printf`.**
- **Dial-out devices for the serial ports,** by first fit.
- **The kernel linked with a linker script,** by first fit, or recorded
  as deferred to 4.6BSD's compiler stage.

## Stage 7: the release (item 8)

1. **The name:** `newvers.sh` says 4.5BSD.
2. **The record of the release:**
   - `status.md` brought up to date;
   - what is done, what is known and left, and the audit findings not
     acted on;
   - what moved to 4.6BSD.
3. **The release notes,** the sets, and the installation guide as a
   release carries them.
4. **The final run of stage 5's tests,** and the tag. 4.6BSD is cut
   from the tag (`plan-4.6.md`).

## Order and gates

- **Stage 2, item 1 (`copyout`) comes before anything else.**
- **Stage 3 needs the ledger (3.1) before 3.2-3.5.**
- **Stage 4 waits for stage 3** wherever they touch the same file
  (`sys_process.c`).
- **Stage 7 waits for everything.**
- **Stages 5 and 6 can run alongside 2-4.**

No decision is open. Ports, Torek's autoconfiguration and X11R5 are
standing items for later (maintainer, 2026-10-30): not done in 4.5BSD
and not scheduled in 4.6BSD either (`plan-4.6.md`, stage 0). The
disk driver stays as it is.

## Work the two releases share

The maintainer's rule (2026-10-30): do nothing twice. A job both
releases need is done once, in 4.5BSD, and 4.6BSD inherits it. A job
4.6BSD will redo is not done in 4.5BSD.

**Done once in 4.5BSD, inherited:**

- the defects of stage 2, which are fixed whatever the rule;
- the clearance ledger, and `build/lineage.py` that fills it. 4.6BSD
  adds rows;
- the replacement of `execve`. It is chosen so that 4.6BSD can extend
  it without redoing it: a line of NetBSD's exec framework, not a
  one-off;
- the test harness and the baseline (stage 5);
- the measuring scripts for the sweep, moved from scratch into
  `build/` with the maintainer's SPDX line, so that 4.6BSD's best-fit
  worksheet uses the same tools;
- holders, licences and SPDX lines (A3, A4);
- the record of where 4.5BSD stopped.

**Left to 4.6BSD, not done in 4.5BSD:**

- the disk driver, ATAPI and the second channel;
- anything that needs Torek's autoconfiguration;
- ports, X11R5;
- the C17 conversion, the compiler, POSIX.1-2024;
- the six first-fit choices 4.6BSD will judge again (stage 4.3).

**The rule that keeps them apart:** if a 4.5BSD fix for a hole is the
same line best fit would pick, it stands. If best fit would pick a
different line, 4.5BSD keeps its first fit and 4.6BSD replaces it when
it touches the file; 4.5BSD does not guess ahead.
