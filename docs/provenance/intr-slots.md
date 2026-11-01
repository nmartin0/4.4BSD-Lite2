<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# The stray-interrupt counter collision

plan-4.5.md stage 2, item 7; `roadmap-audit.md`'s "#687's stray-vector
counter ... [is] still unfixed."

This fix is original to this tree. It is not a donor port: no donor
of the period indexes this table any other way than this fix now
does (FreeBSD 2.0 is corroboration, not a source -- see below). Per
`precedent.md`'s order, own code comes "last of all, and only where
the above have nothing"; that is the case here.

## The defect

Patch 222 (`usr.sbin/config/mkglue.c`'s `vector()`) generates
`vector.s` for the i386 `config` tool. For every `isa`-attached
device in the kernel config, it used to assign that device's entry
in the 16-longword `isa_intr` counter array (`sys/i386/isa/icu.s`)
a **sequential** slot, counted in the order devices appear in the
config file -- `clk` fixed at slot 0, `rtc` fixed at slot 1, then
`count = 2; ...; ++count` for every other probed `isa` device.

That counter has nothing to do with the device's hardware IRQ line.
But `icu.s`'s 16 hardcoded, always-present "stray" interrupt vectors
(`IDTVEC(intr0)` through `IDTVEC(intr15)`, wired to every IDT entry
by `isa_defaultirq()` in `sys/i386/isa/isa.c` before any device is
probed) index that same array by the **raw IRQ number** -- as does
`INTR_SPUR1`/`INTR_SPUR2`'s spurious-interrupt check for IRQ 7 and
15 (those count into a separate `spurintr[2]`, but the stray vectors
for every other unclaimed line count straight into `isa_intr`).

Two unrelated interrupt sources can therefore land on the same
`isa_intr` slot: a configured device's sequential count, and some
other, unconfigured IRQ line's stray count. `sys/i386/conf/
GENERIC.i386`'s own device list demonstrates this is not a
theoretical gap -- it is three live collisions in the kernel this
tree ships:

| device | real IRQ | old sequential slot | collides with |
|---|---|---|---|
| `pc0`  | 1  | 2  | stray IRQ 2 (unconfigured) |
| `com0` | 4  | 7  | stray IRQ 7, `INTR_SPUR1`'s line |
| `wt0`  | 5  | 10 | stray IRQ 10 (unconfigured) |

(`npx0`, `wdc0`, `wdc1`, `fd0`, `ed0`, `ed1` land on slots 3, 4, 5,
6, 8, 9 in config order, none of which happen to be unclaimed IRQs
in this particular config -- so this is undercounted even for
`GENERIC.i386`, and a different device list collides differently.)
Each hit on the colliding slot -- whichever source caused it --
silently inflates the other source's reported count. `vmstat -i`'s
per-line interrupt counts are wrong for any of these lines, with no
indication anything is wrong.

## The fix

`vector()` now assigns every device's counter slot as **its own IRQ
number**, matching `icu.s`'s pre-existing, unchanged convention
exactly: `clk` is slot 0 (IRQ 0), `rtc` is slot 8 (its real IRQ, not
the old fixed slot 1), and every `isa` device takes `dp->d_irq`
directly. A `claimed[16]` table catches both out-of-range IRQs and
two devices claiming the same line (which subsumes the old implicit
clk/rtc reservation). All 16 slots get a name unconditionally --
`"irq%d"` for a line nothing claims -- rather than only the slots a
device happened to occupy.

This still fits in the existing `isa_intr: .space 16*4`; nothing
about the array's size changes, only which slot each name lands in.
`eintrcnt` is fixed at `isa_intr + 64` (16 longwords) rather than
computed from a running count, since the table is now always full.

A full table with placeholder names for unconfigured lines is safe
for every consumer checked: `usr.bin/vmstat/vmstat.c`'s `dointr()`
already skips any slot whose count is zero (`if (*intrcnt) printf(...)`)
while still consuming it from the parallel `intrcnt`/`intrname`
arrays, so an all-16-slots table changes nothing it prints, only
which slot gets attributed to which device.

## Corroboration, not a source

FreeBSD 2.0's `sys/i386/isa/vector.s` (`/home/claude/donors/freebsd`,
tag `release/2.0`) has a static, always-present 16-entry `INTR(0..15,...)`
table with default names `"stray irq0"` through `"stray irq15"` --
the same IRQ-indexed, always-full-table shape this fix adopts. This
is corroboration that IRQ-indexed slots are the established i386-BSD
answer to this exact problem, not a donor port: FreeBSD 2.0's
`usr.sbin/config/mkglue.c` at that same tag is the unrelated,
inherited VAX-style generic config tool (`cntcnt++` sequential
counting, `SCBVEC` labels) -- FreeBSD's actual i386 scheme lives in
the static `vector.s` instead, a different file from the one this
tree's bug and fix are both in. NetBSD's own `mkglue.c` (checked in
`/home/claude/donors/netbsd`, branch `origin/netbsd-1-0`, and
`/home/claude/donors/nb12h`) is the same VAX-era `cntcnt++` tool,
confirmed not applicable to the i386 port's actual needs either.
Neither donor's `mkglue.c` was ported; the fix here is this tree's
own, reasoned from `icu.s`'s own existing convention and confirmed
against FreeBSD's independent, static solution to the same problem.

## Testing

`build/test/mkglue_irq_slots.sh` runs a standalone model
(`mkglue_irq_slots.c`) of both the old sequential scheme and the new
IRQ-indexed scheme against `GENERIC.i386`'s real `isa` device list
(`pc0`, `npx0`, `wdc0`, `wdc1`, `fd0`, `com0`, `ed0`, `ed1`, `wt0`,
in their config-file order), and fails if the old scheme does *not*
reproduce the three collisions above, or if the new scheme produces
any collision. It first greps the real `mkglue.c` for the fix's
anchors (the `rtc` slot, the `claimed[]` array, the `dp->d_irq`-keyed
naming, `eintrcnt`'s fixed offset) so it cannot pass against a
reverted or drifted source.

The model does not link against `mkglue.c` itself (entangled with
`config`'s own `struct device`/`dtab` parsing); it reproduces the two
schemes' arithmetic directly, which is what the grep check above is
for. Beyond the model, the actual generated output was inspected
directly: `usr.sbin/config` was rebuilt from source (the build
normally runs a cached binary -- see "what was not established"),
`build/kernel.sh GENERIC.i386` was re-run, and the resulting
`usr/src/sys/compile/GENERIC.i386/vector.s` was read in full. It has
all 16 `.asciz` names in IRQ order (`clk`, `pc0`, `irq2`, `ed1`,
`com0`, `wt0`, `fd0`, `irq7`, `rtc0`, `ed0`, `irq10`, `irq11`,
`irq12`, `npx0`, `wdc0`, `wdc1`), `eintrcnt` set to `isa_intr + 64`,
and every `VEC(...)` body's `INTR1`/`INTR2` `offst` argument equal to
that device's own IRQ (`wdc1`, IRQ 15, correctly keeps `INTR_SPUR2`
immediately before its own `INTR2(1, biomask, 15)`). The kernel
linked and loaded to the same `vmunix` size class as before (the
counter table's layout, not its size, changed).

## A second change considered and not made

While working on the fix above, `icu.h`'s `INTR_SPUR1`/`INTR_SPUR2`
were also found to write `spurintr` through whatever `%ds` happened
to hold at interrupt entry, before `INTR_HEAD`/`INTR_TAIL` reload
`%ds`/`%es` to the kernel's own selector -- `%ss`, not `%ds`, is the
segment the architecture actually guarantees valid there. An
`%ss:` override was drafted, built, and confirmed by `objdump` to
reach the compiled kernel (the `0x36` prefix byte before both
`incl spurintr` instructions).

It was then dropped at the maintainer's direction (2026-10-31): it
fixes nothing observable -- `sys/i386/i386/machdep.c`'s `gdt_segs`/
`ldt_segs` make both `GDATA_SEL` (kernel) and `LUDATA_SEL` (user)
flat, base 0, limit 4 GB, `SDT_MEMRWA`, so `%ds` is always usable for
this write regardless of which context was interrupted, and no `#GP`
is reachable under this tree's current segment layout -- and the
maintainer asked for the real fix in this item, not a defensive
override with no live bug behind it. `icu.h` is therefore unchanged
by this patch; `INTR_SPUR1`/`INTR_SPUR2` still read exactly as they
did before patch 222. Nothing here rules out revisiting this if the
tree's segment layout ever stops being flat, but that is not a
change made or claimed by this item.

## What was not established

- **The build-tool staleness this task ran into is itself a
  standing hazard, not fixed here.** `build/kernel.sh` runs a
  pre-built `$TOOLS/config` (cached at `$L2_BUILD/tools/bin/config`,
  normally built once by `build/sysroot.sh`) rather than rebuilding
  it from source. A source change to `mkglue.c` can silently fail to
  reach a built kernel without manually rebuilding that cached
  binary first. This is left as a gap for a build-system pass, not
  claimed as fixed.
- **No other `usr.sbin/config`-generated table was audited for the
  same sequential-vs-indexed mismatch.** Only the stray-interrupt
  counter (`isa_intr`, via `vector()`) was in scope for this item.
- **This has not been run on real or emulated hardware.** The
  `vector.s` output was read directly; no boot test (QEMU or
  otherwise) was performed as part of this patch.

## Independent review

Per `method.md`'s rule to get an independent reviewer who has not
seen my conclusions: a reviewer was asked to re-derive the three
`GENERIC.i386` collisions independently from the old sequential-
counting rule and the device list, and to check the new `claimed[]`/
`dp->d_irq`-indexed logic in `mkglue.c` for off-by-one or range
errors, without being shown this document's conclusions first. (That
review also covered the `%ss:` override drafted above, before it was
dropped; its findings on that point are no longer applicable to what
this patch actually ships, and are not repeated here.)
