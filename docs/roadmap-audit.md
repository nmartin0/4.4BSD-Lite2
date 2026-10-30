# Roadmap: the audit, and the replay through time

Decided by the maintainer (2026-10-29), after a four-run audit of every
commit made on top of 4.4BSD-Lite2 (688 commits, 50587b00..dev3 at
patch 222). The audit's per-commit reports are kept outside the tree;
this file is the plan that comes out of them. Nothing here has been
done yet. The 4.5BSD work list in `docs/roadmap.md` is on hold for
the stations marked HOLD below until their station is finished.

## What was decided

1. **Method: replay.** Each flagged commit is taken up as if it were
   being made today, standing at its parent commit: only what the tree
   held then, only the rules below. We redo the decision from the
   gap. If the answer differs from what was committed, the difference
   is fixed going forward.
2. **Scope: what is BSD-native.** The kernel, libc and the other
   libraries, the system's tools, the boot and install machinery, and
   the build scripts. Third-party software (GCC, GDB, binutils, pcc,
   perl, emacs, groff, flex, bc, Taylor UUCP, GNU diff, Kernighan's
   awk, and the like) stays as it is. Only the tree's own records
   about it (imports.md, COPYRIGHT) are corrected, if they are wrong.
3. **History stays as it is** for now. Nothing is rewritten and no
   branch is force-pushed. Every fix is a new patch on top. If
   encumbered text is confirmed in old commits, that is recorded in
   `docs/provenance/` and the tree is made clean going forward; whether
   the history is ever purged is left for later.
4. **Our own tools** (the `build/` scripts and programs, and anything
   else we wrote ourselves) carry the maintainer's copyright:
   `SPDX-FileCopyrightText: <year> Nicholas Martin`. No licence is
   granted by that line; the licence of our own files is not decided.

## The rules every replayed decision follows

The order of sources, for every change, and recorded in every commit:

1. **4.4BSD-Lite2 itself.** The same file; the same directory; every
   other port's version of the same thing; the i386 drivers and
   machine code; lib, usr.bin and the rest. Read first, always.
2. **386BSD 0.0 and 0.1, but only the files that are clear.**
   A file counts as clear only if (a) its own licence header permits
   use, (b) it is not Net/2- or 4.4BSD-derived text that the
   USL v. UCB settlement made unusable, including text the Regents
   marked open that the settlement overrode, and (c) it is not a file
   that FreeBSD, NetBSD or OpenBSD carried forward from those files
   unchanged. Where a 386BSD file is also in Lite2, Lite2's is used.
   386BSD 1.0 is not used (restrictive terms).
3. **The clear parts of FreeBSD, NetBSD and OpenBSD.** The same test
   applies to every donor file: its lineage must be shown, not
   assumed. A donor file of 1993-94 (NetBSD 0.9/1.0, FreeBSD 1.x/2.0)
   may descend from the unusable text and is treated as unknown until
   checked against Lite2 and the settlement's lists.
4. **Mach and the OSF code**, where the file is clearly licensed.
5. **Minix**, last, short of writing it ourselves.
6. **Our own writing.** Marked `AI-ONLY NOTE`, as always.

Always stated in the commit: which sources were checked, in this order,
and what each one held.

**The clearance ledger.** One table, `docs/provenance/clearance.md`,
with a row for every donor file the tree uses: donor, version, file,
the result of the lineage check, the licence found, the date checked.
A donor file is not used until it has a row. The first job of
station A is to build it for the files already in the tree.

**The worksheet** for each replayed commit, filled in the same order:

1. The gap, in Lite2 terms: what fails or is missing, and the evidence.
2. What Lite2 offers: same file, directory, other ports, i386 code, libs.
3. The 386BSD files, with their clearance.
4. The donors that remain, with their clearance.
5. The fit that closes the gap, by the rule of the release the patch
   belongs to (decided 2026-10-30, `plan-4.5.md`):
   - **for 4.5BSD, the first canonical fit:** the earliest candidate in
     the source order that closes the gap with 4.4's semantics, as a
     continuation, with clean provenance; then that line's first fix
     for each hole it is shown to have; ties to the smallest change,
     then donor order. The Part B replay uses this.
   - **for 4.6BSD, the best fit** (`design-4.6.md` rule 1a): every
     candidate from the oldest continuation forward through each donor
     to its plateau, scored on the six points (smallest change to
     Lite2's code, same semantics as 4.4, complete, fewest foreign
     dependencies, clean provenance, still a continuation); a tie goes
     to the candidate whose maintained code is closest to modern before
     its redesign plateau, then to donor order.

   (Until 2026-10-30 this step read "the smallest change that closes the
   gap", and the earliest precedent was in practice taken as the
   smallest. Patch 238 made it best fit for both releases; patch 241
   split it.)
6. Licence, holders and dates the result must carry.
7. Verdict: KEEP / SPLICE (smaller) / REPLACE (other source) / DROP.
8. If not KEEP: the new patch, tested, and the record it corrects.

## Order of work

Two parts. Part A is out of date order on purpose: it is the legal
exposure, and it comes first. Part B is the replay itself, in date
order, because later commits build on earlier ones.

### Part A: first

**A1. Clearance ledger and the rules in the tree** (docs only).
`docs/provenance/clearance.md`, built for every donor file in use;
`precedent.md` brought to the order above, with 386BSD's conditions.

**A2. The text that may have been copied from the encumbered files.**
Commits #185-#195 (`vfs_bio.c`), #204 and #206 (`execve`, 53% of its
lines match 4.4BSD's `kern_exec.c` by the reviewer's measure), #254
(`kern_acct.c`), #375 (`sys_process.c`, `procfs_machdep.c`) and the
`subr_rmap.c` transplant (#279). For each: measure against what we can
lawfully compare with; decide whether the body is Lite2's, a clear
donor's, or ours; re-derive from a clear source where it is not; keep
the Lite2 header only on text that is Lite2's. HOLD: no further kernel
work on these files until done.

**A3. Missing donor holders and licences in files we changed.**
Hannum's copyright in `locore.s`/`cpu.h` (#139) and `npx.c` (#684);
the NetBSD Foundation notice and Nordier's in `boot2.c` (#636); the
Regents/USL notice in `subr_rmap.c` (#279); the Caldera licence put on
`diff3.sh` (#402, since deleted; the records still describe it);
`wd.c`, `wdreg.h` and `sys/cdio.h` (#313); Demetriou's 1996 line in
`nlist.c` (#274); and the small ones the run-4 reports list. Each is
a header fix plus an imports.md line.

**A4. Our own files.** The SPDX copyright line for the maintainer on
every file of ours that lacks it (`build/*.sh`, `build/mklabel.c`,
`build/mkfs-host.c`, `build/addfile-host.c`, `build/shim/*`, new
makefiles and `devices.spec`, the `*.md` method files). The top-level
`COPYRIGHT` extended to list every holder the tree now carries.

### Part B: the replay, in date order

Each station lists the flagged commits it covers. Entries marked (R1)
to (R4) are the audit runs that flagged them.

**B1. Build and portability (commits 1-100).** #4, #6, #7, #9, #10,
#13, #15, #18, #20, #25, #26, #29, #31, #33, #34, #35, #40, #43, #49,
#50, #55, #62, #66, #68, #71, #78, #79, #82, #84. The question for most
is whether a Lite2 idiom from another port or directory would have
done (R1), and whether the donor took more than the fix (R3). Known:
#9 (the FreeBSD claim is contradicted by FreeBSD 2.0, and #39
reverted it), #15 (NetBSD 1.6 csu taken whole; Lite2's own
`crtstuff.c` and crt0 were not weighed), #29 and #82 (the host yacc),
#62/#78/#84/#108 (Kerberos optional: a policy departure from Lite2,
recorded as a decision), #55 (`kvm_i386.c`). Also #1, the test_mk7.3
tools.

**B2. The i386 kernel's first boot (91-170).** #91, #97, #100, #101,
#102, #103, #121, #136, #139, #140, #142, #143, #145, #150, #160,
#161, #165, #166, #168. Known: #101-#103 (`_C_LABEL` through the
kernel against the compiler's own underscore option: decide); #139
(90 lines for processor detection; Hannum); #140 and #121 (Lite2's
`trap.c` copyout was available); #145 (`delay()` rewritten whole for a
one-line fix; a Lite2 sparc/hp300 model exists); #142.

**B3. Kernel debugging and bring-up (171-270).** #198, #208, #214,
#225, #251, #257, #259, #262, #263, #267, #270. These are small fixes
of Lite2 code; the question is whether each is the smallest fix and
whether its record names Lite2's precedent.

**B4. Disks and drivers (271-440).** #313/#314 (FreeBSD 2.1.0's
IDE/ATAPI and slice layer, ~7,000 lines, replacing Lite2's `wd.c`),
#368 (the floppy and its DMA), #437, #438/#439 (`if_ed`, replacing
Lite2's `ne` and `we`; with a known defect `if (sc->type = ...)` in
the donor text), #469. For each the first question is Lite2's own
driver: could the actual fault have been fixed in it? Whatever 386BSD
offers is used only if it is clear (ledger). HOLD for the driver work.

**B5. Types and the C library (337-348).** #337, #338, #348:
`<stdint.h>`, `<inttypes.h>`, `strtoimax`. Lite2's `strtoq`/`strtouq`
and `types.h` first. The toolchain these were for is gone; the
question is whether they belong in 4.5BSD at all or in 4.6BSD.

**B6. The install and release machinery (300-370).** #306, #307, #310,
#316, #317, #318, #350, #355, #359, #360, #369. These are build and
install scripts; the question is what Lite2's own `distrib` and `etc`
trees offered, and the licence line (A4).

**B7. System tools (440-600).** #268 (`expr`), #282, #388, #401, #402,
#418, #501, #506, #513, #516, #521, #539, #545, #575-#589 (tip and
uucp wiring: the Taylor software stays; what is judged is the
displacement of Lite2's `tip` as `cu`, the locks, the group), #490
(the sendmail wiring only). For each: Lite2's program first, then
only the clear donors.

**B8. ddb (591-643).** #591, #592, #610, #636, #643. Is a kernel
debugger in 4.5BSD at all? If so, from which clear source (the CMU
code in 386BSD 0.1 is in the ledger's queue); the boot2 and licence
part is in A3.

**B9. The recent i386 kernel (644-687).** #648 (`syscall()` as a
trapframe; 429 lines for a ~20-line gap), #660, #670, #676, #679,
#683, #684, #686, #687. These include my own patches 219-222:
#686 changes Lite2's `kern_clock.c` and `dkstat.h` where Lite2's own
`dkn` convention existed; #687 emits tables inline where Lite2's
config helpers existed; #687's stray-vector counter and the SPUR
`%ss:` fix are still unfixed.

**B10. The records.** About twenty documentation commits are marked as
misstating or leaving out what Lite2 held: #36, #54, #75, #93, #137,
#159, #167, #217, #218, #231, #237, #248, #310, #317, #318, #332,
#336, #466, #516, #589, #666. Each is corrected in the provenance
file it belongs to. imports.md is corrected wherever the run-4 reports
found a licence missing or understated.

## How each station is run

1. A worksheet per commit is filled by an independent reviewer from the
   parent commit's tree, against the sources in the order above.
2. The claims that decide the verdict (the match with Lite2, the
   clearance of a donor file, the licence text) are checked by me
   against the actual files before anything is changed, and the
   evidence is written next to the verdict.
3. The maintainer sees the verdicts for the station before any patch.
4. Each fix is its own patch with an inline apply script, a test on
   the QEMU guest (boot, the affected tool or driver), and a provenance
   note that names the sources checked, in order.
5. The clearance ledger and the audit file are updated in the same
   patch.

## Open questions for the maintainer

- Whether a kernel debugger belongs in 4.5BSD (B8).
- Whether `<stdint.h>` and `<inttypes.h>` belong in 4.5BSD or 4.6BSD (B5).
- Whether the Kerberos policy (B1) is reaffirmed or reversed.
- The licence for our own files, when it is decided.
