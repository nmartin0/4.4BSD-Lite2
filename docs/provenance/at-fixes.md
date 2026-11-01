<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# `at` treats two-digit years as 19xx and tests the wrong pointer

plan-4.5.md stage 2, item 5; the sweep's #483 (`audit-bestfit.md`
item 8). Two unrelated bugs, both in `usr.bin/at`, folded into one
patch because the sweep folded them into one item.

## Bug 1: a two-digit year is treated as already being `tm_year`

`usr/src/usr.bin/at/parsetime.c`, `assign_date()`:

```c
if (year > 99) {
	if (year > 1899)
	    year -= 1900;
	else
	    panic("garbled time");
}
...
if (year >= 0)
	tm->tm_year = year;
```

A four-digit year (`> 99`) is converted to a `tm_year` value
(years since 1900) by the first block. A plain two-digit year (0-99,
e.g. typed as `"00"`) skips that block untouched and is assigned
straight into `tm_year` as-is -- but `tm_year` *is* a two-digit-ish
count since 1900, so a user-typed `"00"` meaning the year 2000
becomes `tm_year == 0`, the year 1900. Every other year someone
would actually type two digits for (`"01"` through roughly `"68"`
or `"69"`, depending on era) has the same problem once the clock
passes 2000.

### The fit (first canonical fit)

Lite2's import has the bug, unchanged from 4.4BSD-Lite2. Checked
directly against the donor trees:

| candidate | date (see below) | what it has |
|---|---|---|
| OpenBSD, `5ec155afad6` ("Merge in changes from at 2.9") | 1997-03-01 | still has the exact Lite2-shape bug (same two-block structure, just retabbed) |
| NetBSD 1.2 (`origin/netbsd-1-2`) | (NetBSD 1.2, 1996) | has the bug, byte-identical in structure to Lite2's |
| **NetBSD 1.3 (`origin/netbsd-1-3`)** | **1998-10-24** | **has the fix**: a two-digit year below 69 (the Unix-epoch century pivot) is treated as 21st-century |
| OpenBSD, `a2ba2f107e4` (PR 778, alex, "Handle 2-digit year values post-Y2K") | 1999-03-21 | has a fix, but a later and differently-designed one (see below) |

**NetBSD is earliest.** NetBSD 1.3's `parsetime.c` identifies
itself by its own embedded CVS keyword as
`$NetBSD: parsetime.c,v 1.5.2.1 1998/10/24 00:22:35 cgd Exp $` --
read directly from the file's own content (`git show
origin/netbsd-1-3:usr.bin/at/parsetime.c`), not from this mirror's
commit graph. That graph has the same irregularity already recorded
for other files in this tree (`elf-short-read.md`, `if_ed-assign.md`,
`sa-argv.md`): `git blame` on this file's fixed lines attributes
them to a single synthesized commit (`8b8d5a4d4`, dated 2001-01-18)
whose own stated purpose is unrelated to this file at all. The
CVS-keyword date embedded in the file's own text is trusted here
instead, the same way this tree has trusted such embedded identifiers
elsewhere when the graph itself cannot be. 1998-10-24 is about five
months earlier than OpenBSD's PR 778 fix (1999-03-21), and OpenBSD's
own `at 2.9` merge eight months before that (1997-03-01) is confirmed
to still have the bug, so NetBSD's fix is not simply a copy of
something OpenBSD had all along.

**NetBSD's commit is not a minimal, single-purpose fix.** The
single CVS revision that introduces it also pulls in an unrelated,
much larger resync (a whole-file retab from spaces to tabs, function
prototypes, day-of-week parsing, an AM/PM fix, pluralization
checking, and more) -- bundled together with the Y2K fix in one
`cgd` commit on 1998-10-24, since NetBSD was resyncing from a newer
upstream snapshot of the same third-party `at` package at the time.
Importing all of that here would be several unrelated holes at once,
not "the first fix of this hole." Instead, only the specific,
narrow behavior change is ported: a two-digit year below 69 is
treated as 21st-century. NetBSD's own code expresses this with its
`TM_YEAR_BASE` macro, which this tree does not carry (introducing it
is itself part of what the OpenBSD PR 778 diff bundles in, along
with a new `<tzfile.h>` include); this tree's version keeps Lite2's
existing hardcoded-1900 style instead:

```c
if (year >= 0 && year < 69)
	year += 100;
```

This is behaviorally identical to NetBSD's `if (year < 69) tm_year =
year + 2000 - TM_YEAR_BASE; else tm_year = year + 1900 -
TM_YEAR_BASE;` for every year value that can reach it, including a
shared limitation inherited directly from NetBSD's own fix: because
the check runs on whatever `year` holds after the four-digit block
above (not only on genuinely-two-digit input), a four-digit year
already reduced to a value below 69 (a year between 1900 and 1968,
inclusive, typed out in full, e.g. `"1950"`) also gets the century
bumped, becoming 2050. This is not a defect introduced here -- it is
the exact behavior of NetBSD 1.3's own applied fix, checked directly
against its branch content above -- and in practice it is caught by
`parsetime()`'s own existing "Trying to travel back in time" check
for any date that is actually in the past, which a typed-out
`"1950"` almost always would be.

OpenBSD's PR 778 fix (1999-03-21) uses a different, more careful
design -- comparing the given two-digit year against the current
`tm_year % 100` with an explicit one-year grace period, rather than
a fixed 69 pivot -- refined again on 2000-01-05. That is a real
improvement (4.6BSD's concern, by best fit, not 4.5BSD's), but it is
not the first fix found, so it is not what 4.5BSD carries.

## Bug 2: `writefile()` checks the wrong pointer

`usr/src/usr.bin/at/at.c`, `writefile()`, writing out the job's
captured environment:

```c
eqp = strchr(*atenv, '=');
if (ap == NULL)
	eqp = *atenv;
else {
	...
	eqp++;
}
```

`ap` here is `char *ap`, declared once at file scope in `main()`'s
locals and reused across this function's unrelated earlier loop
(building the queue-file name, `for (ap = ppos; *ap != '\0';
ap++)`), which leaves it pointing at that string's own terminating
NUL -- never `NULL` -- by the time this code runs. `strchr()` just
above is what can actually return `NULL`, for an environment string
with no `=` in it at all; checking `ap` instead means that case is
never caught, and the code falls into the `else` branch and
increments `eqp` (which may itself be `NULL`) before using it.

### The fit (first canonical fit)

Checked directly against the donor trees:

| candidate | date | what it has |
|---|---|---|
| NetBSD 1.2, 1.3 (`origin/netbsd-1-2`, `origin/netbsd-1-3`) | -- | both have the bug: `if (ap == NULL)` |
| **OpenBSD, `45108c9efd4`** (millert, "Fix thinko, check eqp for NULL, not ap"; reported by ath@algonet.se) | **2000-04-23** | **has the fix**: `if (eqp == NULL)`, a clean, single-line, non-synthetic commit |
| NetBSD 1.4 (`origin/netbsd-1-4`) | 2000-05-05 (per the file's own `$NetBSD: at.c,v 1.12.2.1 2000/05/05 13:22:44 he Exp $`) | has the same fix, about two weeks later |

OpenBSD is earliest here, and -- unlike the Y2K half above -- this
fix needs no adaptation: `eqp` already exists in this tree's code,
exactly as in OpenBSD's. The fix is the one-word change `ap` to
`eqp`.

## The fix

Both changed as above, each with an AI-ONLY NOTE.

## The test

`build/test/at_fixes.c` tests both halves:

- `assign_date()` has no side effects reaching the filesystem or
  the network, so the test `#include`s the real
  `usr.bin/at/parsetime.c` textually (as `build/test/mkdir_nul.c`
  does for `mkdir.c`) and calls the real, unmodified function
  directly. `panic()` is stubbed to `longjmp()` back into the test
  instead of exiting, so the deliberate panic case (a year before
  1900, unaffected by this patch) can be checked rather than ending
  the test program.
- `writefile()` opens real files, forks a mail pipeline, and reads
  the real environment, so it is not practical to call directly
  here (the same reasoning as `build/test/sa_argv.c` for
  `usr.sbin/sa`'s `main()`): the `eqp`/`ap` mistake is reproduced
  standalone instead, built the same way the real bug is made (a
  stale pointer left over from walking off the end of an unrelated
  string, which is never `NULL`) against an environment string with
  no `=` in it, the one case the real code needs `eqp`'s `NULL` to
  be caught.

`build/test/at_fixes.sh` first greps the real `at.c` to confirm it
still reads `eqp == NULL`, not `ap == NULL` (the half of the test
that cannot `#include` the real source and so cannot be kept in
sync any other way), then builds and runs the harness with the
host's own `cc`.

| case | pre-fix | fixed |
|---|---|---|
| `assign_date()`, year `00` | 1900 | 2000 |
| `assign_date()`, year `25` | 1925 | 2025 |
| `assign_date()`, year `68` | 1968 | 2068 |
| `assign_date()`, year `69` | 1969 (unaffected either way) | 1969 |
| `assign_date()`, year `99` | 1999 (unaffected either way) | 1999 |
| `assign_date()`, year `2026`, `1999` (four-digit) | unaffected | unaffected |
| `assign_date()`, year `1950`, `1968` (four-digit, pre-1969) | 1950, 1968 | 2050, 2068 (the shared limitation above, checked, not just described) |
| `assign_date()`, no year given | stays at `now`'s year | stays at `now`'s year |
| `assign_date()`, year `1899` | panics either way | panics either way |
| `eqp`/`ap`, no `=` in string | `ap` check misses it (never `NULL`) | `eqp` check catches it |

Ran `build/test/at_fixes.sh` three times in a row with identical
output each time (`assign_date()` here has no randomness or
uninitialized state to make it flaky, unlike `sa_argv.c`'s
fork/pipe capture).

An independent reviewer asked for the pre-1969 four-digit case
(above) to be covered by an actual assertion, not just described in
this doc's prose; added (`probe("year 1950 ...", 1950, 0, 2050)` and
the same for 1968) before this patch was committed.

Shown to actually fail on the pre-fix code, for both halves:

- Built a scratch copy of the test tree with both fixes manually
  reverted (`year >= 0 && year < 69` removed; `eqp == NULL` changed
  back to `ap == NULL`) and reran the test against it: the three
  genuinely two-digit years (`00`, `25`, `68`) came back wrong
  (1900, 1925, 1968) exactly as the bug predicts, while `69`, `99`,
  the four-digit years, and the no-year and pre-1900 cases were
  unaffected either way, matching the table above.
- Separately, reintroduced just the `at.c` half of the bug in the
  real tree (`eqp == NULL` back to `ap == NULL`) and reran
  `build/test/at_fixes.sh`: the drift-check caught it and stopped
  before even building ("`at.c` still has the pre-fix 'ap == NULL'
  form"), then the fix was restored and the file diffed back to
  exactly the intended change (12 insertions, 1 deletion -- the
  `eqp` fix and its AI-ONLY NOTE, nothing else).

Regression: cross build verified clean,
`cd usr/src/usr.bin/at && sh ../../../../build/make.sh`.

## What is not established

- The exact NetBSD commit/author/date for the Y2K fix: only that
  NetBSD 1.2 has the bug and NetBSD 1.3 has the fix, per the
  mirror-graph irregularity above; the embedded CVS date
  (1998-10-24, cgd) is trusted, but the commit itself is not
  isolated from the larger resync it arrived with.
- Whether NetBSD's bundled resync (day-of-week parsing, the AM/PM
  fix, pluralization checking, prototypes) has holes of its own that
  this tree should eventually close separately; not evaluated here,
  since none of it was imported.
- Real behavior of `at(1)` actually scheduling and running a job
  with a two-digit-year date or with an environment variable that
  has no `=`: the Y2K half is tested against the real, unmodified
  `assign_date()`; the `eqp`/`ap` half is tested as a standalone
  reproduction of the exact mistake, not through a full `at(1)`
  invocation.
