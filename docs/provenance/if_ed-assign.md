# `if_ed` assigns where it compares

plan-4.5.md stage 2, item 5; the sweep's #443 (`audit-bestfit.md`
item 6).

## The defect

`usr/src/sys/i386/isa/if_ed.c`, `ed_probe_WD80x3()` (not `ed_attach()`,
a separate, later function), in the `ED_TYPE_SMC8216C` /
`ED_TYPE_SMC8216T` case (both types share one `case` label, told apart
afterward):

```c
if (sc->type = ED_TYPE_SMC8216C) {
	sc->type_str = "SMC8216/SMC8216C";
} else {
	sc->type_str = "SMC8216T";
}
```

and the same pattern again lower down, for the 8K-memory variants
(`SMC8416C`/`SMC8416T`). `=` where `==` was meant: the condition is
always true (`ED_TYPE_SMC8216C` is a non-zero constant, `0x2b`), so the
`else` branch -- the SMC8216T/SMC8416T report -- can never be taken,
and `sc->type` itself is overwritten. A genuine SMC8216T board
(`sc->type` holds `0x2a`) is both misreported as an SMC8216C in the
attach message and left with its own type field silently rewritten to
`0x2b`. Nothing downstream in this function reads `sc->type` again
before it would matter, so the rewrite is latent rather than causing a
second, visible fault -- but it is the same class of bug.

## The fit (first canonical fit) and the alternatives

Lite2's import has the bug, unchanged from 4.4BSD-Lite2. The donors:

| candidate | what it has |
|---|---|
| NetBSD, OpenBSD | `if_ed.c`'s SMC8216/8416 branch was not found in either donor's history for this file (search run here did not isolate it; **not verified either present or absent**) |
| FreeBSD, commit `26f0d64ac` (1995-11-15, "Brought in changes from rev 1.85: fix broken probe for 8216/8416 cards") | **introduces** the SMC8216C/8216T split, and introduces the bug with it: before this commit the two types shared one flat `case` with no comparison at all |
| FreeBSD, commit `4ddc73e65` (1996-03-31, "Fix a bogus message: ed0: Invalid irq configuration...", an unrelated fix to the same file) | already has `==` in both places; the fix itself landed in some earlier commit this mirror's history does not isolate cleanly (see below) |

**The exact fixing commit is not recoverable from this mirror.** This
donor tree's history around the turn of 1996 mixes real commits with
cvs2svn-synthesized ones for branch and tag creation, and at least one
such synthetic commit (`3b7e1cc80f`, dated 1996-01-01, whose own stated
purpose -- "Bump libutil revision" -- has nothing to do with this file)
already carries the `==` fix while a release-branch tag from the same
day (`RELENG_2_1_0_RELEASE`, commit `a3ab491d64`) still has the bug,
meaning the fix landed on trunk before some already-branched release
line picked it up. `git merge-base --is-ancestor` shows the two
commits are not in a direct ancestor relationship in this mirror's
graph at all, so the graph itself cannot be used to order them here,
only the two commits' own dates: the bug is present as late as the
2.1.0 release tag and absent by `4ddc73e65` (1996-03-31). This mirrors
the NetBSD 1.2 case in `elf-short-read.md` ("a single... squash, so the
fix's own date is not recoverable"): the fix is credited to FreeBSD,
dated no later than 1996-03-31, with the exact commit and the earlier
audit's specific claim of "2.1.7" both left unconfirmed rather than
restated as fact.

The fix itself is one character, twice, in both FreeBSD's version and
this tree's: there is no design choice to weigh, only the question of
whether `=` should be `==`, which it should.

## The fix

Both guards changed from `=` to `==`, with an AI-ONLY NOTE at the
first explaining what the bug did.

## The test

QEMU's NIC emulation is `ne2000`, not the WD790/SMC8216 family this
code path identifies, so there is no hardware, real or emulated, to
reach this path by booting a kernel here. `build/test/if_ed_type.c`
lifts both guarded `if` statements out of the real source verbatim
(copy-pasted, not retyped) -- the 16K-memory report and the nested
8K-memory (`ED_WD790_RAR_SZ8`) report, each with its own pair of
strings -- into a tiny host harness and drives each with both board
types, run by `build/test/if_ed_type.sh`, which first greps the real
`if_ed.c` to confirm the embedded copies have not drifted from it: the
shared condition line (no remaining `=` form; exactly two `==` guards)
and, separately, each guard's own four strings, so a drift in either
guard's body is caught, not only a reversion of the shared condition.

| case | pre-fix | fixed |
|---|---|---|
| SMC8216T board (`type` = `0x2a`) | reported "SMC8216/SMC8216C"; `type` rewritten to `0x2b` | reported "SMC8216T"; `type` unchanged |
| SMC8216C board (`type` = `0x2b`) | reported "SMC8216/SMC8216C"; `type` unchanged | reported "SMC8216/SMC8216C"; `type` unchanged |
| SMC8416T board (8K variant, `type` = `0x2a`) | reported "SMC8416C/SMC8416BT"; `type` rewritten to `0x2b` | reported "SMC8416T"; `type` unchanged |
| SMC8416C board (8K variant, `type` = `0x2b`) | reported "SMC8416C/SMC8416BT"; `type` unchanged | reported "SMC8416C/SMC8416BT"; `type` unchanged |

So the test fails (shows the wrong report and the silent rewrite) on
the pre-fix statement and passes on the fixed one, run back to back in
the same program, for both guards.

### Correction (same patch, before delivery)

The first version of this test and this section both claimed "both
guarded `if` statements" were copied verbatim, but only the first
(16K-memory) guard's body actually was; the second (8K-memory) guard
shared the first's condition line but not its own two strings, so
nothing in the test or the drift-check could have caught that guard's
body drifting from the real source. An independent reviewer found
this by deliberately corrupting a scratch copy of the second guard's
string and showing the suite passed anyway. Both the test
(`decide_pre_8k`/`decide_post_8k`, four more checks) and the
drift-check (now greps for all four strings, not just the shared
condition) were corrected before this patch, and the same corruption
re-tried against the corrected script now fails as it should.

Regression: `build/kernel.sh GENERIC.i386` builds clean (the same
pre-existing, unrelated pointer-type warnings as before this patch,
nothing new); `build/test/copyio.sh` passes on the resulting kernel,
confirming the kernel still boots and this driver's file still
compiles and loads correctly as part of it.

## What is not established

- Whether NetBSD or OpenBSD's `if_ed.c` ever carried this same
  SMC8216/8416 code, with or without the bug: not found by the search
  run here, not ruled out.
- Real SMC8216/SMC8416/WD790 hardware behavior: this tree has never
  run on it, under QEMU or otherwise; the test is of the lifted
  decision logic, not of the driver attaching to a real board.
- Whether `sc->type`'s silent rewrite to `ED_TYPE_SMC8216C` has any
  further visible effect elsewhere in `ed_attach()` or later in the
  driver's life for an SMC8216T board; not traced past this function.
