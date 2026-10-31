# `deroff` has three separate buffer-overflow holes

plan-4.5.md stage 2, item 5. Three unrelated overflow bugs in
`usr/src/usr.bin/deroff/deroff.c`, folded into one patch because all
three sit in the same file and the same 2002 import, and all three
are closed by later commits on the same donor branch.

Our tree's `deroff.c` (as of the last commit before this patch,
`31e1c3c6`) is byte-identical to OpenBSD's own original import of
this file -- `25a69cdca46` (Todd Miller, two commits,
2002-02-28, "This is the 4.4BSD deroff, now freed by Caldera.") --
except for one already-applied fix, a `strcpy()` with overlapping
arguments changed to `memmove()` (same commit, same AI-ONLY NOTE,
already in the tree before this patch). Confirmed directly with
`diff -u` against that commit's own tree. So this genuinely is "the
2002 code," not a later revision of it.

## Hole 1: `chars[]` is sized 128, indexed up to 255

`chars[]` is declared `char chars[128]` and is used, among other
places, in `msputwords()` as `chars[(unsigned char)*p1]` -- an input
byte with its high bit set (0x80-0xFF) indexes 128 bytes past where
the array actually ends.

## Hole 2: `regline()`'s line buffer has a fixed size and no bound check

`line` is a fixed `char line[LINE_MAX]` (`LINE_MAX` is 2048 here).
`regline()`'s own copying loop lets `lp` reach exactly
`line + LINE_MAX` and then writes there anyway -- both the loop's
own last character and the terminating NUL written after the loop.
One input line of `LINE_MAX` bytes or more with no embedded newline
overflows this array by at least one byte.

## Hole 3: `inpic()`'s `.PS`/`.PE` quoted-text copy has no bound at all

Inside a `.PS`/`.PE` block (troff's `pic` preprocessor syntax),
`inpic()` copies a quoted string's contents into `line` one byte at
a time with no length check whatsoever -- any quoted text long
enough overflows `line` regardless of hole 2's fix, since this copy
never checks against `line`'s size at all, fixed or dynamic.

## The fit (first canonical fit)

Checked directly against the donor trees (`/home/claude/donors/`).

**FreeBSD does not carry `usr.bin/deroff/deroff.c` at all** --
confirmed, `git show main:usr.bin/deroff/deroff.c` fails with "path
... does not exist."

**Correction, same patch, before delivery:** I initially believed,
and said in passing while investigating this file, that NetBSD also
does not carry it. That was wrong, and I am retracting it rather
than letting it stand. NetBSD *does* carry
`usr.bin/deroff/deroff.c` (`$NetBSD: deroff.c,v 1.14 2025/02/20
19:32:16 rillig Exp $` in the current tree), imported "taken from:
OpenBSD: deroff.c,v 1.6 2004/06/02 14:58:46 tom Exp" -- a 2004
OpenBSD snapshot, well before any of the three fixes below (2014,
2023, 2026). Checked NetBSD's current tree directly
(`chars[128]`, a fixed `char line[LINE_MAX]`, no `.PS` length
check) and confirmed it has picked up none of the three fixes:
whatever syncing NetBSD has done with OpenBSD since 2004, this file
was not among it, at least not for these three holes. So NetBSD is
a real donor for this file, just not an earlier (or even equal) one
for any of these three holes; it does not change the outcome, but
the original claim that the file was simply absent was false and is
corrected here rather than repeated.

OpenBSD has all three, each in its own small, non-synthetic commit;
`git log`/`git blame` on all three is clean (unlike several other
files in this tree with cvs2svn mirror-graph irregularities), so
dates and authors are taken directly:

| candidate | date | what it has |
|---|---|---|
| OpenBSD, `7c6b5cdc9fb` (deraadt, "chars[] array needs to be sized 256 because it is indexed at many places by (unsigned char) from input data. from Renaud Allard") | 2026-04-01 | `chars[256]` |
| OpenBSD, `08e9b19a4d0` (millert, "Use a dynamically-allocated line buffer and resize as needed. Fixes a buffer overflow for lines over 2048 bytes. Problem reported by Crystal Kolipe.") | 2023-09-27 | `line` becomes `size_t linesz; char *line;`, grown in `regline()` |
| OpenBSD, `1e6d196437f` (millert, "Fix buffer overflow in .PS parsing. Found by afl and based on a diff from jsg@.") | 2014-12-03 | adds a bound check in `inpic()`'s quoted-string copy |
| NetBSD, current tree, per its own embedded `$NetBSD:` string | 2025-02-20 | none of the three; still `chars[128]`, fixed `line[LINE_MAX]`, no `.PS` bound |
| FreeBSD | -- | does not carry this file |

OpenBSD is the only donor with any of the three fixes, so there is
no donor-order tiebreak to make.

## A latent bug in OpenBSD's own current tree, not ported

OpenBSD's `inpic()`, in its own latest available tree here
(`origin/master`), still computes the quoted-text bound as:

```c
ep = line + sizeof(line) - 1;
```

This was correct when `inpic()`'s 2014 fix (`1e6d196437f`) was
written, because `line` was still the fixed array `char
line[LINE_MAX]` at that point. OpenBSD's own later commit
(`08e9b19a4d0`, 2023-09-27) changed `line` to `char *line;` a
pointer, for the dynamic-growth fix above -- and left this one
`sizeof(line)` expression in `inpic()` unchanged. `sizeof(line)` on
a pointer is the size of the pointer itself (8 bytes on a 64-bit
host, 4 on this tree's own 32-bit i386 target), not the buffer's
real size, so OpenBSD's own current `.PS` handling enforces a bound
of a handful of bytes instead of the buffer's real size -- far more
restrictive than intended, and arguably a bug of its own (though not
a security one: it under-reads, not over-reads). Confirmed directly:
`git show origin/master:usr.bin/deroff/deroff.c` still has this
exact line, with `char *line;` a few lines above it.

This tree's `line` is also a pointer, by the same fix, so copying
this expression verbatim would carry the same mistake forward. The
fix applied here instead computes the bound from `linesz` (the
buffer's current real size, tracked alongside `line` since the 2023
fix), which is what the 2014 fix actually intended and what OpenBSD's
own code did correctly before 2023:

```c
ep = line + linesz - 1;
```

This is a deliberate, documented deviation from literal donor code
--the AI-ONLY NOTE at the site says so -- consistent with this
project's standing practice of adapting rather than copying verbatim
when the literal donor text is itself wrong (see also the `at`
Y2K patch's own minimal, non-verbatim adaptation).

## The `reallocarray` adaptation

OpenBSD's 2023 commit itself -- checked directly,
`git show 08e9b19a4d0 -- usr.bin/deroff/deroff.c` -- grows the
buffer with `reallocarray(line, linesz, 2)`. This tree's own libc
does not carry `reallocarray()` at all (confirmed:
`usr/src/lib/libc` has no such file or symbol), so
`realloc(line, linesz * 2)` is used instead, the same
size-doubling policy without that function's extra guard against
the multiply overflowing. This does not matter in practice here:
`linesz` starts at `LINE_MAX` (2048) and only ever doubles, so it
would take over 60 doublings (past `2^51` bytes) to even approach
where `linesz * 2` could overflow a `size_t` on this tree's 32-bit
target, long before any real input or any real memory budget would
get there.

## What is not established by any of these fixes: negative-index `chars[]` access

**Correction (same patch, found by independent review, before
delivery):** this section originally named `chars[c]` in
`comline()` as the example site. That was wrong and is retracted
here rather than restated: `comline()`'s `c` is only ever assigned
from `getc()` (0-255, with EOF handled separately via `eof()`) or
small positive character literals, so `chars[c]` there is never
actually negative. The independent reviewer found the real,
reproducible site instead: `msputmac()` and `meputmac()` (used for
`-ms`/`-mm` and `-me` respectively) both do `last = *(t - 1);` (or
`last = np[-1];`) -- dereferencing a `char *` into a plain `int`,
with no `(unsigned char)` cast -- and then index `chars[last]`
directly. On this tree's target, `char` is signed, so a high-bit
input byte (e.g. `0xFF`) sign-extends through `last` to a negative
`int`, and `chars[last]` reads before the start of the array
regardless of its size -- `chars[256]` does not fix it, because the
problem here is the index's sign, not the array's size. Confirmed
directly, reproducibly, on the already-patched binary:

```
$ printf '.ZZ hello\377\n' | ./deroff -ms
```

crashes under ASan with `global-buffer-overflow ...
deroff.c:946 in msputmac`. Checked OpenBSD's own current tree
(`origin/master`) and confirmed the identical unfixed `chars[last]`
pattern is still present at both sites there too -- no donor fix
exists for this, in neither this nor OpenBSD's own latest code.
It is a real, open, but distinct hole from the three this patch
closes; left as a known, documented limitation rather than fixed
here, since there is no donor fix available for it in
first-canonical-fit terms (closing it would mean writing original
code, which is 4.6BSD's standard, not 4.5BSD's).

## The fix (summary)

- `chars[128]` -> `chars[256]`.
- `char line[LINE_MAX];` -> `size_t linesz; char *line;`, with
  `linesz = LINE_MAX; line = malloc(linesz);` in `main()` and a
  grow-by-doubling check at the top of `regline()`'s copying loop.
- `inpic()` gets a `p1 + 1 >= ep` bound check before each byte it
  copies into `line`, erroring out (`errx(1, ".PS length exceeds
  limit");`) rather than overflowing, with `ep` computed from
  `linesz` rather than OpenBSD's own (here, stale) `sizeof(line)`.

Each AI-ONLY NOTE at the three sites in `deroff.c` cites its own
commit and explains its own adaptation (the `linesz`-vs-`sizeof`
deviation, the `reallocarray`-vs-`realloc` adaptation) in place.

## The test

`deroff`'s vulnerable code paths span several interacting functions
and file-scope state (the growing `line` buffer shared across
`regline()`/`inpic()`/`msputwords()`, `inpic()`'s own small state
machine) -- not one function includable or callable standalone the
way `at`'s `assign_date()` was for `build/test/at_fixes.c`. So
`build/test/deroff_overflows.sh` builds the real
`usr/src/usr.bin/deroff/deroff.c` itself, with the host's own `cc`
(`-D'__dead='`, since Lite2's `__dead` macro isn't visible to a host
build), and drives the resulting binary against three crafted
inputs, one per hole, checking exit codes and output rather than
comparing against a separate `.c` harness's own printed transcript
-- the "T ..." lines it emits and diffs against
`build/test/deroff_overflows.want` are the script's own narration of
each check, in the same spirit as the other tests' harnesses. It
also greps the real source first (`chars[256]`, `char *line;`, the
`.PS length exceeds limit` message) so it cannot silently drift from
what's actually in the tree.

### Crash-before / clean-after, shown by hand

The script above proves the *fixed* binary behaves correctly; it
does not re-demonstrate the pre-fix crash on every run (its own
drift-check refuses before reaching a pre-fix build, which is
correct -- that's not its job once the fix is in place). The
pre-fix crash was shown once, by hand, with a saved pre-fix copy of
the file (`git show HEAD:usr/src/usr.bin/deroff/deroff.c`, the
commit immediately before this patch) built the same way plus
`-fsanitize=address`:

```
gcc -std=gnu89 -w -g -fsanitize=address -D'__dead=' \
    -o /tmp/deroff_pre /tmp/deroff_prefix/deroff.c
gcc -std=gnu89 -w -g -fsanitize=address -D'__dead=' \
    -o /tmp/deroff_host usr/src/usr.bin/deroff/deroff.c
```

- **Long line** (5000 `'a'` bytes plus a trailing newline, no
  embedded newline before it): pre-fix build aborts under ASan,
  `global-buffer-overflow ... deroff.c:510 in regline`,
  "located 0 bytes after global variable 'line' ... of size 2048";
  post-fix build exits 0 with the line's content preserved exactly.
  (A version of this input with *no* trailing newline produces empty
  output and exit 0 on *both* builds -- not a sign the bug isn't
  there, but `eof()`'s own unconditional `exit(0)` when input ends
  with no more files, which fires before anything is printed either
  way. The trailing newline is needed so the line actually
  completes and gets processed.)
- **Oversized `.PS` quoted string** (`.PS`, a `dummy` line, a quoted
  string of 5000 `'x'` bytes, `.PE`): pre-fix build aborts under
  ASan, `global-buffer-overflow ... deroff.c:803 in inpic`; post-fix
  build exits 1 with `.PS length exceeds limit` on stderr, no crash.
  A short, legitimate `.PS` block (`.PS`, `dummy`, `"short"`, `.PE`)
  still passes through unchanged post-fix, confirming the bound
  check doesn't false-trigger on ordinary input. (The `dummy` line
  is needed: `inpic()` unconditionally skips the rest of the line
  right after `.PS` itself -- ordinary `pic` syntax, where that line
  is normally a sizing spec, not content -- so a quoted string placed
  directly after `.PS` with no line of its own in between is simply
  never reached.)
- **High-bit byte under `-w`** (`hello \200 world\n`, run with the
  `-w` flag so `msputwords()`'s `chars[(unsigned char)*p1]` path is
  actually reached -- plain `textline()` without `-w` never indexes
  `chars[]` at all): pre-fix build aborts under ASan,
  `global-buffer-overflow ... deroff.c:896 in msputwords`, "located 0
  bytes after global variable 'chars' ... of size 128"; post-fix
  build exits 0, output `hello` then `world` on separate lines (the
  high-bit byte itself, once correctly classified via the now
  large-enough `chars[]`, is treated as a word separator).
  (A single byte of `0xE9` alone, tried first, did not crash either
  build -- 105 bytes past a 128-byte array landed outside ASan's own
  redzone instrumentation for that global, not a sign the bug was
  absent. `0x80`, the very first out-of-bounds index, immediately
  adjacent to the array's end, does crash reliably and is what the
  permanent test and this doc both use.)

Regression: cross build verified clean,
`cd usr/src/usr.bin/deroff && sh ../../../../build/make.sh` --
`/root/.cache/lite2/tools/bin/i386-unknown-bsd4.4elf-gcc -O2 -o
deroff ...`, no errors or warnings.

`build/test/deroff_overflows.sh` itself was run repeatedly against
the fixed source with identical "T ..." transcripts each time, and
separately confirmed to refuse outright (before building anything)
against a scratch copy with the pre-fix `chars[128]` size restored,
by its own drift-check grep.

## What is not established

- The negative-index `chars[]` access (a plain signed `char`/`int`
  used to index `chars[]` without a `(unsigned char)` cast) --
  documented above as a known, separate, still-open hole with no
  available donor fix; not closed by this patch.
- Whether any donor other than OpenBSD independently discovers and
  fixes any of these three holes at some later date than the
  commits cited here: not traced further for NetBSD past its
  current tree (already confirmed to have none of the three) or for
  any donor beyond OpenBSD, FreeBSD and NetBSD, since OpenBSD's own
  fixes already establish the earliest fit for all three and no
  other checked donor has any of them at all.
- Whether `deroff`'s other parsing paths (tbl, eqn markup, macro
  package handling) have further, similarly-shaped overflows not
  reached by the three crafted inputs used here: not swept
  exhaustively; only the three holes actually identified and fixed
  by the cited donor commits were tested.
