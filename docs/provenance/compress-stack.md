# `compress` can overrun its decode stack

plan-4.5.md stage 2, item 4; the sweep's #526 (`audit-bestfit.md`).

## The defect

`zread` (`usr.bin/compress/zopen.c`) decodes an LZW code by walking
`tab_prefixof()` back to a literal byte, pushing each byte onto
`de_stack` as it goes:

```c
while (code >= 256) {
	*stackp++ = tab_suffixof(code);
	code = tab_prefixof(code);
}
```

`de_stack` is the tail of the `htab` table, reused to save memory, and
nothing bounds `stackp` against the end of that table. A corrupted or
hand-crafted compressed stream -- one that uses the "KwKwK" special
case (`code >= free_ent`) together with a `CLEAR` code to leave a
prefix chain that loops back on itself -- makes the loop push forever,
running `stackp` off the end of `htab` and into memory that is not
part of the table at all. This is the public CVE-2011-2895.

## The fit (first canonical fit) and the alternatives

Lite2's `zopen.c` has the bug, unchanged from the 4.4BSD-Lite2 import.
Checked in the donor trees (dates are commit dates in the donor
repositories):

| candidate | what it has |
|---|---|
| NetBSD, `nbhist.git` / `netbsd` (1994 import through the mid-2000s) | not found by searching the `zopen.c` history; **not verified against a NetBSD release that may carry an equivalent fix earlier than OpenBSD's** |
| OpenBSD, commit `b530fee1e03` (2005-08-25, Todd Miller, "Prevent zs_stackp from going past the end of zs_htab... Based on a diff from miod@") | adds a bound check immediately before each push in the `while (code >= 256)` loop: `if (zs->zs_stackp >= (u_char *)&zs->zs_htab[HSIZE]) { errno = EINVAL; return (-1); }` |
| FreeBSD, `usr.bin/compress/zopen.c` (seen already in the tree between 2009 and 2012; exact commit not isolated) | a different, larger rewrite: refuses the "KwKwK" case outright when `oldcode == -1` or `code > free_ent`, rather than bounding the stack, and carries its own `EINVAL` paths |

OpenBSD's 2005 fix is the earliest found and is a minimal, local
change on the same function shape this tree already has, so it is
taken under the first-canonical-fit rule. FreeBSD's fix is later and
restructures more of `zread`'s KwKwK handling than this hole requires;
it is noted for 4.6BSD's best-fit pass but not pulled now, under the
no-duplicate-work rule.

## The fix

Added the same bound check, in this tree's own naming (`htab`,
`HSIZE`, `char_type`, `de_stack`) rather than OpenBSD's renamed
`zs_htab`/`zs_stackp`:

```c
if (stackp >= (char_type *)&htab[HSIZE]) {
	errno = EINVAL;
	return (-1);
}
```

placed, like OpenBSD's, inside the loop, before the push.

## The test

`build/test/zfuzz.c`, a host-side program (not a kernel test; `compress`
is a user program and the bug is in its own stack, not the kernel's).
It is deliberately a fuzzer, not a hand-built trigger: it makes a
genuinely valid `.Z` stream by running mixed repetitive/high-entropy
data through the tree's own `zopen()` in write mode, then flips a
random handful of bits in the compressed bytes (never the header) and
tries to read the result back through `zopen()` in read mode, under
AddressSanitizer, over many random seeds. `funopen()` is shimmed with
`fopencookie()` so the unmodified `zopen.c` builds and runs as a
32-bit host program (so `long` is 4 bytes, as on the i386).

Run both before and after the fix, 20000 trials per seed, seeds 1-10:

| build | trials x seeds | result |
|---|---|---|
| pre-fix (`7fc0a128`'s `zopen.c`) | 20000 x 10 | no overrun, every seed |
| with the fix | 20000 x 3 (reviewer, independently) | no overrun, every seed |

Round-trip of a well-formed stream (300000 mixed bytes through
`zopen()` write then read) is byte-identical on both builds: the fix
changes nothing for valid input (reviewer, independently; also true of
this tree's own 1994 test file through `compress`/`zcat`).

## What is not established

- **The overflow was not reproduced by this fuzzer, on either build.**
  `de_stack`'s capacity (`HSIZE * sizeof(count_int) - (1 << BITS)` =
  210468 bytes at `BITS` = 16, on a 32-bit build) is large enough that
  a single, acyclic KwKwK chain through the table cannot exceed it: a
  chain can visit at most `(1 << BITS) - 256` = 65280 distinct codes
  before it must terminate or repeat one, since `tab_prefixof` is a
  pure lookup with nothing mutating it mid-chain. 65280 is far short of
  210468. Only a genuine *cycle* in the prefix chain -- made by a
  `CLEAR` that resets `free_ent` while a stale, larger entry still
  points through now-reassigned smaller ones -- makes the loop run
  unbounded, and random bit-flip mutation of an otherwise-valid stream
  did not stumble onto one in the 200000 trials run here (10 seeds,
  both builds). An independent reviewer confirmed this reasoning by
  hand-building a direct two-code cycle in a standalone harness (not
  committed; scratch only): it drove `stackp` to exactly the computed
  capacity before the check fired, with zero ASan complaints below
  that point, matching the arithmetic exactly. The evidence that the
  unbounded case is reachable from a real corrupted file is OpenBSD's
  own commit message ("may happen for corrupted files... Based on a
  diff from miod@") and the public CVE, not a reproduction in this
  tree. This mirrors the buffer-cache race in `provenance/bufcache.md`:
  a real defect, shown here only by citing the upstream report and a
  direct structural argument, not by natural reproduction.
- The reviewer additionally checked that the fix's placement (one
  check, inside the `while (code >= 256)` loop) also covers the
  function's other two unguarded pushes to `stackp`
  (`*stackp++ = finchar` at the KwKwK branch, and the one right after
  the loop exits): the first always runs with `stackp == de_stack`
  because the forward-output loop always drains the stack back to
  `de_stack` before the branch can run again; the second is only
  reached when the `while` loop exits normally, which (by the same
  65280-code bound above) leaves `stackp` nowhere near the boundary,
  and is never reached after a cycle, since the loop's own check fires
  first in that case. So the single check is sufficient for all three
  pushes, but only because of this chain-length invariant -- not
  because of where the check sits by itself.
- Whether NetBSD carries an earlier equivalent fix: not found by the
  history search run here, but not exhaustively ruled out either
  (`netbsd-1-1`/`netbsd-1-2` tags were not individually diffed for
  this file).
- FreeBSD's fuller rewrite of the KwKwK handling, noted above for
  4.6BSD.
