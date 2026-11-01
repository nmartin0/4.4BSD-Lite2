<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

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
   **Done, patch 246** (`provenance/elf-short-read.md`).
3. **`bwrite` touches the buffer after an asynchronous write** (#252):
   NetBSD 1.1's corrections to `bwrite`, `getblk` and `getnewbuf`.
   **Done, patch 248** (`provenance/bufcache.md`).
4. **`compress` overruns its decode stack** (#526): the first fix of
   CVE-2011-2895 among the three BSDs.
   **Done, patch 249** (`provenance/compress-stack.md`).
5. **The utilities**, each with the first fix of its hole:
   - `if_ed`. **Done, patch 250** (`provenance/if_ed-assign.md`; the
     "FreeBSD 2.1.7" attribution below could not be confirmed and is
     corrected there).
   - `test` with three arguments. **Done, patch 251**
     (`provenance/test-3arg.md`).
   - `mkdir -p`. **Done, patch 252** (`provenance/mkdir-nul.md`).
   - `sa` (NetBSD 1.3). **Done, patch 253** (`provenance/sa-argv.md`).
   - `at` (NetBSD 1.3's 1998 Y2K fix to `parsetime.c` -- earlier than
     OpenBSD's 1999 one -- then OpenBSD's 2000 `eqp` fix). **Done,
     patch 254** (`provenance/at-fixes.md`).
   - `uucpd` (OpenBSD 1996). **Done, patch 255**
     (`provenance/uucpd-doit.md`).
   - `deroff` (OpenBSD's own later fixes, each the first for its
     hole). **Done, patch 256** (`provenance/deroff-overflows.md`).
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
   **Done, patch 257** (`provenance/smaller-fixes.md`): `re_comp`'s
   `const` fix was NetBSD 1.1 (pk, rev 1.5) as predicted, but the
   empty-string fix was NetBSD 1.2 (jtc, rev 1.6), not FreeBSD --
   NetBSD is earlier here and donor order prefers it regardless.
   `wcd`'s fit was FreeBSD, ache, 1996-01-30, and is itself incomplete
   (see that section's "what this fix does NOT close"). `strtoimax`'s
   fit is FreeBSD with the exact commit unrecoverable (bisected to
   between release/5.3.0 and release/5.5.0). All eight sub-items are
   in the one patch.
7. **My own two from 4.5BSD:** the stray-interrupt counter of patch
   222, and the `%ss:` overrides for `INTR_SPUR`.
   **Done, patch 258** (`provenance/intr-slots.md`): the counter-slot
   fix is confirmed against three live collisions in `GENERIC.i386`
   (`pc0`, `com0`, `wt0`), corroborated (not sourced) by FreeBSD 2.0's
   static `vector.s`. Independent review re-derived the collisions
   and the fix's logic from scratch and found no errors. The `%ss:`
   override was drafted, built, and confirmed by `objdump` to reach
   the compiled kernel, but dropped at the maintainer's direction
   (2026-10-31): it fixes nothing this tree's current flat segment
   layout can actually reach, and the real fit for this item is the
   counter-slot fix alone. `icu.h` is unchanged by this patch.

## Stage 3: provenance

1. **The clearance ledger** (`provenance/clearance.md`, A1): one row
   per donor file in use, and the 386BSD rule applied to each.
   **Built, patch 259.** 54 current donor files (40 kind D, 10 kind
   E, 4 kind F). Counted by hand: 22 have an actual stated clearance
   finding (the `wd.c` family and slice layer, `kvm_i386.c`, `ddb`,
   `bc`/`dc`, `uucp`, and kind E's and F's near-uniform "clear"
   verdicts); 3 are split between GNU's own clean release text and a
   donor's unmeasured changes to it (`gdb-4.16`, `gcc-2.95.3`,
   `binutils-2.11.2`); 3 were HOLD pending A2 (`sys_process.c`,
   `procfs_machdep.c`, `subr_rmap.c`; resolved in patch 265, item 3.3
   below); the remaining 26 were "not yet
   checked" (27 of 60 rows now) -- that backlog is A1's real remaining work, now visible
   instead of implicit. (This entry originally said "nine" and
   miscounted a HOLD row as settled; independent review caught both
   in `clearance.md` itself, and this entry is corrected to match,
   2026-10-31.) Building it also found eight patches `imports.md`
   mislabeled kind D (small fixes inside Lite2's own files, not file
   transplants) and one direct self-contradiction (patch 213);
   corrected in `imports.md` to kind B/C, per the standing rule that
   the tree's own records get fixed when wrong. (56 rows after patch
   262, which added the exec framework and the ELF loader's missing
   row; 24 with a stated finding.)
2. **`execve`** (A2; #204, #206). The `getxfile` lines quoted from 4.4
   come out. Candidates in source order:
   - 386BSD 0.1's own `execve`, if the ledger finds it clear and it
     closes the gap with the ELF loader this tree uses;
   - otherwise the first release of NetBSD's exec framework that does,
     NetBSD 1.0, with 1.1 for ELF.

   Tested: init, ELF static binaries, `#!` scripts, set-user-ID,
   argument limits, `ps` strings, saved IDs.
   **Partly done, patch 261** (`provenance/exec-stack.md`): the
   quoted fragment is out, replaced with the same reasoning in this
   tree's own words, with a regression test. Measured 386BSD 0.1's
   own `execve` (`/home/claude/386bsd`, tag `r01`): clear (Jolitz/
   TeleMuse's own licence, an independent, deliberately simplified
   design, not carried forward unchanged by any later BSD), but
   `a.out`-only -- no ELF, no `#!`, no `ps_strings` -- so it does not
   close this tree's gap. Per this item's own source order, the fit
   moves to NetBSD's exec framework: `origin/netbsd-1-0` for the
   generic driver/vmcmd-list/format-table shape, `origin/netbsd-1-1`
   for `exec_elf.c`. **Not yet done:** the actual rewrite of
   `kern_exec.c` to that shape -- a boot-critical change (every
   process start, including `init`'s own first exec, goes through
   it) too large to land in the same patch as the measurement. Until
   it lands, A2's 53% line-match finding is not resolved, only the
   one literal quotation inside it.
   **Done, patch 262** (`provenance/exec-stack.md`): NetBSD's exec
   framework shrink-wrapped in place of the body #204 wrote --
   NetBSD 1.0's `kern_exec.c`, `exec_subr.c`, `exec_script.c` and
   `sys/exec.h` framework with 1.1's fixes, 1.1's `exec_conf.c` and
   ELF loader -- by `precedent.md`'s rule for a missing body, after
   the maintainer weighed every option (OSFMK, Mach, Lites, a
   386BSD splice, FreeBSD's line, NetBSD) and chose NetBSD. Tested by
   `build/test/execve.c` under QEMU, the plan's whole list and the
   behaviours that change; the same test against the old kernel
   shows which lines tell them apart, and a real-shell test and a
   multi-user boot run the system's own programs. Independent review
   found one regression -- NetBSD 1.0's stack setup killed the exec of
   any process with a stack limit not a whole number of pages -- fixed
   with NetBSD 1.3's line in `kern_resource.c`, and the test now shows
   it. Four errors in patch 261's record corrected there.
   `kern/exec_elf.c`, never in `imports.md` and so never in the
   ledger, now in both. Left for the maintainer: a traced process may
   exec a program it cannot read, as every BSD allows.
3. **The other A2 files:** `kern_acct.c`, `subr_rmap.c`,
   `kern_physio.c`, `sys_process.c`. Each is measured, given a
   ledger row, and re-derived if it is not clear.
   **Done, 2026-11-01 (patch 265)**, `provenance/a2-files.md`. All
   measured, plus `procfs_machdep.c` and `vfs_bio.c`. Only `vfs_bio.c`
   needed an edit: 18 comment lines that matched 386BSD 0.0 word for
   word, reworded; the object file is byte-identical. The ledger had no
   rows for `kern_acct.c`, `kern_physio.c`, `vfs_bio.c` or `tty_subr.c`;
   they have them now (61 rows after 267, no HOLD). Left open: `kern_acct.c` and
   `subr_rmap.c` have no readable Net/2 body to compare, so they rest on
   authorship; `tty_subr.c` is unmeasured and its "interim" note is
   stale; the 17 interface lines in `vfs_bio.c`; the first-fit replay of
   `sys_process.c` to NetBSD 1.0 (stage 4, #380).
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
   **Done, 2026-11-01 (patch 267)**, `provenance/floppy-dma.md`: clear,
   no Net/2-era text Berkeley changed found by the test (not "clear" in a stronger sense; see the record); Lite2's `at_dma` is a different text and FreeBSD 2.0 is
   the earliest readable source with the fixes, so the code stays. The
   missing MacLachlan credit is restored in `isa.c` (comment only).
6. **Holders and licences (A3)** and **our own files' SPDX lines and
   `COPYRIGHT` (A4)**.
   **Surveyed, 2026-11-01 (patch 267)**, `provenance/holders-licences.md`:
   five wrong ledger cells fixed, six own files given their SPDX line.
   **Not done:** `COPYRIGHT` itself, 24 partial holder cells, A3's named
   files, and a header-against-donor check (0 of 60). The write-ups got the line in
   patch 268 (maintainer: yes); the 20 `.want` fixtures cannot, as the
   test compares them exactly.

## Stage 4: the replay, by first fit

`roadmap-audit.md` Part B, with worksheet step 5 as first fit.

1. **Three of the sweep's decisions become corrections** under 4.5BSD's
   rule:
   - **`ptrace`** (#380): NetBSD 1.0's Lite-based `sys_process.c`
     (1994) comes before FreeBSD 2.2's (1997). Its unchecked
     `PT_WRITE_U` is a hole, fixed by the first fix of it. This
     coincides with the A2 item.
     **Done, 2026-11-01 (patch 271)**, `provenance/ptrace-firstfit.md`.
     NetBSD 1.0's file, but its line's first fix (NetBSD 1.1 removes
     `PT_READ_U`/`PT_WRITE_U`) breaks this tree's gdb, shown; the hole is
     closed with the `ptrace_write_u` the tree already had, recorded as
     a departure from the rule.
   - **The stack limit** (#442): FreeBSD 2.0, the source of
     `MAXDSIZ`, sets `MAXSSIZ` to 64 MB in the same file. Taking its
     line completes the patch.
     **Done differently, 2026-11-01 (patch 272)**,
     `provenance/stack-limit.md`: NetBSD 1.0 (8 MB) is the earlier donor,
     and the maintainer's decision of 2026-10-18 keeps the stack limits
     Lite2's, so `MAXSSIZ` is Lite2's 32 MB, with NetBSD 1.0's `maxsmap`
     cap so that a root limit above it cannot break exec.
   - **`microtime`** (#686): NetBSD 1.0's `microtime.s` is a donor
     text that closes the gap. Under the source order it comes before
     our C. `resettodr`'s donor (NetBSD 1.0's) is measured the same
     way.
     **Done, 2026-11-01 (patch 270)**, `provenance/microtime.md`:
     NetBSD 1.0's `microtime.s`; `resettodr` kept (its donor needs
     NetBSD's whole RTC section).
2. **One decision must be measured first:**
   - **`sh`'s `getpwd`** (#264): NetBSD 1.1's four-line `getcwd` form
     is earlier than the loop we took. The loop is kept only if it
     fixes a hole the 1.1 form has.
     **Measured, 2026-11-01 (patch 273)**, `provenance/pwd.md`: the form
     is first in OpenBSD 1995-12 and NetBSD 1.2, not 1.1, and its
     256-byte buffer fails on a path over 255 characters (shown in the
     guest with a 574-character path). The loop stays.
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
   **Done, 2026-11-01 (patch 274)**: each note corrected in its file,
   and the list is in `provenance/precedent.md`, *Corrections to
   committed messages*. `tip` moved to OpenBSD's form with its objects
   unchanged.
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
4. **One entry point and two tiers** (maintainer, 2026-10-30: the tests
   are built here, once, for both releases). A single command runs
   them on a given kernel or release and prints PASS or FAIL. The
   quick tier runs on every patch (the per-defect tests, boot,
   multi-user); the full tier runs per milestone (the CD install, the
   system building itself, the network services, the suite). Results
   are stored in a form that can be diffed against the baseline.
5. **Every defect fixed in stage 2 keeps a test that fails without
   the fix,** as `build/test/copyio.sh` does (patch 243). Tests are our
   own writing, in the tree. The suite stays outside it; the harness
   only needs to know where to find it and how to run it.
6. **The baseline is behaviour, not correctness.** It says what 4.5BSD
   does, known failures recorded as known, so that 4.6BSD has to show
   that nothing got worse; the suite says what is wrong.

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
- **Stages 5 and 6 can run alongside 2-4,** but stage 5 is complete
  before the tag: 4.6BSD's first stage needs its baseline.

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
