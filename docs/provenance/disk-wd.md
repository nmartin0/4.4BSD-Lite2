# The IDE driver's lineage, measured

Measured 2026-10-30 at the maintainer's request, to settle plan-4.5.md
stage 3, item 4: is FreeBSD 2.1.0's `wd.c`, taken in patches 318-319,
usable under the 386BSD rule (`roadmap-audit.md`, rule 2)? Facts, not
legal advice.

## Why it needed measuring

FreeBSD 2.1.0's `wd.c` still carries the stamp `from: @(#)wd.c 7.2
(Berkeley) 5/9/91`, the Net/2-era version, where Lite2's file is 8.1. A
stamp says where a file came from, not how much of the old text is
left. The question that matters is whether any text survives that
Berkeley changed or dropped when it made Lite2, because the settlement
was fought over such text.

## Method

`build/lineage.py`. It takes the distinct lines of each file with at
least twelve letters or digits, whitespace folded, and sorts a
candidate's lines by where they first appear:

1. **Lite2's `wd.c`** (the cleared text);
2. **386BSD 0.0's `wd.c`** and not Lite2's: the Net/2-era lines that
   Berkeley changed or dropped (31 lines);
3. **386BSD 0.1's `wd.c`** and neither of those: what Jolitz added in
   0.1 (313 lines);
4. none of these: the candidate's own.

386BSD 0.0 and 0.1 are `386BSD-0.0` and `386BSD-0.1` in the Unix
history repository (`usr/src/sys.386bsd/i386/isa/wd.c`). Nothing in the
CSRG history was read.

## Results

| `wd.c` | lines | in Lite2 | early, not Lite2 | first in 0.1 | its own |
|---|---|---|---|---|---|
| 386BSD 0.0 | 752 | 718 | 34 | - | - |
| FreeBSD 1.0.0 | 644 | 289 (44%) | 17 | 276 (42%) | 62 |
| FreeBSD 2.0 | 1020 | 169 (16%) | 2 | 119 (11%) | 728 |
| **FreeBSD 2.1.0 (ours)** | **1087** | **130 (11%)** | **2** | **80 (7%)** | **875** |
| NetBSD 1.0 | 745 | 94 (12%) | 2 | 16 (2%) | 633 |

What it shows:

- **Lite2's own `wd.c` is 386BSD 0.0's.** 718 of 752 lines match. The
  Net/2-era file is what Berkeley shipped as 8.1, so that text is
  cleared by being in Lite2.
- **The 31 lines Berkeley changed between the two are mostly
  cosmetic:** `#include "param.h"` became `#include <sys/param.h>`,
  and the copyright notice was rewritten.
- **FreeBSD 1.0.0 still carried 17 of them.** FreeBSD 2.0 and 2.1.0
  carry **two, and both are the copyright notice** (`Copyright (c) 1990
  The Regents`, `All rights reserved`). The rest were replaced between
  1.0.0 and 2.0.
- **7% of 2.1.0 is 386BSD 0.1's.** These are 80 distinct lines, mostly
  the drive state machine and its comments (`DKFL_SINGLE`,
  `wdustart`, the `wdopen` signature). They are not Net/2 text,
  because they are not in 0.0's file. They are 386BSD 0.1's own,
  carried by FreeBSD.
- **NetBSD 1.0's `wd.c`** carries less of 0.1 (2%), but has no ATAPI
  and no CD driver.

## The other files of patches 318-319

The same test, against Lite2's `wdreg.h`, `ufs_disksubr.c` and
`dkbad.c` and the 386BSD 0.0 copies:

| File | lines | early, not Lite2 | notes |
|---|---|---|---|
| `wdreg.h` | 112 | 2 (copyright) | 57 in Lite2's |
| `subr_diskslice.c` | 390 | 2 (copyright) | Bruce Evans, 1994 |
| `subr_dkbad.c` | 92 | 2 (copyright) | Bruce Evans, 1994 |
| `diskslice_machdep.c` | 243 | 2 (copyright) | Bruce Evans, 1994 |
| `atapi.c` | 408 | 0 | Cronyx, 1995; 0 lines in 386BSD 0.0 |
| `wcd.c` | 523 | 0 | Cronyx, 1995; 3 lines in 386BSD 0.0 |

## Verdict

No Net/2-era text that Berkeley changed or dropped survives in the
disk driver files other than the copyright notices. That includes
`wd.c` itself. There is one residue, the 80 lines from 386BSD 0.1 in
`wd.c`. These are second in the source order, under a Regents'
header, not Net/2 text, but carried forward by FreeBSD, which clause
(c) of the rule reads against "unchanged". The file is 80% changed.
Recorded as: **clear, with the residue named.** The ledger row says so.

## Limits of the measurement

- **A lower bound.** A renamed or reflowed line counts as new, so
  copying with edits is under-counted.
- **386BSD 0.0's file is taken to be Net/2's.** Its stamp is Net/2's
  (7.2, 5/9/91), but that is UNVERIFIED against Net/2 itself, which
  this work does not read.
- **The settlement's lists were not consulted.** The test finds whether
  anything survives that Berkeley changed, not whether the change was
  required by the settlement.
- **The copyright of 386BSD 0.1's additions.** Their header is the
  Regents' 1990 notice; whether Jolitz held a separate notice for 0.1
  is not established here.

## What this settles for 4.5BSD and what it leaves

- **FreeBSD 2.1.0's driver stays in 4.5BSD.** Replacing it would be
  work for nothing: 4.6BSD replaces it with OpenBSD's `wdc`/`wd`/`atapiscsi` on
  Torek's autoconfiguration (`plan-4.6.md`, stage 4).
- The residue is named in the ledger row, not rewritten.
- **The holes in this driver's lineage are still fixed:** `wcd`'s
  bounds checks (FreeBSD 2.1.7), `if_ed` (a separate file).

The same test is run for each ledger row, starting with the floppy's
DMA code (`isa.c`, #373).
