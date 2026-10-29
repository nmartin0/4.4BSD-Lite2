# How little machine-dependent code each port has needed

Measured 2026-10-29 at the maintainer's request. The question: across
the BSD releases, which had the least machine-dependent code for each
port, and when? It serves a standing goal, secondary to POSIX
conformance but never in tension with it: **as much of the system as
possible in generic C, with just enough assembly for the machine to run
well, on every port the system supports.** Supporting a port is not in
question; the aim is that each port costs as little machine-dependent
code as the hardware forces (`design-4.6.md`, rule 5).

## Method

For each port and release, the lines of `.c`, `.h`, `.s` and `.S` in
the port's kernel directory, and in the shared directories it uses
(`m68k` for hp300 and luna68k, `mips` for pmax and newsmips, NetBSD's
`x86` for i386 and amd64 from 2.0). Boot programs (`stand`, `boot`),
kernel configurations and compile directories are left out.

- **core** is the port's own machine code: the directory named after the
  CPU or machine (`i386/i386`, `hp300/hp300`, ...), `include`, and the
  core of any shared directory. That is locore, trap handling, the pmap,
  context switch, machdep, interrupt dispatch, the bus code and the
  headers.
- **other** is everything else under the port: drivers and bus
  attachments kept in the machine directory, emulations, floating-point
  emulators.
- **asm** is the assembly within core.

Donors: Lite2 (this tree's import), NetBSD 1.0-1.6, 2-8 and trunk,
OpenBSD from 1996 to 2026, FreeBSD 2.0-9.0. A few FreeBSD releases could
not be read completely (FreeBSD 2.2.0's kernel, three FreeBSD libc
releases) and are left out. Lines measure text, not function: a
smaller port that does less is not more efficient. The measuring
script is `mdcount.py` in the session scratchpad; it can go into
`build/` if the maintainer wants the measurement repeatable.

## The kernel: the smallest core for each port, and when

Core lines / core assembly lines, the smallest release first.

| Port | Smallest | Then | And | Today |
|---|---|---|---|---|
| i386 | **Lite2 10,076 / 1,704** | NetBSD 1.0 16,114 / 2,338 | OpenBSD 1996 20,185 / 2,272 | OpenBSD 43,880; NetBSD 77,028 (with x86) |
| amd64 | **OpenBSD 2004 28,741 / 2,661** | FreeBSD 6.0 31,108 / 2,227 | NetBSD 2.0 36,984 / 3,175 (with x86) | OpenBSD 59,894; NetBSD 76,845 |
| hp300 | **Lite2 15,945 / 3,122** | NetBSD 1.0 21,705 / 3,511 | OpenBSD 1996 22,116 / 3,069 | NetBSD 41,045 |
| luna68k | **Lite2 12,432 / 2,641** | NetBSD 1.5 23,588 / 3,247 | -- | NetBSD 38,622 |
| sparc | **Lite2 18,559 / 4,549** | NetBSD 1.0 20,472 / 4,702 | OpenBSD 1996 27,897 / 5,380 | NetBSD 51,723 |
| vax | **Lite2 9,682 / 1,296** | NetBSD 1.0 10,597 / 2,235 | OpenBSD 1996 14,072 / 2,169 | NetBSD 31,082 |
| pmax | **Lite2 20,244 / 6,763** | NetBSD 1.0 21,216 / 6,740 | OpenBSD 1996 23,600 / 6,793 | NetBSD 60,703 (with mips) |
| news3400 | **Lite2 9,297 / 3,264** | (NetBSD newsmips 1.5 30,247, with mips) | -- | NetBSD 57,261 |
| tahoe | **Lite2 2,239** | no daughter kept tahoe | -- | -- |

This tree's i386 today: 15,253 core lines, 2,325 of them assembly (up
from Lite2's 10,076 and 1,704: ddb, the ELF loader, the RTC and clock
work, the boot path).

**For every Lite2 port, Lite2 itself is the minimum.** Every daughter's
machine-dependent core grew from the first release on, and kept growing:
the i386 about four and a half times Lite2's by 2014 in OpenBSD, and
more than seven times in NetBSD today (counting its shared x86
directory). Assembly grew far less than C:
OpenBSD's i386 core assembly went from 2,272 lines (1996) to 3,930
(2026) while its C grew from 18,000 to 40,000. So the growth is machine
*function* written in C, not new assembly.

**For amd64, OpenBSD's 2004 port is the minimum** (it is NetBSD's of
2003, flattened into one directory), and FreeBSD's 6.0 port is close.

## Does sharing machine code between ports make each port smaller?

Measured, for i386 and amd64 together:

| | i386 | amd64 | shared x86 | Total |
|---|---|---|---|---|
| OpenBSD 2004 (no sharing) | 31,086 | 28,741 | -- | 59,827 |
| NetBSD 2.0 (shared x86) | 38,021 | 23,598 | 13,386 | 75,005 |
| NetBSD 4.0 | 38,694 | 24,629 | 20,420 | 83,743 |
| OpenBSD 2014 | 46,160 | 38,790 | -- | 84,950 |

Sharing did not make NetBSD's x86 smaller than OpenBSD's unshared pair:
what NetBSD put in `x86` was mostly new function (its interrupt
redesign, ACPI glue, MTRRs, CPU identification), not code that had been
duplicated. The m68k and mips shared directories tell the same story
with a twist: they hold real common code (pmax's own assembly fell from
6,740 lines in NetBSD 1.0 to 173 in 1.3 when it moved to `mips`), and
the m68k directory also holds Motorola's floating-point emulation
package, about 67,000-74,000 lines of assembly that only the
FPU-less 68k machines need (it is under "other", not core).

So sharing is worth doing where the code is truly common, and for
maintenance; it is not by itself what makes a port small. What makes a
port small is keeping function out of the machine directory.

## libc

Machine-dependent lines in libc (all of it, almost all assembly):

| | i386 | amd64 | m68k | sparc | vax | mips |
|---|---|---|---|---|---|---|
| Lite2 | **2,000** | -- | 4,038 (hp300) | 3,107 | 5,180 | 2,511 |
| NetBSD 1.0 | 3,658 | -- | 4,241 | 3,198 | -- | 2,375 |
| OpenBSD 1996 | 3,254 | -- | 4,556 | 3,435 | **1,944** | 2,626 |
| OpenBSD 2004 | 3,112 | **1,877** | 4,410 | 3,174 | 2,386 | 2,516 |
| OpenBSD 2026 | 2,630 | 3,915 | -- | -- | -- | -- |
| NetBSD 4.0 (with `common/lib/libc`) | 4,283 | 3,846 | 6,812 | 3,712 | 3,447 | 3,578 |
| NetBSD trunk (with `common/lib/libc`) | 4,969 | 4,345 | 8,205 | 4,099 | 3,523 | 5,403 |

Most of it is optimized string and memory functions (`bcopy`, `memcpy`,
`strlen`) and the system call stubs. The stubs are needed on every port.
The string functions already exist in portable C in Lite2's own
`lib/libc/string`; per-port assembly versions exist only for speed.

## What it suggests for 4.6BSD

1. **Lite2 is the floor, and the standard to measure against.** For each
   port, 4.6BSD's machine-dependent core should be measured against
   Lite2's figure above, each patch stating its growth, as the
   retention of Lite2 is measured.
2. **Function goes to machine-independent C where it can.** The
   daughters' growth was mostly C in the machine directory: bus code,
   interrupt establishment, CPU identification, ACPI glue, MP bring-up.
   The 4.6 rules already place drivers in `sys/dev` behind
   `bus_space`/`bus_dma`; the same test applies to the rest: only what
   touches the processor or its privileged state is machine-dependent.
3. **Assembly only where C cannot do it:** the trap and interrupt
   entry, context switch, the first instructions after boot, the system
   call stub, `setjmp`/`longjmp`, and the few instructions C17 has no
   form for. C17's `<stdatomic.h>` and the compiler's builtins replace
   per-port atomic and spin-lock assembly; libc's string functions stay
   in portable C unless a measurement shows a port needs assembly.
4. **Share between ports only what is truly common,** as NetBSD's `mips`
   does; a shared directory is not itself a saving.
5. **For amd64, OpenBSD's 2004 port is the smallest starting point**, as
   well as the canonical one (`analysis-4.6-continuations.md`).
