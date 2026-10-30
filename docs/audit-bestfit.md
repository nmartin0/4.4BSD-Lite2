# The best-fit sweep of every patch

Done 2026-10-30 at the maintainer's request: every patch since the
4.4BSD-Lite2 import (`b3d2855f`) judged against the best-fit rule
(`design-4.6.md` rule 1a). The verdict for each patch is in
`provenance/bestfit-sweep.tsv`, one line per code patch.

## Method

- **All 708 patches** (`b3d2855f..` patch 238) were sorted by what they
  touch:

  | Kind | Patches |
  |---|---|
  | userland code | 209 |
  | kernel code | 188 |
  | both | 17 |
  | docs | 168 |
  | third-party (`contrib/`, `gnu/`) | 68 |
  | build tools | 56 |
  | other | 2 |

- **The 294 patches that are not code** were checked only for touching
  BSD code outside `contrib/`, `gnu/` and `share/doc`. None does.
- **The 414 code patches were measured.** For each BSD-native file a code
  patch touched (819 file versions), our result was compared with the
  same file in 23 donor snapshots, measuring:
  - how many of Lite2's lines each candidate changes;
  - how close each candidate is to our version;
  - whether it contains the lines our patch added.

  The snapshots were OpenBSD 1996-06 to 2014-06, NetBSD 1.0 to 4,
  FreeBSD 2.0 to 4.0, and DragonFly 1.2 and 2.0. The script is
  `fit.py` in the session's scratch area, not in the tree.
- **Every code patch was then read** against those measurements and
  the donor sources, and given a verdict on the six points and the
  tie rule:

  | Verdict | Meaning |
  |---|---|
  | KEEP | the chosen source is the best fit |
  | BETTER-FIT | another candidate fits Lite2 better |
  | MORE-COMPLETE | the same line has later corrections |
  | DONOR-EXISTS | own writing where a donor had the fix |
  | PROVENANCE-CONCERN | a clearance problem |
  | NEEDS-MAINTAINER | a judgement call |

- **The findings below were checked again against source** where marked
  "(checked)".

Third-party programs inside `usr/src` (sendmail, awk, cron, at) were
judged only on their choice of version; the rule keeps them as they are.

## Result

| Verdict | Patches |
|---|---|
| KEEP | 354 |
| MORE-COMPLETE | 22 |
| PROVENANCE-CONCERN | 14 |
| NEEDS-MAINTAINER | 11 |
| BETTER-FIT | 7 |
| DONOR-EXISTS | 6 |

By type, 163 were donor fixes, 81 build glue, 60 own fixes, 37 donor
imports, 30 adaptations, 19 docs in the source tree, 16 third-party
and 8 other.

**Most of the tree passes.** Of the 60 that do not:

- 18 were already put right by later patches;
- 6 need only their attribution corrected (one of them, #54, also a
  small change of style);
- 36 call for a change or a decision: 21 defects, 3 provenance
  problems and 12 decisions for the maintainer.

## Open: defects in the tree today

These are bugs in the current tree that a better or more complete fit
would have avoided. In order of severity:

1. **`copyout` does not check that its destination is in user space**
   (#126, #145; checked). Lite2's C `copyout` refused addresses at or
   above `USRSTACK`. Patch 126 deleted it in favour of the assembly one,
   which never had the check; `locore.s` has no `VM_MAXUSER_ADDRESS`
   test. On a 486 or later, a program can pass a kernel address to
   `read()` and overwrite kernel memory. The fit is NetBSD 1.0's
   `copyout` (`sys/arch/i386/i386/locore.s`), which adds the length,
   faults on carry and compares with `VM_MAXUSER_ADDRESS`.
   - The 386 path patch 145 added has two further bugs (checked). After
     `call trapwrite`, the loop advances from the return value in
     `%eax` instead of the saved address. And `jnz cpyflt` leaves two
     words on the stack.
   - Lite2-first, the alternative is Lite2's own C `copyout` for the
     386.
   - `copyin` has no bound check either; NetBSD 1.0 relies on the
     user segment.
2. **ELF loader accepts a short read** (#207; checked). NetBSD 1.1's
   `elf_read_from` returns success when `resid != 0`. NetBSD 1.2 returns
   `ENOEXEC`. One line.
3. **`bwrite` touches the buffer after an asynchronous write** (#252,
   #254; checked). `SET(bp->b_flags, B_AGE)` after `VOP_STRATEGY` on a
   buffer `biodone` may already have released. NetBSD 1.1 removed it,
   and corrected `getblk` and `getnewbuf` in the same functions.
4. **`compress` overruns its decode stack on a crafted file** (#526;
   checked). FreeBSD's fix for CVE-2011-2895 (in 9.0, and in NetBSD 8)
   adds the bad-stream checks to `zread`, about 15 lines on the same
   Lite code.
5. **`deroff` indexes `chars[128]` with any byte** (#564; checked), and
   lacks OpenBSD's 2014 `.PS` overflow fix and its 2023 long-line fix.
   All are later fixes on the same 2002 code.
6. **`if_ed` assigns where it compares** (#443; checked). `if (sc->type =
   ED_TYPE_SMC8216C)` twice. FreeBSD 2.1.7, the same branch and driver
   interface, fixes it with others.
7. **`uucpd` changes directory as root and never sets the login name**
   (#586; checked). OpenBSD's 1996 fixes to the same `doit()`.
8. **`at` treats two-digit years as 19xx and tests the wrong pointer**
   (#483; checked). OpenBSD's and NetBSD's 1999-2000 fixes.
9. **`test` gets three-argument `-a`/`-o` wrong** (#504; checked).
   FreeBSD 2.2's one-line fix, which patch 504's own tests exercise.
10. **`mkdir -p` writes '/' over the path's terminating NUL** (#452;
    checked). FreeBSD's later line in the same `build()`.
11. **`sa` passes `char **` to `%s`** (#512; checked). NetBSD 1.3.
12. Smaller ones:
    - `re_comp("")` recompiles, and `re_comp` takes `char *` against
      Lite2's own prototype (#75). NetBSD 1.1 and 1.4.
    - `pstat`'s mount-flag table keeps Lite2's duplicate entry with a
      null name (#78). NetBSD 1.3.
    - ddb's ELF symbol reader lacks its author's same-day sanity checks
      (#646). NetBSD rev 1.16.
    - `wcd` lacks FreeBSD 2.1.7's table-of-contents bounds checks (#318).
    - The ELF loader's specified-address arithmetic is wrong but never
      used (#208).
    - `strtoimax`'s "0x" handling (#343).
    - `tip`'s `uucplock` `snprintf` (#587).
    - Two man-page corrections (#370, #567).

## Open: provenance

- **`execve`** (#209, #211): written by reading the settlement-removed
  4.4 `kern_exec.c`, with lines of 4.4's `getxfile` quoted in code and
  comment. This is the A2 item already in hand: the replacement from
  NetBSD's exec framework, which must now be chosen by fit (NetBSD 1.0,
  1.1 or later) rather than by date.
- **The floppy DMA code** (#373): FreeBSD 2.0's `isa.c` credits it to a
  1992 contribution of the 386BSD patchkit era. NetBSD 1.0 and OpenBSD
  carry it line for line. It has no clearance-ledger row.
- **FreeBSD 2.1.0's `wd.c`** (#318) carries the Net/2 "7.2" version
  through 386BSD and FreeBSD 1.x. It has no ledger row; it is already
  due to leave with the driver work.
- **The clearance ledger itself** (`docs/provenance/clearance.md`) does
  not exist yet; several reviews needed it.

## Open: decisions for the maintainer

- **#264 `sh`'s `getpwd`:** OpenBSD's 4-line change keeping Lite2's
  fork path, or ours, which replaces about 30 Lite2 lines with the later
  complete loop.
- **#273 `expr`:** NetBSD 1.0's `expr.y`, a dead end in its own line, or
  OpenBSD's maintained `expr.c` (NetBSD 1.1's, first in donor order).
- **#296 the boot blocks:** FreeBSD's BTX (its own design) or 4.4's
  libsa carried forward in NetBSD's or OpenBSD's i386 boot. This bears
  on the UEFI decision already recorded.
- **#318, #319 the disk driver:** FreeBSD 2.1.0's `wd.c` with the slice
  layer and a wider `minor()`, already due to leave with the driver work.
- **#380 `ptrace`:** FreeBSD 2.2's, or NetBSD 1.0's Lite-based rewrite,
  which also keeps `PT_READ_U`/`PT_WRITE_U`. NetBSD's `PT_WRITE_U` is
  unchecked.
- **#442 stack limit:** raising `MAXDSIZ` to 128 MB also raised the
  maximum stack (Lite2 defines `MAXSSIZ` as `MAXDSIZ`). FreeBSD 2.0
  sets 64 MB, NetBSD 1.0 8 MB.
- **#474 `ps_strings`:** keep the `char **` interface every donor
  adopted, or fix Lite2's `char *` form with a small guard of our own.
- **#686 `microtime`:** the donors' assembly, or our C rewrite in
  `clock.c`.
- **#106-#108 `_C_LABEL`:** 370 lines of Lite2 assembly rewritten, or a
  kernel-only compiler flag that would leave them untouched.
- **#20 the ELF start-up files:** NetBSD 1.6's, right for GCC 2.95.3, or
  NetBSD 2.0's, robust for a newer compiler.
- **#342 `<stdint.h>`:** NetBSD 2's i386 types (`intptr_t` as `int`,
  `int_fast16_t` as `int`), an ABI choice.

## Attribution only (the code stays)

- #66 `register_t`: NetBSD 1.1 and OpenBSD 1996 have it (checked).
- #48: FreeBSD 3.0 installs the same headers.
- #629, #630: OpenBSD has both ddb fixes, and a BSD donor comes before
  Mach in the source order.
- #683: OpenBSD's 2010 `rtcstop()`.
- #54: OpenBSD's 1997 `tip` declarations fit Lite2's style with fewer
  lines.
- From the reviews, several commit messages credit a later release
  than the one that first had the change (#9, #12, #31, #55, #56) or
  cite the CSRG history for a line that also exists in a copyable
  tree (#109).

## Already put right by later patches

#14, #121, #148, #155, #157, #171, #180; the `vfs_bio.c` bodies written
from encumbered text (#190-#198, #200), removed and replaced by NetBSD
1.0's file; Berkeley's proprietary-marked `diff3.sh` (#406, #407),
removed by #412.
