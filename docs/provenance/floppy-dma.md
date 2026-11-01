# The floppy's DMA code, measured

Measured 2026-11-01 for plan-4.5.md stage 3, item 5 (#373), the same
way `disk-wd.md` measured `wd.c`. Facts, not legal advice. Everything
in this file is AI-ONLY NOTE: it was written by the assistant that ran
the measurement, from the commands named below.

## What the floppy's DMA code is

Four places in the tree, since patch d72ca64a:

| Where | What |
|---|---|
| `sys/i386/isa/isa.c` | `isaphysmem`, `isa_dmastart`, `isa_dmadone`, `isa_dmarangecheck`, the `DMA1_*`/`DMA2_*` register macros (replacing Lite2's `at_dma`) |
| `sys/i386/isa/ic/i8237.h` | the five `DMA37MD_*` mode bits (12 lines) |
| `sys/i386/i386/pmap.c` | `DMAPAGES`, `avail_start = firstaddr + DMAPAGES*NBPG`, and the block that maps them at `isaphysmem` |
| `sys/i386/isa/fd.c` | the calls to `isa_dmastart`/`isa_dmadone`, and the Sense Interrupt changes |

## The reference texts

All read with `git show`, read-only. Nothing from the CSRG history
was read.

| Label | What | Source |
|---|---|---|
| Lite2 | `isa.c` 8.1, 254 lines; `fd.c` 8.1, 671 | this repo, the import commit b3d2855f |
| 386BSD 0.0 | `isa.c` 7.2, 253 lines; `fd.c` 7.4, 641 | /home/claude/386bsd, tag r00, `arch/i386/isa/` |
| 386BSD 0.1 | `isa.c` 7.2, 533 lines; `fd.c`, 673 | same, tag r01, `usr/src/sys.386bsd/i386/isa/` |
| FreeBSD 2.0 | `isa.c` rev 1.35 (ache, 1994/11/01), 1030 lines; `fd.c`; `ic/i8237.h` rev 1.2 | /home/claude/donors/fbh, tag `release/2.0` |
| FreeBSD 2.1.0 | same files, `isa.c` 1056 lines | same, tag `release/2.1.0` |
| NetBSD 1.0 | `isa/dma.c` (271 lines), `fd.c` | /home/claude/donors/netbsd, `origin/netbsd-1-0`, `sys/arch/i386/isa/` |
| OpenBSD 1995 | `sys/dev/isa/isadma.c` (273 lines), first import 1995-10-18 (df930be708d); `fd.c` | /home/claude/openbsd/src |

`build/lineage.py` as in `disk-wd.md`: distinct lines of twelve or more
letters or digits, whitespace folded. The candidate is the DMA region
of our `isa.c` (the register macros through `isa_dmarangecheck`, not
the AI-ONLY NOTE above them): **95 distinct lines**. Lite2's `isa.c`
and 386BSD 0.0's `isa.c` were taken whole.

## Result 1: Lite2's own DMA routine is a different text

Lite2's `at_dma` (32 distinct lines, with `kernel_space` and
`dma_bounce`) and our region share **3 lines**: `/* copy bounce buffer
on write */`, `/* translate to physical */` and
`if (dma_bounce[chan] == 0)`. Those are generic.

386BSD 0.0's `at_dma` is Lite2's: 32 of 32 lines match, and whole-file
`isa.c` 0.0 has **0** lines that Lite2 changed or dropped and that our
region carries. So **no Net/2-era text that Berkeley changed or
dropped survives in the DMA routines.** `at_dma` itself is not in the
tree any more.

Why the tree does not use Lite2's candidate, from reading `at_dma`
(not by running it): it writes channel 2's controller ports (0xC, 0xB,
0x4, 0x5, 0x81) whatever channel it is given, using `chan` only for the
mask; it bounces only a user address (`kernel_space`), so a kernel
buffer above 16 MB goes to the controller as it is; and its bounce
buffer is `geteblk`'s, which may lie anywhere. The in-tree note says
that with 32 MB a floppy read came back as zeros. I did not boot
anything and did not reproduce that.

## Result 2: where the 95 lines come from

Sorted by first appearance, in the order Lite2, 386BSD 0.0, 0.1,
FreeBSD 2.0:

| Region of our `isa.c` | lines | Lite2 | in 0.0, not Lite2 | first in 386BSD 0.1 | first in FreeBSD 2.0 | ours |
|---|---|---|---|---|---|---|
| register macros through `isa_dmarangecheck` | 95 | 3 (3%) | 0 | 48 (51%) | 42 (44%) | 2 (2%) |

- **The 48 of 0.1** are the `isa_dmastart` family's skeleton:
  `isaphysmem`, `dma_bounce[]`, `bounced[]`, `dmapageport[]`, the
  bounce copy in and out, the register writes, the `panic` strings,
  and the comments (`/* region of physical memory known to be
  contiguous */`). 386BSD 0.1's `isa.c` is still under the Regents'
  1991 notice (`7.2 (Berkeley) 5/13/91`). 386BSD 0.0 has none of it.
- **The 42 of FreeBSD 2.0** are what FreeBSD's header credits to
  "92/08/18 Frank P. MacLachlan (fpm@crash.cts.com)": the channel
  argument to `isa_dmarangecheck`, the `dma_pgmsk` 64 KB/128 KB
  page-crossing test, the `priorpage = 0` initialisation, the
  `DMA1_*`/`DMA2_*` register macros, the B_RAW autoinitialise cases,
  and the per-channel range panic. None of them is in 386BSD 0.1.
  The macros and a few lines (the `phys >= ISARAM_END`) may be
  FreeBSD's later work; the file does not say which.
- **The 2 of ours** differ only in `kernel_pmap` for `pmap_kernel()`.
- 386BSD 0.1 has the same functions without those fixes: `priorpage`
  is uninitialised, `isa_dmarangecheck` takes no channel and has no
  64 KB page-crossing test, and the word-channel count is shifted the
  wrong way. For the floppy (channel 2, 512-byte blocks) the first
  two matter; the third does not.

NetBSD 1.0's `dma.c` and OpenBSD's 1995 `isadma.c` share **60** and
**58** of the 95 lines, and **1** and **0** that are in none of Lite2,
0.0, 0.1, FreeBSD 2.0. They are the same lineage, rewritten in K&R with
renamed headers; they are not "line for line" as the AI-ONLY NOTE in
`isa.c` says. That note should say 60 and 58 of 95. Neither is an
earlier source than FreeBSD 2.0 for these lines.

## Result 3: the other three places

- **`ic/i8237.h`** is byte-identical to FreeBSD 2.0's (`$Id`
  1.2, 1993, Rodney Grimes). The five defines are the 8237's mode bits.
  The file has no copyright notice, FreeBSD's or ours.
- **`pmap.c`**: the comment `reserve special hunk of memory for use by
  bus dma ... -wfj` and `extern vm_offset_t isaphysmem;` are in 386BSD
  0.1 (Jolitz's initials) and in FreeBSD 2.0, and in neither Lite2 nor
  386BSD 0.0. `DMAPAGES` and `avail_start = firstaddr + DMAPAGES*NBPG`
  are FreeBSD 2.0's. The `pmap_map(...)` line is the tree's own wording.
- **`fd.c`**: of 292 distinct lines, 261 (89%) are Lite2's, **0** are
  0.0's and not Lite2's, 0 are first in 0.1, and 31 are the tree's
  own. Read by eye, they are the tree's AI-ONLY NOTE comment lines, the
  new `fd_sensei()` and `isa_dma*` calls and the status test; one
  (`dp->b_actf = bp->b_actf;`) is also in FreeBSD 2.0's `fd.c`. `fd.c`
  is Berkeley's, corrected, as `imports.md` says. FreeBSD 2.0's `fd.c`
  (732 distinct lines) is a different, much larger file, of which 59
  lines are Lite2's.

## Verdict

**No Net/2-era text that Berkeley changed or dropped was found, residue
named** (the reviewer held that "clear" overstated this, since the test
sees only lines 386BSD 0.0 has and Lite2 lacks; Net/2 itself was never
read, a reworded line counts as new, and the 48 lines first in 386BSD
0.1 are a 1992 addition (its rcsid is "1.2 92/01/21") whose author is not
shown, whatever the 1991 notice above them says). On the basis `disk-wd.md`
used: the test finds **no Net/2-era text that Berkeley changed or dropped** in
the floppy's DMA code (0 lines from 386BSD 0.0 that Lite2 lacks; the
Lite2 candidate `at_dma` shares 3 generic comment lines and is not in
the tree). The code that is there is:

1. 386BSD 0.1's, **48 of 95 lines (51%)**, under the Regents' notice,
   carried by FreeBSD 2.0 (second in the source order);
2. FreeBSD 2.0's, **42 lines (44%)**, which its own header credits to
   MacLachlan's 1992 patchkit changes. Whether MacLachlan held a
   separate notice is **not established here**: FreeBSD 2.0's file
   carries only the Regents' header and a credit line.

FreeBSD 2.0 is the **earliest source I could read that has the
page-crossing and `priorpage` fixes**. 386BSD 0.1 is earlier but has
the hole; the patchkit itself (1992), where the fixes were first
published, is not among the sources available, so an earlier text
that is both clear and fixed could not be searched for.

## What could not be tested

- **A lower bound.** A renamed or reflowed line counts as new, so
  copying with edits is under-counted (the lineage tool's own limit).
- **386BSD 0.0 is taken to be Net/2's** for `isa.c` 7.2, the stamp on
  both. Net/2 itself was not read.
- **The 1992 patchkit** and FreeBSD 1.x (1993) are not available
  here: the first FreeBSD tag is 2.0.
- **The settlement's lists** were not consulted.
- **Behaviour.** I read `at_dma`; I did not run it, and did not
  reproduce the zeros-at-32-MB failure.
- **Authorship inside the 48.** The 0.1 lines are not shown to be
  Jolitz's rather than someone else's patchkit text; the file's
  header is the Regents', which is all that was read.

## What this leaves for the integrator

The following are recommendations; no source or shared document was
changed.

**Ledger row** (`clearance.md`, FreeBSD donor table):

```
| FreeBSD | 2.0, rev 1.35 (ache 1994/11/01); `ic/i8237.h` rev 1.2 | `i386/isa/isa.c` (`isaphysmem`, `isa_dmastart`, `isa_dmadone`, `isa_dmarangecheck`, `DMA1_*`/`DMA2_*`), `i386/isa/ic/i8237.h`, `i386/i386/pmap.c` (`DMAPAGES`, `isaphysmem`) | **measured, 2026-11-01** (`floppy-dma.md`): of 95 distinct lines in the DMA region, 3 are Lite2's `at_dma` (generic comments), **0** are 386BSD 0.0's and not Lite2's, 48 (51%) are first in 386BSD 0.1 (Regents' notice), 42 (44%) first in FreeBSD 2.0 (credited to MacLachlan, 1992), 2 ours. Net/2 itself not read; the 1992 patchkit not available; MacLachlan's own notice not established. Clear, residue named | Regents' (386BSD 0.1 lines); FreeBSD 2.0 header credits Frank P. MacLachlan 92/08/18; `i8237.h` has no notice | d72ca64a; #373 |
```

**Source change, one, small:** add to the "code to manage AT bus"
comment of `usr/src/sys/i386/isa/isa.c` the credit FreeBSD 2.0 carries
(`92/08/18 Frank P. MacLachlan (fpm@crash.cts.com): Fixed
uninitialized variable problem and added code to deal with DMA page
boundaries in isa_dmarangecheck(). Fixed word mode DMA count
compution and reorganized DMA setup code in isa_dmastart()`). The
tree took the code and left the credit out. Also correct the
AI-ONLY NOTE's "line for line" for NetBSD 1.0 and OpenBSD to the
60 and 58 of 95 measured here. Replacing the code is not recommended:
nothing earlier and clear carries the fixes, and 4.6BSD replaces the
ISA layer (`plan-4.6.md`).
