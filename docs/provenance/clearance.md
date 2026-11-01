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
| NetBSD | 1.6 | `lib/csu/*` (619 lines) | not yet checked; `imports.md` gives no USL-clearance statement, only that ELF startup code could not have existed in 1995 | -- | -- |
| NetBSD | 1.0, adapted | `lib/libkvm/kvm_i386.c` (221 lines) | clear by construction: ports Berkeley's own `kvm_hp300.c` (Regents copyright) to a machine Berkeley never supported; in no CSRG tree at all | Regents (via NetBSD) | -- |
| FreeBSD | 2.0.5 | `vmstat/names.c`, i386 case (23 lines) | not yet checked | -- | #221 |
| NetBSD | 1.0 | `lib/libl/Makefile` (4 lines) | not yet checked | -- | -- |
| NetBSD | 1.3, adapted | `lib/libc/gen/nlist_elf32.c` | not yet checked | Demetriou's four-clause, advertising clause | -- |
| NetBSD | 1.3, adapted | `usr.sbin/kvm_mkdb/nlist_elf32.c` | not yet checked | Demetriou's four-clause, advertising clause | -- |
| NetBSD | 1.1, rev 1.3 (Christos Zoulas); adapted, then whole less its dynamic-linking and emulation parts (patch 262) | `kern/exec_elf.c` | **measured** (`exec-stack.md`): 0 of its 91 distinct code lines in 386BSD 0.0's or 0.1's `kern_execve.c`; clear on what can lawfully be compared. **Missing from this ledger until 2026-10-31**: `imports.md` had never listed it, and the ledger was built from `imports.md` | Zoulas's three-clause | #202/#203; 262 |
| NetBSD | 1.3 (`Shdr`/`Sym`, rev 1.1 from NetBSD 1.1 for the header) | `sys/exec_elf.h` | not yet checked | Demetriou's four-clause, advertising clause | -- |
| FreeBSD | 4.1, adapted | `sys/boot/i386` (`boot1.s`, `boot2.c`, `lib.h`, `sio.s`, `btx*.s`/`.h`) | not yet checked | Nordier's own (notice + disclaimer kept) | -- |
| FreeBSD | 4.1, adapted | `usr.sbin/btxld` | not yet checked | two-clause BSD | -- |
| FreeBSD | 2.0.5 | `i386/autoconf.c`: `MFS_ROOT` (4 statements) | not yet checked | -- | -- |
| FreeBSD | 2.0.5, adapted | `release/write_mfs_in_kernel.c` | not yet checked | Poul-Henning Kamp's "beer-ware" notice | -- |
| FreeBSD | 3.0, adapted | `usr.sbin/crunch` (`crunchgen`, `crunchide`) | not yet checked | U. Maryland permission notice; Demetriou's (crunchide back end) | -- |
| NetBSD | 1.1, adapted | `distrib/i386/floppies` | not yet checked | Demetriou's four-clause, advertising clause | -- |
| FreeBSD | 2.1.0, adapted | `i386/isa/wd.c` | **settled, 2026-10-30** (`disk-wd.md`): no Net/2-era text Berkeley changed or dropped survives but the copyright notice; 7% is 386BSD 0.1's own (not Net/2 text); the rest is FreeBSD's own; clear, residue named | Regents (via Jolitz's contribution) | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/wdreg.h` | **measured** (`disk-wd.md`): 2 of 112 lines early-not-Lite2 (the copyright notice only); clear | Regents | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/atapi.c` | **measured** (`disk-wd.md`): 0 lines in 386BSD 0.0 at all; clear | Cronyx (Serge Vakulenko) permission notice | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/wcd.c` | **measured** (`disk-wd.md`): 3 of 523 lines in 386BSD 0.0 (negligible); clear. Bounds-check fix applied, patch 257 | Cronyx (Serge Vakulenko) permission notice | #318/#319; fix patch 257 |
| FreeBSD | 2.1.0, adapted | the slice layer: `kern/subr_diskslice.c`, `kern/subr_dkbad.c`, `i386/isa/diskslice_machdep.c` | **measured** (`disk-wd.md`): 2 lines each early-not-Lite2 (copyright notice only); clear | Bruce Evans, 1994 | #318/#319 |
| FreeBSD | 2.1.0, adapted | `<sys/diskslice.h>` | not yet checked -- `disk-wd.md`'s measured table covers the three `.c` files above but not this header; grouping it with them was an overstatement, corrected 2026-10-31 | -- | #318/#319 |
| FreeBSD | 2.1.0, adapted | `usr.sbin/config`: `isa_biotab()` | not yet checked (same release as the `wd.c` family it accompanies, but not itself measured) | per-drive table in Lite2's `isa_device` layout | #318/#319 |
| FreeBSD | 2.1.0, adapted | `i386/isa/if_ed.c`, `if_edreg.h` | not yet checked (`if_ed-assign.md` is a later bugfix, not a clearance measurement) | David Greenman's own notice | -- |
| NetBSD | 1.0 | `lib/libcompat/4.1`: `gtty`, `stty`, `ftime` | not yet checked | -- | -- |
| NetBSD | 1.0 | `usr.bin/m4/m4.1` | not yet checked | -- | -- |
| FreeBSD | 2.2, rev 1.24 (Fagan 1994), adapted | `kern/sys_process.c` | **measured, 2026-11-01** (`a2-files.md`): 16 of 182 lines (9%) are in 386BSD's `sys_process.c` (Net/2's body, 0.1) and not Lite2's stub -- all interface (`ptrace`'s signature, `case PT_...:` labels, a few one-line idioms); clear on this test. The Net/2 share fell 51%, 30%, 18%, 8% through FreeBSD 1.0, 1.1.5.1, 2.0 and 2.2 as Fagan's rewrite replaced it; NetBSD 1.0's is 4%. First-fit replay (NetBSD 1.0's) is stage 4, #380 | Sean Eric Fagan 1994, four-clause, advertising clause | #375 |
| NetBSD (as it had it, Jan 1994), adapted | -- | `i386/i386/procfs_machdep.c` | **likely clear, 2026-11-01** (`a2-files.md`; a sibling of Lite2's own files, nothing comparable readable): Pendry's own `/proc` code for 4.4BSD, of which Lite2 ships two siblings (`hp300`, `pmax`) under the same Regents-and-Pendry notice; 45% of its lines are in `hp300`'s; 386BSD, which carries Net/2's text, has no `/proc`. Not a settlement body | Regents and Pendry's | #375 |
| FreeBSD | 2.2, adapted | `i386/i386/machdep.c`: `ptrace_set_pc`, `ptrace_write_u` | not yet checked | -- | -- |
| FreeBSD | 2.2 | `i386/include/ptrace.h`: `PT_GETREGS` family | not yet checked | -- | -- |
| NetBSD (1993) / FreeBSD 2.1.0 | -- | `i386/include/reg.h`: `struct reg` (NetBSD), `struct fpreg` (FreeBSD) | not yet checked | -- | -- |
| FreeBSD | 2.2, adapted | `lib/libc/sys/ptrace.2` | not yet checked | -- | -- |
| FreeBSD | 5.0, unmodified | `kern/genassym.sh`, `sys/assym.h` | not yet checked. **AI-ONLY NOTE:** no reason is stated in `imports.md` for reaching to FreeBSD 5.0 specifically -- every other late-reaching donor choice in that file gives one (ELF `csu` needing a modern crtbegin, `boot2` needing `.code16`-capable assembly); this one does not. Flagged, not guessed at. | -- | -- |
| NetBSD | 1.0's system `diff` | `contrib/diff-2.6` (GNU diff 2.6) | not yet checked for the NetBSD wrapper/Makefile; the GNU code itself is third-party, out of this project's BSD-native scope (`roadmap-audit.md` item 2) | GPL | -- |
| NetBSD | 1.1's system `grep` | `contrib/grep-2.0` (GNU grep 2.0) | same as above | GPL | -- |
| NetBSD | 1.1's system `bc`/`dc` | `contrib/bc-1.03` (GNU bc 1.03 + dc) | **detailed** (`imports.md`): byte-for-byte NetBSD 1.1's import (Phil Nelson, 1 Dec 1994); Berkeley's own `bc`/`dc`/`libmp` were AT&T's, cut from Lite2 | GPL v2 | #194 |
| NetBSD | 1.1's system `uucp` | `contrib/uucp-1.06.1` (Taylor UUCP) | **detailed** (`imports.md`): byte-for-byte NetBSD 1.1's import; Berkeley's `uucp` AT&T-marked 1991, cut from Lite2 | GPL | #197/#198 |
| NetBSD | 1.0, adapted (CMU Mach 3.0 origin) | `sys/ddb`, i386 `db_*.c`, `db_machdep.h` (28 files) | **detailed** (`imports.md`): CMU's licence alone, no USL exposure since the text is CMU/Mach-origin, not Berkeley/Net2; cross-checked against CMU Mach 3.0 directly | Carnegie Mellon | #199 |
| NetBSD | 1.0, unmodified | `share/man/man4/ddb.4` | not yet separately checked; presumably the ddb files' own clearance | CMU (via NetBSD) | #199 |
| NetBSD | 1.3's `db_elf.c` (interface of rev 1.17/1.14) | `sys/ddb`: ELF symbol reading | not yet checked | NetBSD/Demetriou lineage (same family as `nlist_elf32.c`) | #201 |
| GNU release + FreeBSD 2.2.8's changes and glue | gdb 4.16 | `contrib/gdb-4.16` (1820 files) | kind A for GNU's own release (third-party, out of scope); kind D for FreeBSD's twelve changed files plus FreeBSD's own glue (BSD makefiles, hand-made `config.h`/`bfd.h`, `i386b-nat.c`, `core-aout.c`, `kvm-fbsd.c`) | GPL (GNU) + Berkeley/FreeBSD on the glue | #204-#206 |
| GNU release + NetBSD 1.6's changes, this tree's own glue | gcc 2.95.3 | `contrib/gcc-2.95.3` | not yet checked for lineage; GNU's release is verified byte-identical to its own `md5sum.txt` (third-party, out of scope), NetBSD's `netbsd-1-6` changes (78 files changed, 18 added, 6 removed) are kind D and not yet measured against the settlement; this tree's own system-name/link glue is kind G | GPL v2 (+ LGPL/runtime exception on libraries, file by file) | -- |
| GNU release + NetBSD 1.6's changes, this tree's own glue | binutils 2.11.2 | `contrib/binutils-2.11.2` | not yet checked for lineage; GNU's release verified against its own release + the 2.11.2a re-release GNU's key signs (third-party, out of scope), NetBSD's `netbsd-1-6` changes (114 changed, 30 added) are kind D and not yet measured; this tree's own glue is kind G | GPL v2 | -- |

## Kind E -- a reimplementation of something the Lite cut removed

| Donor | Version | File | Clearance | Licence/notice | Patch |
|---|---|---|---|---|---|
| NetBSD | 1.0 | `lib/libcompat/4.3/regex.c` (93 lines) | clear: Berkeley-licensed (James da Silva, U. Maryland, contributed to Berkeley), not a NetBSD invention; same text in NetBSD 1.1 and OpenBSD 1996 | Regents | -- |
| NetBSD | 1.0, unmodified | `bin/expr` (679 lines) | clear: public domain (Pace Willisson) + Conklin's rewrite; the manual page is separately copyrighted | public domain (code) / four-clause w/ advertising (page, Winning Strategies Inc. 1993) | -- |
| NetBSD | 1.2, rev 1.11 (surveyed against 1.0's rev 1.9, functionally identical) | `sys/kern/subr_rmap.c` | **no readable Net/2 text to compare** (`a2-files.md`): Lite2's file is stubs, 386BSD has none. 11 of 130 lines (8%) are Lite2's stub's. Rests on the author's statement in the file (written from how swap uses the interface and from `sys/map.h`'s comments) and `clist.md`'s survey. The Regents/USL notice A3 names is still to be settled | Wolfgang Solfrank / TooLs GmbH, 1992/1994, four-clause, advertising clause | #279 |
| FreeBSD | 2.0, rev 1.5 (Demetriou 1994), two lines adapted | `kern/kern_acct.c` | **no readable Net/2 text to compare** (`a2-files.md`): `acct` and `acct_process` were withheld in Net/2 itself (386BSD 0.0 and 0.1 still say `Body deleted`), so no public body existed to copy; Demetriou wrote them for NetBSD in 1994. 62 of 192 lines (32%) are Lite2's stub's, 0 beyond. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou 1994 and the Regents', three-clause | 2a3443d3 |
| NetBSD | 1.0, rev 1.20 (Demetriou 1994), three mechanisms adapted | `kern/kern_physio.c` | **measured, 2026-11-01** (`a2-files.md`): 1 line in common with 386BSD's `kern__physio.c` (Jolitz's notice) beyond Lite2's stub, `return (EFAULT);`; the Net/2 original cannot be read. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou 1994 and the Regents', three-clause | b27b80dd |
| NetBSD | 1.0 with 1.1's corrections (patch 248) | `kern/vfs_bio.c` | **measured, 2026-11-01** (`a2-files.md`): 35 of 428 lines (8%) were in 386BSD 0.0's `vfs__bio.c` and not Lite2's stub -- 18 of comment text, reworded in patch 265 with `vfs_bio.o` byte-identical before and after; 17 left, all interface lines Lite2's `<sys/buf.h>` and `<sys/vnode.h>` fix. `bufcache.md` is the defects record. **Missing from this ledger and from `imports.md` until 2026-11-01** | Demetriou/Solfrank-era NetBSD notice | c02f7fce; 248 |
| FreeBSD | 2.0, rev 1.7 (Greenman 1994) | `kern/tty_subr.c` | not measured: neither 386BSD has a `tty_subr.c` to compare with (`clist.md`); a `ndqb` is stubbed again. Its note calls it an interim state to be replaced by writing, which patch 262's rule has superseded (`a2-files.md`). **Missing from this ledger and from `imports.md` until 2026-11-01** | Greenman's own ("may be used ... sold") | 3b761a25 |
| NetBSD | 1.0 (kern_exec.c rev 1.55, exec_subr.c rev 1.8, exec_script.c rev 1.8.2.1, exec_script.h rev 1.5, exec.h rev 1.43.2.2) with 1.1's fixes; exec_conf.c from 1.1 (rev 1.14) | `sys/kern/kern_exec.c`, `exec_subr.c`, `exec_conf.c`, `exec_script.c`; `sys/exec_script.h`; the framework in `sys/exec.h` | **measured** (`exec-stack.md`): of 373 distinct code lines in the four `.c` files, 2 are also in 386BSD 0.0's `kern_execve.c` and 4 in 0.1's, all generic; none of the body it replaces survives except lines NetBSD's text has in its own right. The encumbered 4.4BSD file is not compared (never read); clear on what can lawfully be compared | Demetriou's four-clause, advertising clause; Solfrank / TooLs GmbH on `kern_exec.c`; de Raadt and the Regents on `exec.h` | 262 (replacing #204, #206) |
| Kernighan (onetrueawk) | 20121220, unmodified | `usr.bin/awk` | clear: Lucent's 1997 permission notice, no advertising clause; replaced the gawk Lite2 itself shipped | Lucent 1997 | #154 |
| NetBSD | 1.0, NetBSD's later fixes | `usr.sbin/cron`, `usr.bin/crontab` | clear: Vixie's cron 3.0; Berkeley's was AT&T's, cut from Lite2 | Vixie's notice | #181 |
| NetBSD | 1.0, NetBSD's + OpenBSD's later fixes | `usr.bin/at`, `libexec/atrun` | clear: Koenig's `at` + Parsons's changes; Berkeley's AT&T's, cut from Lite2 | Koenig/Parsons, via NetBSD | #181 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.sbin/sa`, `usr.sbin/ac` | clear: Demetriou's `sa` (1994) + Gerraty's `ac`; Berkeley's AT&T's, cut from Lite2 except `ac.8`/`accton` | Demetriou/Gerraty | #187 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.bin/units` | clear: Adrian Mariano's `units` (1993); Berkeley's was V7's, cut from Lite2 | Mariano's | #190 |
| NetBSD | 1.0, three BSDs' later fixes | `usr.sbin/quot` | clear: Wolfgang Solfrank's `quot` (TooLs GmbH, 1991/1994); Berkeley's was 32V's, cut from Lite2 | Solfrank/TooLs GmbH, four-clause, advertising clause | #192 |
| NetBSD | 1.1, NetBSD's + OpenBSD's later fixes | `sbin/fsdb` | clear: John T. Kohl's `fsdb` (1995); Berkeley's was Computer Consoles' code (1988), AT&T-marked, cut from Lite2 | Kohl's three-clause | #193 |

## Kind F -- Berkeley's own file, restored

| Donor (route) | Version | File | Clearance | Licence/notice | Patch |
|---|---|---|---|---|---|
| NetBSD (route), originally Berkeley's | Lite's own `compress` + NetBSD 1.0's `-b` fix (`zopen.c`, Hannum, via Berkeley 8.2, Apr 1995) | `usr.bin/compress` | clear: redistributable-class in Berkeley's own tree; left out of Lite2 for the LZW patent, not licence | Regents | #189 |
| OpenBSD (route), originally Berkeley's 4.4 | OpenBSD's first commit, 28 Feb 2002 (Todd Miller) | `usr.bin/deroff` | clear: Berkeley's was AT&T-proprietary-marked 1991, cut from Lite2; Caldera's 2002 licence freed it | Regents + Caldera (2002) | #195 |
| NetBSD (route), originally Berkeley's 4.4 | NetBSD's netbsd-4 branch, 30 Jun 2005 (Perry Metzger) | `games/ching` | clear: Berkeley's AT&T-marked, cut from Lite2; NetBSD's carries Caldera's notice beside the Regents' | Regents + Caldera | #196 |
| FreeBSD (route), originally Berkeley's own program | FreeBSD 2.2.8's tree, sendmail 8.8.8 (Oct 1997, last release under Berkeley's terms) | `usr.sbin/sendmail` | clear: Berkeley's program at its last release under its own terms; Lite2's README withheld the beta at the developers' request, not for licence | Regents | #182 |

## Previously here, now removed

| Donor | Version | File | Note |
|---|---|---|---|
| OpenBSD | 25 Jun 2003 import of Berkeley's 4.3-Reno `diff`/`diff3` under Caldera's licence | `usr.bin/diff`, `diff3.sh` | present patches 155-157; removed by patch 157, "an alternative not derived from Caldera-licensed code is preferred"; GNU diff 2.6 (kind D, above) took its place. Not a current donor file. |

## What this ledger shows, plainly

AI-ONLY NOTE: **the counts from here to "No row is HOLD any more" are
history (patches 259 and 262) and are superseded by the script count
above (patch 265): 60 rows, no HOLD.**

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
not having been reproducible: the three kind tables hold **60 rows**
(41 kind D, 15 kind E, 4 kind F). Four were added today --
`kern_acct.c`, `kern_physio.c`, `vfs_bio.c` and `tty_subr.c`, which
`imports.md` had never listed and so the ledger, built from it, had
not either (`a2-files.md`) -- and the three HOLD rows were resolved.
Of the 60, **27 say "not yet checked" or "not measured"** in their
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
