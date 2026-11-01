<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# microtime: NetBSD 1.0's microtime.s, in place of our C

Plan 4.5 stage 4 item 1 (#686). The sweep had decided `microtime` with
our own C; the source order puts a donor text first. This records the
import, the measurement of ours against the donor for `microtime` and
for `resettodr`, and what the test shows and does not.

## What was taken

`usr/src/sys/i386/i386/microtime.s`: NetBSD 1.0's
`sys/arch/i386/i386/microtime.s`, `$Id: microtime.s,v 1.8 1994/05/04
02:07:24 mycroft Exp $`, Steve McCanne's, under the Regents' 1993
notice with the advertising clause, which the file keeps with its
RCS tag. Read from a plain export of the NetBSD 1.0 tree
(`netbsd-1-0.microtime.s`); NetBSD 1.1 and 1.2 were read alongside.
1.1 (v1.16, cgd, April 1995) differs in being shorter, in testing
`ipending` first, and in scaling by 3433/4096 instead of 1000/1193;
1.2 is identical to 1.1. 1.0 is the earliest, so 1.0 is taken.
FreeBSD 2.0's `microtime.s` was not read for this; our C had been
written after FreeBSD 2.0.5's, which the first line of the old note said.

Changes, each labelled `AI-ONLY NOTE` in the file, and no others:

| change | why |
|---|---|
| `<machine/asm.h>` replaced by `_C_LABEL` and `ENTRY` written out | this tree has no i386 `asm.h`; the two definitions are locore.s:98 and :180's |
| `isareg.h` is `isa.h`; `timerreg.h` is new | `timerreg.h` holds the TIMER_ lines that `clock.c` carried under a comment saying they belonged there: moved, none written |
| `#define LOCORE` before the includes | `icu.h` has C prototypes unless LOCORE is set, and the NORMAL_S rule does not set it |
| `_time` is `_C_LABEL(time)` | ELF |
| `orb _ipending,%al` cut | no soft-interrupt register in this port |
| OCW3 `0x0a` written before the IRR is read | `icu.h`'s `INTR_SPUR1` leaves the in-service register selected; the donor assumes the request register |
| `cli ... sti` is `pushfl ... cli ... popfl` | a caller with interrupts off must not get them back (an earlier draft cited locore.s:270-300 for the pair; those lines use it for CPU detection, not interrupts, and the citation is withdrawn) |
| entry named `microtime_8254` | see the next section |
| `files.i386` has `i386/i386/microtime.s standard` | |

The divide by 1193 stands: a whole tick (11932 counts) is 10001
microseconds there, which is what the old C note counted against.

## What stays ours

`isa/clock.c` keeps a `microtime()` of fourteen lines that calls
`microtime_8254` and will not answer less than its previous answer
within one tick. That check is not in the donor. It is patch 217's,
carried over with its logic unchanged, and it is the only code of ours
in this path. Why it is needed is in the next-but-one section. The
rest of the old C (about thirty lines) is gone.

## Measurement, ours against the donors

Counts by a script of the `lineage.py` kind (distinct substantive
lines of twelve letters or digits or more, whitespace folded; code
lines with comments stripped). Our text is what stood in `clock.c`
before this change.

| | total lines | code lines | substantive lines | shared with donor |
|---|---|---|---|---|
| microtime, ours (C, patch 217) | 83 | 39 | 53 | 0 |
| microtime, NetBSD 1.0 `microtime.s` | 133 | 46 | 82 | |
| resettodr, ours (patch 216) | 67 | 43 | 40 | 0 |
| resettodr, NetBSD 1.0 `clock.c:428` | 58 | 40 | 19 | |

Nothing is shared in either pair (one is C against assembler, the
other has different names throughout), so the count says nothing about
copying: only that two independent texts exist.

**Earlier.** The donor's `microtime.s` is 1994, older than Lite2
(June 1995); ours is 2026. Under the source order that decides it.
**Clearer.** Ours, to read: it is C, thirty-nine lines. The donor's
comment on the overflow heuristic is the better explanation and ours
quoted it. Clearer to read is not a reason; the rule is the order.

**`resettodr`: not a first-fit change, kept ours.** The donor's body
is 40 code lines but is not the unit that would move. It calls
`rtcget` and `rtcput`, which call `rtcinit`, and uses `struct rtc_st`,
`RTC_NREG`, `RTC_NREGP`, `RTC_SET`, `RTC_VRT`, `RTC_UIP`, `RTC_HM`,
`dectohexdec`, `yeartoday`, the `month[]` table and the `timeset` flag
that only NetBSD's `inittodr` sets. This tree's `rtc.h` has none of
the structure or the names; its `inittodr` is the Jolitz one, which
reads with `rtcin` and uses DAYST/DAYEN. So the import is the whole
of NetBSD 1.0's RTC section, about 150 lines in `clock.c`, with
`inittodr` replaced. Three things would change in behaviour, and the
first is a fault:

1. `rtcinit`, run on the first `rtcget`, writes register B as
   `RTC_HM` alone. `enablertclock` here sets the periodic-interrupt
   bit in register B for the statistics clock; the first call of
   `resettodr` would clear it and stop `rtcintr`.
2. The donor applies `tz_dsttime`; this tree's `inittodr` applies
   DAYST/DAYEN and never reads `tz_dsttime`. A time set inside the
   window would be written back an hour wrong and read back wrong
   again.
3. `rtcget` tests `inb(IO_RTC+1) & RTC_VRT == 0`, which C reads as
   `& (RTC_VRT == 0)`: the validity check never fires.

Ours is the inverse of the `inittodr` in this tree, as its own note
says. Under "No original works" a function body is not to be written,
and this one was; the answer the rule gives is an import, and the
import is the RTC section, not the function. That is a larger change
than this item names and it is not taken here. It is left for the
maintainer, with the three points above.

## The test, and what it shows

`build/test/microtime.c` and `.want`, run by `build/test/ktest.sh
microtime`: `gettimeofday` in a tight loop for two seconds of the
kernel's clock. It asks that no answer is below the one before, that
`tv_usec` is under a million, that 100 or more tick boundaries were
crossed, and that more than 100 distinct values fall inside one tick.

Three runs, all kernels built by `sh build/kernel.sh`:

1. **The donor alone** (no check in `clock.c`): FAIL. 122 steps back
   in 859,751 calls, the worst a whole tick (10,000 us).
2. **The donor with `clock.c`'s check** (this commit): PASS. 298,914
   calls over 200 ticks, no step back, 3,183 distinct values in the
   best tick.
3. **A deliberately broken `microtime.s`** (the counter's microseconds
   zeroed before they are added, which is what this port did before
   patch 216): FAIL on the last line only, best window 1 value,
   largest step exactly 10,000 us. The file was restored and the
   kernel rebuilt.

`ktest.sh execve` was also run on the final kernel: PASS, 63 lines.

## What this does not show

QEMU's clock is emulated. QEMU TCG programs the 8254 and the 8259
from its own virtual timers, and the guest sees the counter and the
request register as software models that are not updated at one
instant. Run 1 is the evidence: the model raises IRQ 0 some counts
after the counter reloads, a window in which the donor's algorithm,
which is right if the chip raises the request at the reload, answers a
tick early. That is a statement about QEMU as measured on this machine
and about the donor on QEMU. It does not say the donor is wrong on a
real 8254, and the check in `clock.c` may never act on one; neither is
tested here, no hardware was run. Equally, that run 2 passes shows the
clamp hides the emulator's window, not that the donor's reading is
accurate: nothing here compares the microseconds against a second
clock, so the scale (10,000 us a tick, the counter's resolution of
0.84 us) is untested beyond monotonic and sub-tick. The largest step
printed by the test was 14,118 us in run 2 and 13,982 us in run 1:
more than a tick, so the emulator or its host stalled the guest; it
is printed and not judged for that reason.

Not tested: a machine with interrupts off calling `microtime` (the
`pushfl` change); `hz` other than 100 (the donor is the 100 Hz
version, `#ifndef HZ`, and hard-codes 11932, where the old C followed
`hz`: this is a regression for a kernel run at another `hz`, which no
configuration of this tree was checked for; the donor says the same
under `#ifndef HZ`, and a kernel that defined `HZ` on the compiler's
line would lose `microtime_8254` and fail to link, not run wrong); NetBSD 1.1's 4096
form, which does not have the 10001; a real RTC write by `resettodr`.
Nor was FreeBSD 2.0's `microtime.s` read; it exists as an export on
request.
