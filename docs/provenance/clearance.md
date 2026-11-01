<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# The clearance ledger

Stage 3, item 1; `roadmap-audit.md`'s audit station A1, "the legal
exposure," which that file puts ahead of everything else in the
replay: "the first job of station A is to build it for the files
already in the tree." One row per donor file this project has put
into `usr/src` -- a specific other-system file, with its own licence
header, that was not in 4.4BSD-Lite2 -- with whatever this tree's own
records say about its clearance. This is a census of what has (and
has not) actually been checked, not a claim that everything below is
settled; most rows are not, and say so.

## What counts as a donor file here

`imports.md`'s kind D ("transplanted, never in 4.4BSD-Lite2"), kind E
("a reimplementation of something the Lite cut removed"), and kind F
("Berkeley's own file, restored") are the three kinds that put another
system's text into `usr/src` (`imports.md`'s own framing, "A note on
sys/i386" section). Those are this ledger's rows. Kind G ("ours,
outside Berkeley's tree") is not a donor file; it belongs to A4
(`roadmap-audit.md`), not here. Kinds A, B and C are Berkeley's own
code, corrected or judged with or without a donor's line to follow --
also not a donor file, even where a donor is cited, because the file
living in `usr/src` is still Lite2's own.

**AI-ONLY NOTE:** `imports.md` labeled eight patches "kind D" that are
not file transplants at all -- small corrections inside Lite2's own
`clock.c`, `icu.s`, `icu.h`, `locore.s` and `kern_proc.c` (patches 208,
209, 212, 213, 214, 215), a single compile flag with no file of its
own (patch 203), and a conversion of `locore.s`'s own syscall frame
following FreeBSD's design (patch 202). Patch 213's own text said "no
donor of the period does that at shutdown; it is mine" for part of a
change the same entry called kind D -- a direct self-contradiction.
Building this ledger is what surfaced it. Corrected in `imports.md`
today (2026-10-31), each to kind B (patch 213 to B/C, since part of it
has no donor at all), per the standing rule that "only the tree's own
records about it ... are corrected, if they are wrong" (`roadmap-
audit.md`, item 2). None of the eight appear below; they were never
donor files.

**AI-ONLY NOTE, a scope call, not resolved here:** `sys/vm` and the
per-port `pmap.c` files are Mach-derived (`precedent.md`'s "The VM
system is Mach's" section), but they shipped that way in 4.4BSD-Lite2
itself -- this project did not transplant them. Read narrowly,
`roadmap-audit.md`'s "for every donor file the tree uses ... for the
files already in the tree" could mean literally every file with any
non-Berkeley ancestry, which would pull in `sys/vm`'s roughly 17 files
and 7+ `pmap.c`s. This ledger does not include them, on the reading
that "donor file" means a file this project introduced from a donor,
matching kinds D/E/F's own definitions ("never in 4.4BSD-Lite2," "the
Lite cut removed," "restored") -- all three describe something this
tree's own history did, not Lite2's inherited baseline. If the
maintainer wants `sys/vm`/`pmap.c` rows too, that is a second pass,
not folded in here silently.

## The lineage test applied

`roadmap-audit.md`'s rule 2, which `disk-wd.md` calls "the 386BSD
rule," is written for 386BSD specifically:

> 2. **386BSD 0.0 and 0.1, but only the files that are clear.**
>    A file counts as clear only if (a) its own licence header permits
>    use, (b) it is not Net/2- or 4.4BSD-derived text that the
>    USL v. UCB settlement made unusable, including text the Regents
>    marked open that the settlement overrode, and (c) it is not a file
>    that FreeBSD, NetBSD or OpenBSD carried forward from those files
>    unchanged. Where a 386BSD file is also in Lite2, Lite2's is used.
>    386BSD 1.0 is not used (restrictive terms).

Rule 3 extends the same test to FreeBSD, NetBSD and OpenBSD directly:
"its lineage must be shown, not assumed... treated as unknown until
checked against Lite2 and the settlement's lists." `disk-wd.md`'s
method -- classify every line of a candidate file as Lite2's,
"early, not Lite2" (the condition-(b) risk: Net/2-era text Berkeley
later changed or dropped), or genuinely new to the donor -- is how
that showing gets done in practice; it is the only method in this
tree so far that actually produces a clearance finding rather than
a bare "the licence is X's own notice."

Most rows below have not had that method run on them. "Not yet
checked" below means exactly that: `imports.md` states where the file
came from, but no `disk-wd.md`-style measurement against the
settlement's conditions has been done. This is the gap A1 exists to
close, not a finding that the files are clean.

## Kind D -- transplanted, never in 4.4BSD-Lite2

| Donor | Version | File | Clearance | Licence/notice | Patch |
|---|---|---|---|---|---|
| NetBSD | 1.6 | `lib/csu/*` (619 lines when listed; 754 now) | not yet checked; `imports.md` gives no USL-clearance statement, only that ELF startup code could not have existed in 1995 | Four-clause BSD with advertising clause throughout, four notices: Christopher G. Demetriou 1995 (common.c, common.h); Christos Zoulas 1998 + Demetriou 1995 (i386_elf/crt0.c); The NetBSD Foundation 1998, 2001 / 2001 (crtbegin.c, dwarf2_eh.h); Ross Harvey 2001 (both dot_init.h). crtend.c and the Makefile carry none. All notices identical to netbsd-1-6. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0, adapted | `lib/libkvm/kvm_i386.c` (221 lines) | clear by construction: ports Berkeley's own `kvm_hp300.c` (Regents copyright) to a machine Berkeley never supported; in no CSRG tree at all | Regents of the University of California 1989, 1992, 1993 (LBL CSE group, DARPA BG 91-66); four-clause BSD, advertising clause; identical to NetBSD 1.0's. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.0.5 | `vmstat/names.c`, i386 case (23 lines) | not yet checked | Regents 1991, 1993, four-clause BSD, advertising clause (the file's own notice, identical to FreeBSD 2.0.5's); the i386 case carries no separate notice; FreeBSD attributes it to Rodney W. Grimes. (header checked against the donor's original, 2026-11-01, patch 275) | #221 |
| NetBSD | 1.0 | `lib/libl/Makefile` (4 lines) | not yet checked | none (the donor Makefile has no notice; it says it comes from Berkeley's Makefile 5.1 of 6/18/90) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.3, adapted | `lib/libc/gen/nlist_elf32.c` | not yet checked | Christopher G. Demetriou 1996, four-clause BSD, advertising clause; identical to NetBSD 1.3 rev 1.6. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.3, adapted | `usr.sbin/kvm_mkdb/nlist_elf32.c` | not yet checked | Christopher G. Demetriou 1996, four-clause BSD, advertising clause; identical to NetBSD 1.3 rev 1.4.2.1. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.1, rev 1.3 (Christos Zoulas); adapted, then whole less its dynamic-linking and emulation parts (patch 262) | `kern/exec_elf.c` | **measured** (`exec-stack.md`): 0 of its 91 distinct code lines in 386BSD 0.0's or 0.1's `kern_execve.c`; clear on what can lawfully be compared. **Missing from this ledger until 2026-10-31**: `imports.md` had never listed it, and the ledger was built from `imports.md` | Christos Zoulas 1994, three-clause BSD, no advertising clause; identical to NetBSD 1.1 rev 1.3. (header checked against the donor's original, 2026-11-01, patch 275) | #202/#203; 262 |
| NetBSD | 1.3 (`Shdr`/`Sym`, rev 1.1 from NetBSD 1.1 for the header) | `sys/exec_elf.h` | not yet checked | Christos Zoulas 1994, three-clause BSD, no advertising clause (identical to NetBSD 1.1 rev 1.1; NetBSD 1.3 rev 1.11, source of Shdr/Sym, has the same notice). (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 4.1, adapted | `sys/boot/i386` (`boot1.s`, `boot2.c`, `lib.h`, `sio.s`, `btx*.s`/`.h`) | not yet checked | Robert Nordier 1998, his one-paragraph permissive notice (notice, permission paragraph and disclaimer to be kept), no numbered clauses, no advertising clause; identical to FreeBSD 4.1's in all nine source files, except that boot2.c also carries, in a second block added in patch 275, The NetBSD Foundation's 1997 notice (Jason R. Thorpe), four-clause with an advertising clause, for its -d symbol loading from NetBSD 1.3's loadfile.c; Makefiles carry none (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 4.1, adapted | `usr.sbin/btxld` | not yet checked | Robert Nordier 1998, two-clause BSD, no advertising clause (btxld.c, btx.h, btxld.8); identical to FreeBSD 4.1's. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.0.5 | `i386/autoconf.c`: `MFS_ROOT` (4 statements) | not yet checked | none of its own; the fragment sits under the file's Regents notice (Regents 1990[, 1993], William Jolitz contribution, four-clause, advertising clause), the same holder as FreeBSD 2.0.5's file. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.0.5, adapted | `release/write_mfs_in_kernel.c` | not yet checked | Poul-Henning Kamp, "THE BEER-WARE LICENSE" (Revision 42): keep the notice; no numbered clauses, no advertising clause; identical to FreeBSD 2.0.5's (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 3.0, adapted | `usr.sbin/crunch` (`crunchgen`, `crunchide`) | not yet checked | U. Maryland 1994 permission notice (X11-style; no numbered clauses; U.M. name not to be used in advertising) on crunchgen and the man pages; crunchide.c: Christopher G. Demetriou 1997 + U. Maryland 1994 under the U.M. notice; crunchide/exec_elf32.c and extern.h: Demetriou 1997, four-clause BSD, advertising clause. All identical to FreeBSD 3.0's. (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.1, adapted | `distrib/i386/floppies` | not yet checked | Demetriou 1994, four-clause, advertising clause (inst/dot.profile, dot.hdprofile, dot.instutils, install.sh; inst-common/dot.commonutils); the Regents' 1980/1985/1989, four-clause, advertising clause (inst-common/termcap.pc3); the remaining NetBSD files carry no notice; devices.spec is the tree's own (SPDX 2026 Nicholas Martin) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.0, rev 1.35 (ache 1994/11/01); `ic/i8237.h` rev 1.2 | `i386/isa/isa.c` (`isaphysmem`, `isa_dmastart`, `isa_dmadone`, `isa_dmarangecheck`, `DMA1_*`/`DMA2_*`), `i386/isa/ic/i8237.h`, `i386/i386/pmap.c` (`DMAPAGES`, `isaphysmem`) | **measured, 2026-11-01** (`floppy-dma.md`): of 95 distinct lines in the DMA region, 3 are Lite2's `at_dma` (generic comments), **0** are 386BSD 0.0's and not Lite2's, 48 (51%) are first in 386BSD 0.1 (Regents' notice), 42 (44%) first in FreeBSD 2.0 (credited to MacLachlan, 1992), 2 ours. Net/2 itself not read; the 1992 patchkit not available; MacLachlan's own notice not established. No Net/2-era text Berkeley changed found by this test (not "clear" in a stronger sense: Net/2 unread, rewording would not show, the 48 lines of 1992 authorship unshown); residue named | isa.c: the Regents' (Lite2's 1991, 1993; Jolitz contributor), four-clause, advertising clause, plus FreeBSD 2.0's credit to Frank P. MacLachlan 92/08/18 (no notice of his own); i8237.h: no notice (FreeBSD 2.0's, same); pmap.c: the Regents' (Lite2's), four-clause, advertising clause. FreeBSD 2.0's pmap.c, which the DMAPAGES lines come from (introduced by Dyson/Greenman, FreeBSD commit 7f8cb36869, 1994-01-14), also carries John S. Dyson 1994 and David Greenman 1994 under the same four clauses; not in ours; the DMAPAGES lines (a constant's name and two uses) are judged too small to carry Dyson's and Greenman's notice, and recorded here instead (header checked against the donor's original, 2026-11-01, patch 275) | d72ca64a; #373 |
| FreeBSD | 2.1.0, adapted | `i386/isa/wd.c` | **settled, 2026-10-30** (`disk-wd.md`): no Net/2-era text Berkeley changed or dropped survives but the copyright notice; 7% is 386BSD 0.1's own (not Net/2 text); the rest is FreeBSD's own; clear, residue named | the Regents' 1990 (William Jolitz contributor), four-clause, advertising clause; identical to FreeBSD 2.1.0's (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/wdreg.h` | **measured** (`disk-wd.md`): 2 of 112 lines early-not-Lite2 (the copyright notice only); clear | the Regents' 1991 (William Jolitz contributor), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/atapi.c` | **measured** (`disk-wd.md`): 0 lines in 386BSD 0.0 at all; clear | Cronyx Ltd. 1995 (author Serge Vakulenko): unnumbered permission notice (no warranty; use/modify provided the message is kept), no advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/wcd.c` | **measured** (`disk-wd.md`): 3 of 523 lines in 386BSD 0.0 (negligible); clear. Bounds-check fix applied, patch 257 | Cronyx Ltd. 1995 (author Serge Vakulenko): unnumbered permission notice, no advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319; fix patch 257 |
| FreeBSD | 2.1.0, adapted | the slice layer: `kern/subr_diskslice.c`, `kern/subr_dkbad.c`, `i386/isa/diskslice_machdep.c` | **measured** (`disk-wd.md`): 2 lines each early-not-Lite2 (copyright notice only); clear | Bruce D. Evans 1994 with the Regents' 1990 (Jolitz contributor; subr_diskslice.c and subr_dkbad.c only) and 1982, 1986, 1988 (all three), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `<sys/diskslice.h>` | not yet checked -- `disk-wd.md`'s measured table covers the three `.c` files above but not this header; grouping it with them was an overstatement, corrected 2026-10-31 | Bruce D. Evans 1994, two-clause, no advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `usr.sbin/config`: `isa_biotab()` | not yet checked (same release as the `wd.c` family it accompanies, but not itself measured) | the Regents' 1980, 1993 (mkioconf.c, Lite2's and FreeBSD 2.1.0's alike), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/if_ed.c`, `if_edreg.h` | not yet checked (`if_ed-assign.md` is a later bugfix, not a clearance measurement) | David Greenman 1993, unnumbered permission notice (terms retained), no advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0 | `lib/libcompat/4.1`: `gtty`, `stty`, `ftime` | not yet checked | Christopher G. Demetriou 1994, four-clause, advertising clause (gtty.c, stty.c, ftime.c; identical to NetBSD 1.0's) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0 | `usr.bin/m4/m4.1` | not yet checked | no notice (NetBSD 1.0's m4.1 has none either) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0, rev 1.33.2.4 (Demetriou 1994), shrink-wrapped (patch 271); `PT_WRITE_U` confined by FreeBSD 2.2's `ptrace_write_u` | `kern/sys_process.c` | **measured, 2026-11-01** (`ptrace-firstfit.md`): 9 of 253 lines (3%) are in 386BSD 0.1's Net/2 body and not Lite2's stub, all `case PT_...:` labels and one `#ifdef`; clear on this test. Replaces FreeBSD 2.2's (16 of 182, 9%, `a2-files.md`) by first fit. NetBSD 1.1's removal of the U requests not taken (it breaks this tree's gdb, shown) | Christopher G. Demetriou 1994 and the Regents' 1982, 1986, 1989, 1993 with the USL '(c) UNIX System Laboratories, Inc.' derivation paragraph, four-clause, advertising clause; identical to NetBSD 1.0's (header checked against the donor's original, 2026-11-01, patch 275) | #380, #375 |
| NetBSD | 1.0, `i386/microtime.s` v1.8 (the Regents' 1993; no author named in the file), shrink-wrapped, 9 labelled changes (patch 270) | `i386/i386/microtime.s`; `isa/timerreg.h` (the TIMER_ lines moved out of `clock.c`) | **0 lines shared with what it replaces; donor, not measured against Net/2** (`microtime.md`): 386BSD has no `microtime.s`; the Regents' notice is Lite2-era. The one-tick check kept in `clock.c` is patch 217's, in no donor | microtime.s: the Regents' 1993, four-clause, advertising clause (identical to NetBSD 1.0's; no author named in the file); timerreg.h: Lite2 clock.c's notice, the Regents' 1990, 1993 (William Jolitz and Don Ahn contributors), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #686 |
| NetBSD | 1.0, `kern_resource.c` rev 1.20 and `vm_glue.c` rev 1.39: `maxsmap` (patch 272) | `kern/kern_resource.c`, `vm/vm_glue.c` (three lines); `i386/include/vmparam.h` `MAXSSIZ` 32 MB, Lite2's value | clear: Lite2's own files with NetBSD 1.0's three Lite-based lines (`stack-limit.md`) | Lite2's own notices: kern_resource.c, the Regents' 1982, 1986, 1991, 1993 with the USL derivation paragraph; vm_glue.c, the Regents' 1991, 1993 plus Carnegie-Mellon University 1987, 1990 (Mach permission notice); vmparam.h, the Regents' 1990, 1993; Regents' notices four-clause, advertising clause; NetBSD 1.0's copies carry the same holders (header checked against the donor's original, 2026-11-01, patch 275) | #442 |
| NetBSD (as it had it, Jan 1994), adapted | -- | `i386/i386/procfs_machdep.c` | **likely clear, 2026-11-01** (`a2-files.md`; a sibling of Lite2's own files, nothing comparable readable): Pendry's own `/proc` code for 4.4BSD, of which Lite2 ships two siblings (`hp300`, `pmax`) under the same Regents-and-Pendry notice; 45% of its lines are in `hp300`'s; 386BSD, which carries Net/2's text, has no `/proc`. Not a settlement body | Regents of the University of California 1993 + Jan-Simon Pendry 1993; 4 clauses, advertising clause (as NetBSD 1.0's process_machdep.c; fpregs code from FreeBSD 2.1.0 under the same notice) (header checked against the donor's original, 2026-11-01, patch 275) | #375 |
| FreeBSD | 2.2, adapted | `i386/i386/machdep.c`: `ptrace_set_pc`, `ptrace_write_u` | not yet checked | Regents of the University of California (Lite2's file), 4 clauses, advertising clause; ptrace_set_pc/ptrace_write_u from FreeBSD 2.2's machdep.c, whose notice is Terrence R. Lambert 1992 + Regents 1982, 1987, 1990, same 4 clauses with advertising clause; ptrace_write_u itself was added to FreeBSD's machdep.c by Bruce Evans on 1995-01-14 (security fix, fbh 20415301cd), after Lambert's 1992 work, so Lambert's notice is not carried for it (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.2 | `i386/include/ptrace.h`: `PT_GETREGS` family | not yet checked | Regents of the University of California 1992, 1993; 4 clauses, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD (1993) / FreeBSD 2.1.0 | -- | `i386/include/reg.h`: `struct reg` (NetBSD), `struct fpreg` (FreeBSD) | not yet checked | Regents of the University of California (Jolitz); 4 clauses, advertising clause, as both donors' reg.h (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 2.2, adapted | `lib/libc/sys/ptrace.2` | not yet checked | public domain (FreeBSD 2.2's page, rev 1.10, itself NetBSD's rev 1.2); no holder (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| FreeBSD | 5.0, unmodified | `kern/genassym.sh`, `sys/assym.h` | not yet checked. **AI-ONLY NOTE:** no reason is stated in `imports.md` for reaching to FreeBSD 5.0 specifically -- every other late-reaching donor choice in that file gives one (ELF `csu` needing a modern crtbegin, `boot2` needing `.code16`-capable assembly); this one does not. Flagged, not guessed at. | assym.h: Marcel Moolenaar 1999, 3 clauses, no advertising clause; genassym.sh: no notice (as FreeBSD 5.0) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0's system `diff` | `contrib/diff-2.6` (GNU diff 2.6) | not yet checked for the NetBSD wrapper/Makefile; the GNU code itself is third-party, out of this project's BSD-native scope (`roadmap-audit.md` item 2) | GPL v2 or later (FSF); NetBSD 1.0's makefiles carry no notice; licence text in diff/COPYING (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.1's system `grep` | `contrib/grep-2.0` (GNU grep 2.0) | same as above | GPL (FSF): v2 or later, kwset.c v1 or later; NetBSD 1.1's Makefile carries no notice; licence text in COPYING (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.1's system `bc`/`dc` | `contrib/bc-1.03` (GNU bc 1.03 + dc) | **detailed** (`imports.md`): NetBSD 1.1's import (not byte for byte, as this cell first said: bc's proto.h and nine uucp files are patched, no notice line touched; found 2026-11-01, patch 275) (Phil Nelson, 1 Dec 1994); Berkeley's own `bc`/`dc`/`libmp` were AT&T's, cut from Lite2 | GPL v2 or later (FSF; bc by Philip A. Nelson); licence text in bc/COPYING, dc/COPYING (header checked against the donor's original, 2026-11-01, patch 275) | #194 |
| NetBSD | 1.1's system `uucp` | `contrib/uucp-1.06.1` (Taylor UUCP) | **detailed** (`imports.md`): NetBSD 1.1's import (not byte for byte, as this cell first said: bc's proto.h and nine uucp files are patched, no notice line touched; found 2026-11-01, patch 275); Berkeley's `uucp` AT&T-marked 1991, cut from Lite2 | GPL v2 or later (Ian Lance Taylor; some files FSF; proty.c Jorge Cwik and Ian Lance Taylor); crc.c Gary S. Brown, unrestricted use; contrib/tstout.c (not built) Regents 1988, old advertising-acknowledgement paragraph; licence text in COPYING (header checked against the donor's original, 2026-11-01, patch 275) | #197/#198 |
| NetBSD | 1.0, adapted (CMU Mach 3.0 origin) | `sys/ddb`, i386 `db_*.c`, `db_machdep.h` (28 files) | **detailed** (`imports.md`): CMU's licence alone, no USL exposure since the text is CMU/Mach-origin, not Berkeley/Net2; cross-checked against CMU Mach 3.0 directly | Carnegie Mellon University 1990, 1991 (Mach permission notice, no numbered clauses, no advertising clause), as NetBSD 1.0's text (header checked against the donor's original, 2026-11-01, patch 275) | #199 |
| NetBSD | 1.0, unmodified | `share/man/man4/ddb.4` | not yet separately checked; presumably the ddb files' own clearance | Carnegie Mellon University 1990, 1991 (Mach permission notice; no advertising clause), via NetBSD 1.0, unmodified (header checked against the donor's original, 2026-11-01, patch 275) | #199 |
| NetBSD | 1.3's `db_elf.c` (interface of rev 1.17/1.14; the file's own header and note say rev 1.2, thorpej 1997/09/05) | `sys/ddb`: ELF symbol reading | not yet checked | The NetBSD Foundation 1997 (Jason R. Thorpe); 4 clauses, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #201 |
| GNU release + FreeBSD 2.2.8's changes and glue | gdb 4.16 | `contrib/gdb-4.16` (1820 files) | kind A for GNU's own release (third-party, out of scope); kind D for FreeBSD's twelve changed files plus FreeBSD's own glue (BSD makefiles, hand-made `config.h`/`bfd.h`, `i386b-nat.c`, `core-aout.c`, `kvm-fbsd.c`) | GPL v2 or later (FSF), readline GPL v1 or later, mmalloc LGPL v2; libiberty's Berkeley files (Regents; strtol.c/strtoul.c 4 clauses with advertising clause); FreeBSD 2.2.8's glue (gnu/usr.bin/gdb) FSF/GPL v2+ where it carries a notice, makefiles none; licence text in COPYING, COPYING.LIB (header checked against the donor's original, 2026-11-01, patch 275) | #204-#206 |
| GNU release + NetBSD 1.6's changes, this tree's own glue | gcc 2.95.3 | `contrib/gcc-2.95.3` | not yet checked for lineage; GNU's release is verified byte-identical to its own `md5sum.txt` (third-party, out of scope), NetBSD's `netbsd-1-6` changes (78 files changed, 18 added, 6 removed) are kind D and not yet measured against the settlement; this tree's own system-name/link glue is kind G | GPL v2 or later (FSF); libgcc2, libio and libstdc++ with GNU's linking exception, file by file; Berkeley files in GNU's release (Regents: gcc/gmon.c, libiberty strtol/strtoul with advertising clause, libio's printf/scanf second block); NetBSD 1.6's added config files carry no notice; licence text in COPYING, COPYING.LIB (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| GNU release + NetBSD 1.6's changes, this tree's own glue | binutils 2.11.2 | `contrib/binutils-2.11.2` | not yet checked for lineage; GNU's release verified against its own release + the 2.11.2a re-release GNU's key signs (third-party, out of scope), NetBSD's `netbsd-1-6` changes (114 changed, 30 added) are kind D and not yet measured; this tree's own glue is kind G | GPL v2 or later (FSF); gprof (built): Regents of the University of California, several notices, with advertising clause/acknowledgement (gmon.h 4 clauses; gprof.c, utils.c, the 1983 machine files; NetBSD 1.6's added mips.c: Regents/LBL, 4 clauses, advertising clause); libiberty's Berkeley files; tc-alpha.c CMU 1993; NetBSD's added a.out/emulation files mostly carry no notice; licence text in COPYING, COPYING.LIB (header checked against the donor's original, 2026-11-01, patch 275) | -- |

## Kind E -- a reimplementation of something the Lite cut removed

| Donor | Version | File | Clearance | Licence/notice | Patch |
|---|---|---|---|---|---|
| NetBSD | 1.0 | `lib/libcompat/4.3/regex.c` (93 lines) | clear: Berkeley-licensed (James da Silva, U. Maryland, contributed to Berkeley), not a NetBSD invention; same text in NetBSD 1.1 and OpenBSD 1996 | Regents 1992 (James da Silva, U. Maryland, contributor), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.0, unmodified | `bin/expr` (679 lines) | clear: public domain (Pace Willisson) + Conklin's rewrite; the manual page is separately copyrighted | public domain (code: Willisson; Conklin's rewrite, no notice) / Winning Strategies, Inc. 1993, four-clause, advertising clause (expr.1) (header checked against the donor's original, 2026-11-01, patch 275) | -- |
| NetBSD | 1.2, rev 1.11 (surveyed against 1.0's rev 1.9, functionally identical) | `sys/kern/subr_rmap.c` | **no readable Net/2 text to compare** (`a2-files.md`): Lite2's file is stubs, 386BSD has none. 11 of 130 lines (8%) are Lite2's stub's. Rests on the author's statement in the file (written from how swap uses the interface and from `sys/map.h`'s comments) and `clist.md`'s survey. The Regents/USL notice A3 names is still to be settled | Wolfgang Solfrank / TooLs GmbH 1992, 1994, four-clause, advertising clause (TooLs GmbH acknowledgement); byte for byte NetBSD 1.2's but for this tree's note, no Lite2 text left, so Lite2's Regents/USL notice covers nothing in it and is not carried (header checked against the donor's original, 2026-11-01, patch 275) | #279 |
| FreeBSD | 2.0, rev 1.5 (Demetriou 1994), two lines adapted | `kern/kern_acct.c` | **no readable Net/2 text to compare** (`a2-files.md`): `acct` and `acct_process` were withheld in Net/2 itself (386BSD 0.0 and 0.1 still say `Body deleted`), so no public body existed to copy; Demetriou wrote them for NetBSD in 1994. 62 of 192 lines (32%) are Lite2's stub's, 0 beyond. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou 1994 + Regents 1982-1993 + USL, one block under the Regents' four-clause licence, advertising clause (Berkeley acknowledgement) (header checked against the donor's original, 2026-11-01, patch 275) | 2a3443d3 |
| NetBSD | 1.0, rev 1.20 (Demetriou 1994), three mechanisms adapted | `kern/kern_physio.c` | **measured, 2026-11-01** (`a2-files.md`): 1 line in common with 386BSD's `kern__physio.c` (Jolitz's notice) beyond Lite2's stub, `return (EFAULT);`; the Net/2 original cannot be read. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou 1994 + Regents 1982-1993 + USL, one block under the Regents' four-clause licence, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | b27b80dd |
| NetBSD | 1.0 with 1.1's corrections (patch 248) | `kern/vfs_bio.c` | **measured, 2026-11-01** (`a2-files.md`): 35 of 428 lines (8%) were in 386BSD 0.0's `vfs__bio.c` and not Lite2's stub -- 18 of comment text, reworded in patch 265 with `vfs_bio.o` byte-identical before and after; 17 left, all interface lines Lite2's `<sys/buf.h>` and `<sys/vnode.h>` fix. `bufcache.md` is the defects record. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou 1994 + Regents 1982-1993 + USL, one block under the Regents' four-clause licence, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | c02f7fce; 248 |
| FreeBSD | 2.0, rev 1.7 (Greenman 1994) | `kern/tty_subr.c` | not measured: neither 386BSD has a `tty_subr.c` to compare with (`clist.md`); `ndqb` is this port's own writing (FreeBSD 2.0 has none; an earlier draft said "stubbed", wrongly). Its note called it interim, superseded by patch 262's rule; decided 2026-11-01 (patch 266) that this is the 4.5BSD state, a donor `ndqb` left to 4.6BSD (`a2-files.md`). **Missing from this ledger and from `imports.md` until 2026-11-01** | Greenman 1994 permissive ("may be used, modified, copied, distributed, and sold"), no clauses, no advertising clause; plus the Regents'/USL's four-clause, advertising clause (Lite2's file header, kept) (header checked against the donor's original, 2026-11-01, patch 275) | 3b761a25 |
| NetBSD | 1.0 (kern_exec.c rev 1.55, exec_subr.c rev 1.8, exec_script.c rev 1.8.2.1, exec_script.h rev 1.5, exec.h rev 1.43.2.2) with 1.1's fixes; exec_conf.c from 1.1 (rev 1.14) | `sys/kern/kern_exec.c`, `exec_subr.c`, `exec_conf.c`, `exec_script.c`; `sys/exec_script.h`; the framework in `sys/exec.h` | **measured** (`exec-stack.md`): of 373 distinct code lines in the four `.c` files, 2 are also in 386BSD 0.0's `kern_execve.c` and 4 in 0.1's, all generic; none of the body it replaces survives except lines NetBSD's text has in its own right. The encumbered 4.4BSD file is not compared (never read); clear on what can lawfully be compared | Demetriou four-clause, advertising clause (exec_subr.c, exec_conf.c, exec_script.c, exec_script.h); kern_exec.c: Demetriou + Solfrank + TooLs GmbH under TooLs GmbH's four-clause, advertising clause; exec.h: Demetriou + de Raadt + Regents + USL under the Regents' four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | 262 (replacing #204, #206) |
| Kernighan (onetrueawk) | 20121220, unmodified | `usr.bin/awk` | clear: Lucent's 1997 permission notice, no advertising clause; replaced the gawk Lite2 itself shipped | Lucent Technologies 1997 permission notice (MIT/HPND-style; no advertising clause, only a no-use-of-name-in-advertising condition) (header checked against the donor's original, 2026-11-01, patch 275) | #154 |
| NetBSD | 1.0, NetBSD's later fixes | `usr.sbin/cron`, `usr.bin/crontab` | clear: Vixie's cron 3.0; Berkeley's was AT&T's, cut from Lite2 | Vixie's notice (distribute freely; keep name; mark changes; no advertising clause); popen.c: Regents 1988 (Ken Arnold) one-paragraph licence with advertising-materials acknowledgement (header checked against the donor's original, 2026-11-01, patch 275) | #181 |
| NetBSD | 1.0, NetBSD's + OpenBSD's later fixes | `usr.bin/at`, `libexec/atrun` | clear: Koenig's `at` + Parsons's changes; Berkeley's AT&T's, cut from Lite2 | Koenig 1993 (+ Parsons 1993 on at.c, parsetime.c), two-clause, no advertising clause; Demetriou 1993 four-clause, advertising clause (at.1, atrun.8, at/pathnames.h, atrun.h) (header checked against the donor's original, 2026-11-01, patch 275) | #181 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.sbin/sa`, `usr.sbin/ac` | clear: Demetriou's `sa` (1994) + Gerraty's `ac`; Berkeley's AT&T's, cut from Lite2 except `ac.8`/`accton` | sa: Demetriou 1994 four-clause, advertising clause; ac.c: Demetriou + Gerraty 1994 own two-condition notice, no advertising clause; ac.8: Regents + Gerraty + Demetriou, four-clause, advertising clause (two acknowledgements), as NetBSD 1.3's merged page (header checked against the donor's original, 2026-11-01, patch 275) | #187 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.bin/units` | clear: Adrian Mariano's `units` (1993); Berkeley's was V7's, cut from Lite2 | Mariano 1993, two-clause, no advertising clause; pathnames.h: Demetriou 1993 four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #190 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.sbin/quot` | clear: Wolfgang Solfrank's `quot` (TooLs GmbH, 1991/1994); Berkeley's was 32V's, cut from Lite2 | Solfrank / TooLs GmbH 1991, 1994, four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #192 |
| NetBSD | 1.1, NetBSD's + OpenBSD's later fixes | `sbin/fsdb` | clear: John T. Kohl's `fsdb` (1995); Berkeley's was Computer Consoles' code (1988), AT&T-marked, cut from Lite2 | Kohl 1995, three-clause, no advertising clause (binary also links this tree's Regents-licensed fsck/ffs sources) (header checked against the donor's original, 2026-11-01, patch 275) | #193 |

## Kind F -- Berkeley's own file, restored

| Donor (route) | Version | File | Clearance | Licence/notice | Patch |
|---|---|---|---|---|---|
| NetBSD (route), originally Berkeley's | Lite's own `compress` + NetBSD 1.0's `-b` fix (`zopen.c`, Hannum, via Berkeley 8.2, Apr 1995) | `usr.bin/compress` | clear: redistributable-class in Berkeley's own tree; left out of Lite2 for the LZW patent, not licence | Regents (contributors Spinellis, Woods, Thomas, Orost), four-clause, advertising clause (header checked against the donor's original, 2026-11-01, patch 275) | #189 |
| OpenBSD (route), originally Berkeley's 4.4 | OpenBSD's first commit, 28 Feb 2002 (Todd Miller) | `usr.bin/deroff` | clear: Berkeley's was AT&T-proprietary-marked 1991, cut from Lite2; Caldera's 2002 licence freed it | Regents (deroff.c 1988, 1993; deroff.1 1990, 1993) four-clause, advertising clause + Caldera International 2001-2002, four-clause, advertising clause ("developed or owned by Caldera International, Inc.") (header checked against the donor's original, 2026-11-01, patch 275) | #195 |
| NetBSD (route), originally Berkeley's 4.4 | NetBSD's netbsd-4 branch, 30 Jun 2005 (Perry Metzger) | `games/ching` | clear: Berkeley's AT&T-marked, cut from Lite2; NetBSD's carries Caldera's notice beside the Regents' | Caldera International 2001-2002, four-clause, advertising clause, + Regents 1993 three-clause (ching.6, ching.sh, hexagrams; macros Caldera only); Regents 1988/1990, 1993 (Guy Harris) four-clause, advertising clause (ching.h, cno, phx) (header checked against the donor's original, 2026-11-01, patch 275) | #196 |
| FreeBSD (route), originally Berkeley's own program | FreeBSD 2.2.8's tree, sendmail 8.8.8 (Oct 1997, last release under Berkeley's terms) | `usr.sbin/sendmail` | clear: Berkeley's program at its last release under its own terms; Lite2's README withheld the beta at the developers' request, not for licence | Regents + Eric P. Allman, four-clause, advertising clause; contrib/ and a few files carry third-party notices as shipped (MIT, CMU, Vinopal, Sharnoff, Beck, U. Illinois/Pomes, Sanders, Motorola), several with advertising clauses (header checked against the donor's original, 2026-11-01, patch 275) | #182 |

## Previously here, now removed

| Donor | Version | File | Note |
|---|---|---|---|
| OpenBSD | 25 Jun 2003 import of Berkeley's 4.3-Reno `diff` and 31 Jul 2003 `diff3prog`, under Caldera's licence | `usr.bin/diff` | present patches 155-157; removed by patch 157 (an alternative not derived from Caldera-licensed code preferred); GNU diff 2.6 (kind D, above) took its place. Not a current donor file. |
| Berkeley's own, from the history (8.1, 1993, marked proprietary) -- not OpenBSD's | -- | `diff3.sh` | Caldera's licence was put on it by patch 156 with no source: OpenBSD never shipped it (its front end is Todd Miller's diff3.ksh). Removed by patch 157. (Row split and corrected 2026-11-01, patch 275.) |

## What this ledger shows, plainly

AI-ONLY NOTE: **the counts from here to "No row is HOLD any more" are
history (patches 259 and 262) and are superseded by the script count
below (patch 265; 63 rows since patch 272), no HOLD.**

AI-ONLY NOTE (2026-10-31, patch 262): two rows added since the count
below -- `kern/exec_elf.c` (kind D, never listed before; measured,
clear) and NetBSD's exec framework (kind E; measured, clear) -- making
56 rows, 24 with a stated finding; the 3 split, 3 HOLD and 26 not yet
checked are unchanged. The count below is patch 259's, kept as made.

54 donor-file rows above (40 kind D, 10 kind E, 4 kind F). Counted by
hand, not estimated:

- **22 carry an actual stated clearance finding**: the `wd.c` family,
  the slice layer's three `.c` files, and `kvm_i386.c` (measured or
  clear by construction, `disk-wd.md`), the `ddb` files, `bc`/`dc` and
  `uucp` (clear because the licence is CMU's or GPL's and the question
  of Net/2 residue in a Berkeley-licensed file does not arise), and
  all of kind E's near-uniform findings except `subr_rmap.c` --
  `regex.c`, `bin/expr`, `awk`, `cron`/`crontab`, `at`/`atrun`,
  `sa`/`ac`, `units`, `quot`, `fsdb` -- plus kind F's four
  (`compress`, `deroff`, `ching`, `sendmail`).
- **3 are split**: `gdb-4.16`, `gcc-2.95.3`, and `binutils-2.11.2` are
  each clear for GNU's own released text (third-party, no Net/2
  question) but not yet measured for the donor's own changes to that
  text, which do carry a kind-D exposure of their own.
- **No row is HOLD any more** (2026-11-01, `a2-files.md`).
  `sys_process.c`, `procfs_machdep.c` and `subr_rmap.c` were; the first
  is measured clear, the second likely clear, the third rests
  on its author's statement because nothing readable can be compared.
- **The remaining are "not yet checked"**: `imports.md` says where
  each one came from, fluently, but nothing in this tree has run rule
  2/3's test against them. That gap, not this table's existence, is
  what A1 still owes.

**Recounted by script, 2026-11-01 (patch 265)**, the hand counts above
not having been reproducible: the three kind tables held **60 rows**
(41 kind D, 15 kind E, 4 kind F); patch 267 added the `isa.c` DMA row
(kind D), making **61** (42 D, 15 E, 4 F). Patches 270 and 272 added two
more (NetBSD 1.0's `microtime.s` and `maxsmap`), and patch 271 replaced
the `sys_process.c` row, making **63**. Four were added today --
`kern_acct.c`, `kern_physio.c`, `vfs_bio.c` and `tty_subr.c`, which
`imports.md` had never listed and so the ledger, built from it, had
not either (`a2-files.md`) -- and the three HOLD rows were resolved.
Of the 60 counted then, **27 said "not yet checked" or "not measured"** in their
clearance cell (two of them, `gcc` and `binutils`, are among the 3
split rows); the other 33 carry a measurement, a settled finding, a
clear-by-construction reason, or a stated reason nothing can be
compared. The paragraphs above are patch 259's and 262's counts, kept
as made; this one supersedes them.

An earlier draft of this summary said "nine" had a stated finding and
counted `subr_rmap.c` among them; both were wrong -- the first from
undercounting kind E's own near-uniform "clear" verdicts, the second
from treating a HOLD as if it were a verdict. Corrected 2026-10-31
after independent review.
