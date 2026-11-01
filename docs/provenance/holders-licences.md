# Holders and licences (A3) and our own files (A4): measurement

Stage 3, item 6 of `docs/plan-4.5.md`; the definitions of A3 and A4 are
in `docs/roadmap-audit.md`. This note is a measurement only. No file
other than this one was changed. Measured on branch `p242` at
`6509bf75` (2026-11-01). **Every count is of that tree, before patch
267**, which then fixed the five wrong ledger cells, gave the five code
files and `devices.spec` their SPDX line (own set then 61 with the line,
72 without) and added one ledger row (61); the "recommended edits" it
lists for those are done (end of this note). The "376 files with an
AI-ONLY NOTE" was recounted by the reviewer as 391 and is unreliable.

**AI-ONLY NOTE:** every number below was computed by a script run
against the working tree; the method and what could not be checked are
at the end. Where a verdict is a reading of a header and not a count,
the table says "reading".

## Summary

**A3, the 60 ledger rows** (`docs/provenance/clearance.md`; 41 kind D,
15 kind E, 4 kind F, as the ledger itself counts):

| header against the ledger's "Licence/notice" cell | rows |
|---|---|
| consistent (ok) | 28 |
| consistent, but a holder in the header is not in the cell (ok/HOLDER) | 1 |
| holder missing from the cell (HOLDER) | 4 |
| cell is "--" or names no holder, while the header names one (GAP) | 8 |
| partly recorded: the holder is there, a clause or a second holder is not (PARTIAL) | 11 |
| **cell and header disagree (MISMATCH)** | **5** |
| no copyright or licence text in the file at all (NOHDR) | 3 |

**Not measured at all: whether any header matches the donor's
original.** 0 of 60. The donor trees on disk are git repositories and
this session could not run git against them (see "What I could not
check"). Every "match" in this note is header against ledger, not
header against donor.

The five mismatches, each in the table with its evidence:

1. **Row 8, `sys/exec_elf.h`.** Ledger: "Demetriou's four-clause,
   advertising clause". Header: Christos Zoulas 1994, three-clause, and
   the file's own note says "Three-clause BSD, Christos Zoulas 1994."
   No Demetriou notice is in the file. Either the cell is wrong or the
   Shdr/Sym lines from NetBSD 1.3 need his notice.
2. **Row 38, `sys/ddb/db_elf.c`.** Ledger: "NetBSD/Demetriou lineage".
   Header: The NetBSD Foundation 1997, four-clause with the advertising
   clause. No Demetriou line.
3. **Rows 45, 46, 47, `kern_acct.c`, `kern_physio.c`, `vfs_bio.c`.**
   Ledger (45, 46): "Demetriou 1994 and the Regents', three-clause".
   Each header is the Regents/USL four-clause block of the Lite2 stub
   (clause 3 is the advertising clause; the "(c) UNIX System
   Laboratories" paragraph is there) with Demetriou's one line added
   above it. As written the file offers Demetriou's code under the
   Regents' four clauses. Whether his original was three-clause was not
   checked (donors unreadable here); `kern_acct.c`'s own note says it
   was.

**Advertising clause.** 39 of the 60 rows have at least one file whose
header carries clause 3 (a "All advertising materials mentioning..."
clause). Where is it recorded?

- 11 rows: the files are Regents text only (or GNU trees where, by the
  script's list of holders, the only advertising files are Regents'); `./COPYRIGHT` reproduces the
  Regents' clause 3. Rows 2, 3, 11, 15, 16, 21, 27, 28, and 39, 40, 41.
- 8 rows: the ledger cell says "four-clause" or "advertising clause"
  (5, 6, 14, 25, 43, 44, 49, 55).
- **20 rows: in neither.** 1, 13, 19, 23, 26, 29, 36, 38, 42, 45, 46,
  47, 48, 52, 53, 54, 57, 58, 59, 60. Rows 26 and 42 are Regents text
  with a Pendry or University of Maryland attribution in the notice,
  and may be counted as covered by COPYRIGHT; I did not. A script that
  searched the other provenance files for the clause beside the file's
  name matched too freely (short names such as `sa`, `at`, `units`),
  so I do not claim any of the 20 is recorded elsewhere. The new
  fact among them is that **Caldera's own 2001-2002 licence (rows 58,
  59, `deroff`, `ching`) is a four-clause licence with an advertising
  clause** ("developed or owned by Caldera International, Inc."), which
  no record states.

**`./COPYRIGHT` names one holder.** It is the Lite2 file unchanged
(the last commit to touch it is the import commit `b3d2855f`). It names
the Regents, the IEEE and X3/CBEMA permissions, and it reproduces the
Regents' four clauses. It names no other holder and no other
licence. Outside `build/` and `docs/`, 27,617 tracked text files were
scanned; 14,355 have a copyright line in their first 80 lines and 8,938
of those name a holder other than the Regents by my parse (275 distinct
strings; the largest are the Free Software Foundation 4,883, MIT 237,
Ian Lance Taylor 175, Eric P. Allman 165, the University of Utah 150,
Pendry 142). Most are Lite2's own `contrib` and sub-licences, not this
project's imports; the parse is rough and the strings are in the
appendix output, not a list to paste.

**A3's named items** (`roadmap-audit.md`), whether the holder is in the
file now:

(This table is the survey's, as measured before patch 275; the column
"in the file now" was true then. What patch 275 did with each item is
in the table under "A3 finished", at the end of this note.)

| file | holder asked for | in the file now (before patch 275) |
|---|---|---|
| `sys/i386/i386/locore.s` | Hannum | no |
| `sys/i386/include/cpu.h` | Hannum | no |
| `sys/i386/isa/npx.c` | Hannum | no |
| `sys/boot/i386/boot2/boot2.c` | The NetBSD Foundation | no (Nordier is) |
| `sys/kern/subr_rmap.c` | the Regents and USL | no (TooLs/Solfrank is) |
| `diff3.sh` | Caldera | the file is gone (patch 157); Caldera's licence was never the donor's for this file (OpenBSD never shipped it); the ledger row is corrected (patch 275) |
| `sys/i386/isa/wd.c`, `wdreg.h` | Regents | yes, both |
| `sys/sys/cdio.h` | (named in A3) | the file has no copyright text at all |
| `lib/libc/gen/nlist.c` | Demetriou 1996 | no (the line is in `nlist_elf32.c`) |

"In the file now" is a search of the first 140 lines of each file for
the name. It says the line is absent, not that the donor's original had
it; that was not checked.

**A4, files we wrote.** The "own" set is every tracked file under
`build/` and `docs/`, the tracked files at the top level other than
`COPYRIGHT`, `.cshrc` and `.profile` (the last two are Lite2's, unchanged
since the import commit), and anything with an `SPDX-FileCopyrightText`
line or the name "Nicholas Martin". That is 144 files, of which 14 are
`docs/reference/test_mk7.3/`, a snapshot of the maintainer's sibling
project. For the other **130 files**:

| kind | with the SPDX copyright line | without |
|---|---|---|
| code, scripts, makefiles | 55 | 5 |
| `.want` expected-output fixtures (`build/test`) | 0 | 20 |
| writing (`.md`, `.tsv`) | 0 | 48 |
| dotfiles (`.gitignore`, `build/shim/.gitignore`) | 0 | 2 |
| **all** | **55** | **75** |

- **Licence identifier: none** in any of the 130. A line-anchored
  search for `SPDX-License-Identifier` finds two files in the whole
  tree, both in the reference snapshot: `tools/console.py` carries
  `MIT`, and `AGENTS.md` shows `<expression>` in an example block (so
  only `console.py` asserts a licence). The snapshot's own README says
  that file is the one exception there.
- **The five code files without the line** are exactly the ones
  `roadmap-audit.md` A4 names: `build/mklabel.c`, `build/mkfs-host.c`,
  `build/addfile-host.c`, plus `build/fs-host.h` and `build/shim/shim.ld`.
  `usr/src/distrib/i386/floppies/inst-common/devices.spec` is also ours
  (A4 names it) and has none; the script's own set misses it because it
  has no marker, so that one is from reading the file.
- **All 32 `docs/provenance` files and the 14 other `docs/*.md` files
  lack the line**, as do `README.md` and `REDO.md`: 48 writing files.
  `docs/method.md` and `docs/roadmap-audit.md` are among them (the
  latter mentions the tag in prose only): A4 asks for the line on "the
  `*.md` method files".
- Only one file under `usr/src` carries the line:
  `lib/csu/i386_elf/Makefile`, as `precedent.md` says. 376 other files
  outside `build/` and `docs/` contain an "AI-ONLY NOTE"; none has the
  line. They are donor or Lite2 files with a note added, not files we
  wrote, so I did not count them as ours. Whether the note's author is a
  copyright holder of the file is a maintainer question.
- **COPYRIGHT's statement.** `COPYRIGHT` says "All of the documentation
  and software included in the 4.4BSD and 4.4BSD-Lite Releases is
  copyrighted by The Regents". It says nothing of this project's files.
  `precedent.md` ("What in this tree is ours") says the Berkeley terms
  "cover the Berkeley material and not those files", but that sentence
  is in `precedent.md`, not in `COPYRIGHT`.

## Recommended edits (not made)

1. **`docs/provenance/clearance.md`**: correct rows 8 and 38 (holder
   and clause as the headers read), rows 45-47 (the header block is the
   Regents' four-clause; "three-clause" is the donor's), and add the
   advertising-clause and second-holder facts for the 20 rows listed
   above and the 12 GAP/HOLDER rows (1, 11, 20, 21, 23, 27, 28, 29, 31,
   52, 54, 60). Name Caldera's clause for 58 and 59.
2. **`usr/src/sys/sys/exec_elf.h`** (row 8): decide whether Demetriou's
   notice belongs with the Shdr/Sym lines, or fix the cell.
3. **`kern_acct.c`, `kern_physio.c`, `vfs_bio.c`, `tty_subr.c`**: decide
   whether the header should say which clauses cover which part.
4. **A3's named files** (table above): add the Hannum lines to
   `locore.s`, `cpu.h`, `npx.c`; the NetBSD Foundation notice to
   `boot2.c`; the Regents/USL notice to `subr_rmap.c`; a notice or note
   to `cdio.h`; Demetriou 1996 to `nlist.c` if the donor's file has it.
   Each only after the donor's original is read (not done here).
5. **A4 files**: add `SPDX-FileCopyrightText: 2026 Nicholas Martin` and
   no identifier to the five code files and `devices.spec`. Decide
   separately for the 48 writing files, the 20 `.want` fixtures
   (a comment line may change what a test compares) and the two
   `.gitignore`s.
6. **`COPYRIGHT`**: it can carry a section that names the other holders
   and licences the tree now has, and one sentence that the files this
   project wrote (SPDX copyright line, no identifier) are outside the
   Berkeley terms, which is what `precedent.md` already says.
7. **`docs/reference/test_mk7.3/tools/console.py`**: the only file in
   the tree asserting a licence (`MIT`). Decide whether the snapshot is
   part of this tree's own files.

## The 60 rows

Columns: "Files / with notice" counts text files under the ledger's
path(s) and how many have a copyright, beer-ware or public-domain line
in the first 100 lines (row 1's count includes `i386_elf/Makefile`,
which carries our SPDX line). "Holders found by name" counts files whose
header names a holder from a fixed list of 45 names; a holder not on
the list is not shown. "Licence class" is from the clause text:
`BSD-4clause` = redistribution, an advertising clause and a no-endorse
clause (this includes the Regents' Lite2 text); `BSD-3clause` = no
advertising clause; `BSD-2clause` = neither. The last column is my
reading of the header against the cell.

| # | K | File(s) in ledger | Files / with notice | Holders found by name | Licence class in headers | Ledger cell | Header vs ledger |
|---|---|---|---|---|---|---|---|
| 1 | D | `lib/csu/*` (619 lines) | 8 / 9 | NetBSD Foundation 4, Demetriou 3, Ross Harvey 3, Zoulas 1 | BSD-4clause 7 | -- | **GAP**: cell is "--"; headers name Demetriou 1995, NetBSD Foundation 1998/2001, Ross Harvey 2001, Zoulas 1998; the NetBSD Foundation notices carry the advertising clause. |
| 2 | D | `lib/libkvm/kvm_i386.c` (221 lines) | 1 / 1 | Regents 1 | BSD-4clause 1 | Regents (via NetBSD) | **ok** |
| 3 | D | `vmstat/names.c`, i386 case (23 lines) | 1 / 1 | Regents 1 | BSD-4clause 1 | -- | **ok**: Regents notice; cell "--" |
| 4 | D | `lib/libl/Makefile` (4 lines) | 0 / 1 | -- | -- | -- | **NOHDR**: no copyright or licence text in the file; ledger cell "--" |
| 5 | D | `lib/libc/gen/nlist_elf32.c` | 1 / 1 | Demetriou 1 | BSD-4clause 1 | Demetriou's four-clause, advertising clause | **ok** |
| 6 | D | `usr.sbin/kvm_mkdb/nlist_elf32.c` | 1 / 1 | Demetriou 1 | BSD-4clause 1 | Demetriou's four-clause, advertising clause | **ok** |
| 7 | D | `kern/exec_elf.c` | 1 / 1 | Zoulas 1 | BSD-3clause 1 | Zoulas's three-clause | **ok** |
| 8 | D | `sys/exec_elf.h` | 1 / 1 | Zoulas 1 | BSD-3clause 1 | Demetriou's four-clause, advertising clause | **MISMATCH**: header and in-file note say Zoulas 1994, three-clause; cell says Demetriou, four-clause with advertising. the ledger says the Shdr/Sym part comes from NetBSD 1.3, but no Demetriou notice is in the file |
| 9 | D | `sys/boot/i386` (`boot1.s`, `boot2.c`, `lib.h` | 9 / 14 | Nordier 9 | -- | Nordier's own (notice + disclaimer kept) | **ok**: Nordier 1998 "freely permitted" text kept; 5 of the 14 files have no copyright line (Makefiles, assembler) |
| 10 | D | `usr.sbin/btxld` | 3 / 4 | Nordier 3 | BSD-2clause 3 | two-clause BSD | **ok/HOLDER**: two-clause with Nordier 1998 in 3 of 4 files; Nordier not named in cell |
| 11 | D | `i386/autoconf.c`: `MFS_ROOT` (4 statements) | 1 / 1 | Regents 1, Jolitz 1 | BSD-4clause 1 | -- | **GAP**: cell "--"; Regents notice (four-clause) in a Lite2 file; no donor holder is named in the header |
| 12 | D | `release/write_mfs_in_kernel.c` | 1 / 1 | -- | beerware 1 | Poul-Henning Kamp's "beer-ware" notice | **ok**: beer-ware notice is in the file; no "Copyright" word, found by the beerware test |
| 13 | D | `usr.sbin/crunch` (`crunchgen`, `crunchide`) | 9 / 14 | U Maryland 7, Demetriou 4, Bruce Evans 1 | permission-notice 6, BSD-4clause 2 | U. Maryland permission notice; Demetriou's (crunch | **PARTIAL**: cell names U. Maryland and Demetriou; Demetriou's files are four-clause with advertising (2 files), not stated; 1 file also names Bruce Evans; 5 of 14 files have no notice |
| 14 | D | `distrib/i386/floppies` | 6 / 18 | Demetriou 5, Regents 1 | BSD-4clause 6 | Demetriou's four-clause, advertising clause | **PARTIAL**: Demetriou four-clause stated; 1 file is the Regents'; 12 of 18 files carry no notice |
| 15 | D | `i386/isa/wd.c` | 1 / 1 | Regents 1, Jolitz 1 | BSD-4clause 1 | Regents (via Jolitz's contribution) | **ok** |
| 16 | D | `i386/isa/wdreg.h` | 1 / 1 | Regents 1, Jolitz 1 | BSD-4clause 1 | Regents | **ok** |
| 17 | D | `i386/isa/atapi.c` | 1 / 1 | Cronyx/Vakulenko 1 | permission-notice 1 | Cronyx (Serge Vakulenko) permission notice | **ok** |
| 18 | D | `i386/isa/wcd.c` | 1 / 1 | Cronyx/Vakulenko 1 | permission-notice 1 | Cronyx (Serge Vakulenko) permission notice | **ok** |
| 19 | D | the slice layer: `kern/subr_diskslice.c`, `ker | 3 / 3 | Regents 3, Bruce Evans 3, Jolitz 2 | BSD-4clause 3 | Bruce Evans, 1994 | **PARTIAL**: Evans 1994 stated; all three also carry the Regents four-clause (advertising) block, not stated |
| 20 | D | `<sys/diskslice.h>` | 1 / 1 | Bruce Evans 1 | BSD-2clause 1 | -- | **HOLDER**: cell "--"; header is Bruce D. Evans 1994, two-clause |
| 21 | D | `usr.sbin/config`: `isa_biotab()` | 1 / 1 | Regents 1 | BSD-4clause 1 | per-drive table in Lite2's `isa_device` layout | **GAP**: cell does not name a holder; file is Lite2's mkioconf.c, Regents four-clause. No donor holder is named in the header |
| 22 | D | `i386/isa/if_ed.c`, `if_edreg.h` | 2 / 2 | Greenman 2 | -- | David Greenman's own notice | **ok**: Greenman 1993 "may be used, modified" text; matches cell |
| 23 | D | `lib/libcompat/4.1`: `gtty`, `stty`, `ftime` | 8 / 8 | Regents 5, Demetriou 3 | BSD-4clause 8 | -- | **GAP**: cell "--"; 5 files Regents, 3 Demetriou 1994, all four-clause with advertising |
| 24 | D | `usr.bin/m4/m4.1` | 0 / 1 | -- | -- | -- | **NOHDR**: no copyright or licence text in m4.1 ($Id says glass); cell "--" |
| 25 | D | `kern/sys_process.c` | 1 / 1 | Fagan 1 | BSD-4clause 1 | Sean Eric Fagan 1994, four-clause, advertising cla | **ok** |
| 26 | D | `i386/i386/procfs_machdep.c` | 1 / 1 | Regents 1, Pendry 1 | BSD-4clause 1 | Regents and Pendry's | **ok** |
| 27 | D | `i386/i386/machdep.c`: `ptrace_set_pc`, `ptrac | 1 / 1 | Regents 1, Jolitz 1 | BSD-4clause 1 | -- | **GAP**: cell "--"; Jolitz/Regents four-clause file; no donor notice in the header |
| 28 | D | `i386/include/ptrace.h`: `PT_GETREGS` family | 1 / 1 | Regents 1 | BSD-4clause 1 | -- | **GAP**: cell "--"; Regents four-clause; no donor notice in the header |
| 29 | D | `i386/include/reg.h`: `struct reg` (NetBSD), ` | 1 / 1 | Regents 1, Pendry 1, Jolitz 1 | BSD-4clause 1 | -- | **GAP**: cell "--"; Regents four-clause with Pendry/Jolitz attribution; no NetBSD or FreeBSD notice in the header |
| 30 | D | `lib/libc/sys/ptrace.2` | 1 / 1 | -- | public-domain 1 | -- | **NOHDR**: page says "in the public domain"; no copyright line; cell "--" |
| 31 | D | `kern/genassym.sh`, `sys/assym.h` | 1 / 2 | Moolenaar 1 | BSD-3clause 1 | -- | **HOLDER**: genassym.sh names Marcel Moolenaar 1999 (three-clause) which appears in no ledger, imports.md or COPYRIGHT text; assym.h has no notice |
| 32 | D | `contrib/diff-2.6` (GNU diff 2.6) | 22 / 32 | FSF 22, Allman 1 | GPL 22, Caldera 1, LGPL 1 | GPL | **ok**: FSF GPL/LGPL in 23 of 32 files; 9 files (Makefile, wrapper) have no notice |
| 33 | D | `contrib/grep-2.0` (GNU grep 2.0) | 15 / 26 | FSF 14 | GPL 15, LGPL 1 | GPL | **ok**: FSF GPL in 15 of 26; 11 without notice |
| 34 | D | `contrib/bc-1.03` (GNU bc 1.03 + dc) | 34 / 71 | FSF 33, Allman 1, Phil Nelson 1 | GPL 31, LGPL 2 | GPL v2 | **ok**: FSF GPL in 33 of 71; Phil Nelson in 1; 37 without a notice |
| 35 | D | `contrib/uucp-1.06.1` (Taylor UUCP) | 202 / 318 | Ian Lance Taylor 189, FSF 187, Regents 1 | GPL 188, LGPL 103, public-domain 3, beerware 1 | GPL | **ok**: Taylor GPL; 3 files public-domain, 1 beer-ware (not in cell); 120 files have no notice |
| 36 | D | `sys/ddb`, i386 `db_*.c`, `db_machdep.h` (28 f | 29 / 29 | CMU 28, NetBSD Foundation 1, Bruce Evans 1 | CMU 28, BSD-4clause 1 | Carnegie Mellon | **ok**: CMU in 28 of 29; the 29th is NetBSD Foundation 1997 (db_elf.c, row 38) |
| 37 | D | `share/man/man4/ddb.4` | 1 / 1 | CMU 1 | CMU 1 | CMU (via NetBSD) | **ok** |
| 38 | D | `sys/ddb`: ELF symbol reading | 1 / 1 | NetBSD Foundation 1 | BSD-4clause 1 | NetBSD/Demetriou lineage (same family as `nlist_el | **MISMATCH**: header is The NetBSD Foundation 1997, four-clause (advertising); cell says "NetBSD/Demetriou lineage"; no Demetriou notice in the file |
| 39 | D | `contrib/gdb-4.16` (1820 files) | 1225 / 1809 | FSF 1079, Cagney 80, AMD 77, Ian Lance Taylor 40, Hewlett-Packard/Utah 22, Pace Willisson 8 | GPL 1040, LGPL 47, public-domain 34, BSD-2clause 3, BSD-4clause 2 | GPL (GNU) + Berkeley/FreeBSD on the glue | **PARTIAL**: GNU tree: counts below; non-FSF holders (AMD, Cagney, HP, Sun, Regents...) not in cell |
| 40 | D | `contrib/gcc-2.95.3` | 1741 / 2790 | FSF 1590, Hewlett-Packard/Utah 63, Sun 41, Allman 18, Regents 13, Ian Lance Taylor 11 | GPL 1526, LGPL 104, permission-notice 83, public-domain 30, BSD-4clause 5 | GPL v2 (+ LGPL/runtime exception on libraries, fil | **PARTIAL**: GNU tree: counts below; 83 permission-notice files, 5 four-clause files, other holders not in cell |
| 41 | D | `contrib/binutils-2.11.2` | 1149 / 3011 | FSF 1048, Ian Lance Taylor 119, Regents 22, Hewlett-Packard/Utah 20, Allman 3, Conklin 3 | GPL 967, LGPL 31, public-domain 26, BSD-2clause 12, BSD-3clause 4 | GPL v2 | **PARTIAL**: GNU tree: counts below; Regents (22 files) and others not in cell |
| 42 | E | `lib/libcompat/4.3/regex.c` (93 lines) | 1 / 1 | Regents 1, U Maryland 1 | BSD-4clause 1 | Regents | **ok**: Regents notice with U. Maryland attribution in the text |
| 43 | E | `bin/expr` (679 lines) | 2 / 3 | Winning Strategies 1 | BSD-4clause 1, public-domain 1 | public domain (code) / four-clause w/ advertising  | **ok**: expr.y public domain (Willisson, Conklin); man page Winning Strategies four-clause: matches cell |
| 44 | E | `sys/kern/subr_rmap.c` | 1 / 1 | Solfrank/TooLs 1 | BSD-4clause 1 | Wolfgang Solfrank / TooLs GmbH, 1992/1994, four-cl | **PARTIAL**: TooLs/Solfrank four-clause matches cell; the Regents/USL notice A3 names is NOT in the header (checked below) |
| 45 | E | `kern/kern_acct.c` | 1 / 1 | Regents 1, Demetriou 1, Jolitz 1, UNIX System Laboratories 1 | BSD-4clause 1, USL-notice 1 | Demetriou 1994 and the Regents', three-clause | **MISMATCH**: cell says three-clause; the header is the Lite2 stub's Regents/USL four-clause block (advertising clause) with Demetriou's line added above it. Donor's own text unchecked |
| 46 | E | `kern/kern_physio.c` | 1 / 1 | Regents 1, Demetriou 1, Jolitz 1, UNIX System Laboratories 1 | BSD-4clause 1, USL-notice 1 | Demetriou 1994 and the Regents', three-clause | **MISMATCH**: same as 45 |
| 47 | E | `kern/vfs_bio.c` | 1 / 1 | Regents 1, Demetriou 1, UNIX System Laboratories 1 | BSD-4clause 1, USL-notice 1 | Demetriou/Solfrank-era NetBSD notice | **MISMATCH**: same as 45; the cell names no holder |
| 48 | E | `kern/tty_subr.c` | 1 / 1 | Regents 1, Greenman 1, UNIX System Laboratories 1 | BSD-4clause 1, USL-notice 1 | Greenman's own ("may be used ... sold") | **PARTIAL**: Regents/USL four-clause block plus Greenman's 1994 text lower in the file; cell names only Greenman |
| 49 | E | `sys/kern/kern_exec.c`, `exec_subr.c`, `exec_c | 6 / 6 | Demetriou 6, Hannum 2, Solfrank/TooLs 1, Regents 1, UNIX System Laboratories 1 | BSD-4clause 6, USL-notice 1 | Demetriou's four-clause, advertising clause; Solfr | **ok**: Demetriou four-clause, Solfrank, de Raadt and Regents lines present; matches cell |
| 50 | E | `usr.bin/awk` | 13 / 15 | Lucent 13, Kernighan/Lucent 1 | permission-notice 13 | Lucent 1997 | **ok**: Lucent 1997 permission notice, no advertising clause; 2 of 15 files without notice |
| 51 | E | `usr.sbin/cron`, `usr.bin/crontab` | 19 / 21 | Vixie 18, Regents 1 | -- | Vixie's notice | **ok**: Vixie in 18 of 21; one Regents file; 2 without notice |
| 52 | E | `usr.bin/at`, `libexec/atrun` | 12 / 14 | Koenig 9, Demetriou 4, Parsons 2 | BSD-3clause 8, BSD-4clause 4 | Koenig/Parsons, via NetBSD | **GAP**: cell names Koenig/Parsons; 4 files carry Demetriou 1993 four-clause (advertising), not in cell |
| 53 | E | `usr.sbin/sa`, `usr.sbin/ac` | 8 / 10 | Demetriou 8, Gerraty 2, Regents 1 | BSD-4clause 7, permission-notice 1 | Demetriou/Gerraty | **PARTIAL**: Demetriou and Gerraty stated; 7 files are four-clause with advertising, not stated |
| 54 | E | `usr.bin/units` | 2 / 6 | Demetriou 1, Mariano 1 | BSD-4clause 1, BSD-3clause 1 | Mariano's | **HOLDER**: cell names Mariano; pathnames.h is Demetriou 1993, four-clause (advertising), not in cell |
| 55 | E | `usr.sbin/quot` | 2 / 3 | Solfrank/TooLs 2 | BSD-4clause 2 | Solfrank/TooLs GmbH, four-clause, advertising clau | **ok** |
| 56 | E | `sbin/fsdb` | 4 / 5 | Kohl 4 | BSD-3clause 4 | Kohl's three-clause | **ok** |
| 57 | F | `usr.bin/compress` | 6 / 9 | Regents 5, Hewlett-Packard/Utah 1 | BSD-4clause 5 | Regents | **ok**: Regents four-clause (advertising); Hannum named in a comment only, not a holder line |
| 58 | F | `usr.bin/deroff` | 2 / 3 | Regents 2, Caldera 2 | BSD-4clause 2, Caldera 2 | Regents + Caldera (2002) | **PARTIAL**: Regents + Caldera named; Caldera's 2001-2002 licence is itself four-clause with an advertising clause ("developed or owned by Caldera"), not stated |
| 59 | F | `games/ching` | 8 / 13 | Regents 7, Caldera 4 | BSD-4clause 8, Caldera 4 | Regents + Caldera | **PARTIAL**: same as 58 |
| 60 | F | `usr.sbin/sendmail` | 182 / 224 | Regents 173, Allman 166, CMU 2, Vixie 1, Hewlett-Packard/Utah 1 | BSD-4clause 175, CMU 2, public-domain 1, BSD-2clause 1 | Regents | **HOLDER**: cell "Regents"; 166 of 224 files also name Eric P. Allman (1983, 1995-1997); 175 files carry the advertising clause |

## Method

All scripts run from the worktree root, read tracked files only, and
read no donor tree. `/home/claude/donors/csrg` was not opened.

- **Rows.** The three kind tables in `clearance.md` were parsed: 60
  rows (41 D, 15 E, 4 F), which matches the ledger's own recount. Each
  row's "File" cell was mapped by hand to paths under `usr/src`
  (directories walked, symlinks and object/archive suffixes skipped).
  Row 1 is `lib/csu/common_elf` and `i386_elf` only, not Lite2's own
  `csu` dirs. Row 21 is `usr.sbin/config/mkioconf.c`, and 27 and 28 are
  `machdep.c` and `ptrace.h`, whole files, as the donor lines are a few
  statements of them.
- **Header.** First 100 lines of each text file (a file with a NUL in
  its first 4,000 bytes is skipped). Holders by regular expression on a
  fixed list of names. Licence class by clause phrases ("advertising
  materials mentioning", "endorse or promote", GNU "General Public
  License", "Carnegie Mellon" with its permission wording, "beer",
  "public domain", "permission to use", "UNIX System Laboratories").
- **Ledger side.** The "Licence/notice" cell, read by hand against the
  header class. The advertising-clause test: the cell contains
  "advertis" or "four-clause".
- **A3 named items.** A search of the first 140 lines of each file for a
  holder name.
- **A4.** The script below. It lists tracked files with `git ls-files`
  (saved to a file first), reads each, and requires a line-anchored
  `SPDX-FileCopyrightText` naming Nicholas Martin in the first 10 lines
  and no `SPDX-License-Identifier` line anywhere.

**The check can fail.** Run with three scratch files added to the set
(not in the tree):

    lacks.sh        no SPDX line
    has-licence.sh  SPDX copyright line plus SPDX-License-Identifier: MIT
    good.sh         SPDX copyright line only

the own-set totals moved from `ok 55, missing 75` to `ok 57, missing
76`; `lacks.sh` was listed under "no SPDX-FileCopyrightText" and
`has-licence.sh` under "licence identifier present", each with its path.
Against the tree itself it fails today on 75 files, which is the
finding above, not a defect in the check.

    #!/usr/bin/env python3
    import os, re, sys, collections
    tracked = [p for p in open(FILELIST, 'rb').read().decode('utf-8', 'surrogateescape').split('\0') if p]
    extra = sys.argv[sys.argv.index('--extra') + 1:] if '--extra' in sys.argv else []
    FT = re.compile(r'^[ \t]*(?:#|//|/\*|\*|;|--|\.\\")?[ \t]*SPDX-FileCopyrightText:\s*(.*)$', re.M)
    LI = re.compile(r'^[ \t]*(?:#|//|/\*|\*|;|--|\.\\")?[ \t]*SPDX-License-Identifier:\s*(.*)$', re.M)
    NM = re.compile(r'Nicholas Martin')
    INHERITED = {'.cshrc', '.profile'}
    def read(p):
        if os.path.islink(p) or not os.path.isfile(p): return None
        try: return open(p, 'rb').read().decode('latin-1')
        except OSError: return None
    txt = {p: read(p) for p in tracked}; txt = {p: t for p, t in txt.items() if t is not None}
    D = {p for p, t in txt.items() if FT.search(t)}
    E = {p for p, t in txt.items() if NM.search(t)}
    own = {p for p in tracked if p not in INHERITED and
           (p.startswith(('build/', 'docs/')) or ('/' not in p and p != 'COPYRIGHT'))}
    own |= D | E | set(extra)
    for p in sorted(own):
        t = txt.get(p) or read(p)
        m = FT.search('\n'.join(t.splitlines()[:10]))
        if LI.search(t): print('licence identifier present', p)
        if not (m and 'Nicholas Martin' in m.group(1)): print('no SPDX copyright line', p)

## What I could not check

- **Header against the donor's original: 0 of 60.** The donors under
  `/home/claude/donors` are git repositories holding the releases
  (`nb10`, `nb12h`, `fb20h`, `nbh` and others) with an empty working
  directory, or a checkout of a current branch. This session refused
  every git command aimed at a directory outside its worktree, so no old
  release file could be read. The tag layouts were not looked at either.
  Whoever integrates this needs a session that can run, for each row,
  the donor's file at the release the ledger names and compare the
  first ~60 lines to the tree's header.
- **Whether a missing holder is required.** I measured that the Hannum,
  NetBSD Foundation and Regents/USL lines are absent. Whether the
  donor's file had them, and whether the lines taken are enough to
  need them, needs the donor.
- **Holders not on my list.** 45 names are matched by pattern. A holder
  outside the list is invisible to the "holders" column; in the large
  GNU trees (rows 39-41) the table shows the common ones and the rest
  is noise to a human reader.
- **Row 39-41 (gdb, gcc, binutils; 7,600 files).** Only counted, not
  read. 619, 1,085 and 1,894 files in those trees have no copyright
  line in the first 120 lines (Makefiles, data, docs). The verdict
  there is "PARTIAL" because the cell says GPL and the files also carry
  other holders and four-clause or permission notices; I did not check
  whether any of those files is this project's donor change and not
  GNU's.
- **Rows 4, 24, 30** have no header text, so a missing holder cannot be
  seen from the file.
- **The AI-ONLY NOTE count (376)** is files containing the string. I
  did not read them to see which are our writing and which a note on a
  donor file.
- **`.want` fixtures and `.md` files**: whether they ought to carry the
  line is policy (see recommended edit 5), not something I could
  measure.
- **Legal effect.** Nothing here says whether a header is sufficient
  under any licence. These are facts about the text of the files.

## Done in patch 267 (2026-11-01), and what was left

Made: the five wrong ledger cells corrected in `clearance.md` (rows for
`exec_elf.h`, `db_elf.c`, `kern_acct.c`, `kern_physio.c`, `vfs_bio.c`),
each checked against the file's header by the integrator; "three-clause"
in the AI-ONLY NOTE of the last three corrected to the Regents' four-clause
block (comment only; objects compared); the SPDX copyright line (no
licence identifier) added to `build/mklabel.c`, `mkfs-host.c`,
`addfile-host.c`, `fs-host.h`, `build/shim/shim.ld` and `devices.spec`.

**Not made, for the maintainer:** extending `COPYRIGHT` to name the other
holders and the advertising-clause files (about 8,900 files name a
non-Regents holder; the Regents' own clause 3 is already there); the 24
partial or missing-holder ledger cells; the named A3 files' holders
(Hannum in `locore.s`, `cpu.h`, `npx.c`; The NetBSD Foundation in
`boot2.c`; `nlist.c`; `cdio.h`); and whether the 48 writing files and 20
test fixtures should carry the SPDX line. A3 is therefore **not done**.
No header was compared with a donor's original (0 of 60): see the limit
at the top.

## Patch 268 (2026-11-01): the write-ups

On the maintainer's instruction ("Yes"), the SPDX copyright line (no
licence identifier) was added to the 45 files under `docs/` that lacked
it: 43 `.md` files (as an HTML comment on the first line) and 2 `.tsv`
files (as a `#` line, the files already begin with `#` header lines). Only
lines were added. The 20 `.want` fixtures are left without it: the kernel
test compares them with its output exactly. Check: every tracked file
under `docs/` except `docs/reference/test_mk7.3` now has the line.

## A3 finished (2026-11-01, patch 275)

Five read-only workers did the measurement in parallel, each writing a
report outside the tree; the integrator checked their key evidence
against the donors and made every edit.

**Every ledger row's header against the donor's original: 63 of 63.**
In 61 rows the tree's notices are the donor's, word for word where the
files are the donor's; no donor holder or licence was found dropped.
The two exceptions are judged and recorded rather than restored:

- `pmap.c` (row 15): FreeBSD 2.0's file carries John S. Dyson's and
  David Greenman's 1994 lines beside the Regents'; what this tree took
  from it is the `DMAPAGES` name and two expressions using it, judged
  too small to carry their notice.
- `machdep.c`'s `ptrace_set_pc`/`ptrace_write_u` (row 30): FreeBSD's
  file carries Terrence R. Lambert's 1992 line, but `ptrace_write_u` was
  added by Bruce Evans on 1995-01-14 (FreeBSD's security fix,
  `20415301cd`), after Lambert's work, so Lambert's line is not carried
  for it (checked by the integrator in FreeBSD's history).

Every row's "Licence/notice" cell now names every holder, the clause
count and whether there is an advertising clause, so the 20 rows whose
advertising clause was recorded nowhere are recorded there. Caldera's
2001-2002 licence on `deroff` and `ching` is in the ledger and in
`COPYRIGHT`. Other corrections the check found: rows 37-38 were not
"byte for byte" NetBSD's (bc's `proto.h` and nine uucp files are
patched, no notice touched); row 42's glue is FSF/GPL, not
"Berkeley/FreeBSD"; row 44 includes gprof's Regents notices; and the
"McCanne" credit for `microtime.s` (row 27) is withdrawn, as the donor
names no author. Rows 48-50: Demetriou's line is inside the Regents'
four-clause block in the donors too.

**A3's named files:**

| file | done |
|---|---|
| `locore.s` | Charles Hannum's 1993-1994 line added (NetBSD 1.0's copyout, CPU detection, esym) |
| `cpu.h` | not needed: no Hannum text; the three `CPUCLASS_` constants are Demetriou's `cputypes.h`, named in the note |
| `npx.c` | not needed: the code is FreeBSD 1.0's, under the Jolitz and Regents notices it carries; its notes and `imports.md` corrected (they credited NetBSD 1.0) |
| `boot2.c` | The NetBSD Foundation's 1997 notice added in a second block (NetBSD 1.3's `loadfile.c` symbol loading), verbatim |
| `subr_rmap.c` | not needed: NetBSD 1.2's file byte for byte, no Lite2 text left |
| `diff3.sh` | records corrected: it was Berkeley's own, not OpenBSD's, and Caldera's licence was put on it without a source |
| `wd.c`, `wdreg.h` | not needed: FreeBSD 2.1.0's headers unchanged |
| `cdio.h` | no notice in any donor; recorded as untraced |
| both `nlist.c` | Christopher G. Demetriou's 1996 line added (NetBSD 1.3's dispatcher and table) |
| run-4's small items | none needs a holder: the donor holders are already present, or the lines are short, only-way hooks; two judgement calls (machdep.c's CPU-name table, clock.c's `delay`) are left without Hannum's line and recorded here |

**`COPYRIGHT`**: Lite2's text unchanged; a section after it names the
other holders and licences, points to the ledger, and states that this
project's own files carry no licence yet and are not under the Berkeley
terms.

Not checked: the full text of sendmail's third-party contrib notices
(matched line by line, not transcribed); Lite2's own `compress`; whether
any header is legally sufficient. The numbers in `roadmap-audit.md`'s
A3 list are run-4 report indexes, not `bestfit-sweep.tsv` rows.

## Hannum's line in machdep.c and clock.c (2026-11-01, patch 276)

The two judgement calls above were decided by the maintainer the
conservative way: Charles Hannum's "Copyright (c) 1993, 1994 Charles
Hannum." line, as NetBSD 1.0's own machdep.c and clock.c have it above
the Regents', is added to i386/i386/machdep.c (for the CPU-name table,
`struct cpu_nameclass` and `i386_cpus[]`, from NetBSD 1.0's
identifycpu, a64a1d48) and to i386/isa/clock.c (for `delay`'s loop and
`gettick`, after NetBSD 1.0's clock.c, 300439b8). The terms are the
Regents' four clauses, the files' own. Lambert's 1992 line in NetBSD's
machdep.c is not added: what was taken is NetBSD's own table, not
Lambert's text (FreeBSD's version of his file has a different table).

