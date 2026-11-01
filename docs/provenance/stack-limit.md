<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# The stack limit (#442): MAXSSIZ back to Lite2's 32 MB

Stage 4, item 1 of `plan-4.5.md`. The plan says to take FreeBSD 2.0's
`MAXSSIZ` (64 MB). The evidence below says otherwise, and this patch
does something different.

## What #442 did

`usr/src/sys/i386/include/vmparam.h` took FreeBSD 2.0's `DFLDSIZ` 64 MB
and `MAXDSIZ` 128 MB (Lite2: 6 and 32) for GCC 2.95.3's C++ compiler.
Lite2 defines `MAXSSIZ` as `MAXDSIZ`, so the stack's maximum went from
32 MB to 128 MB by side effect. `deferred.md` (patch 165) says the
opposite was decided on 2026-10-18: "The text and stack limits stay
Lite2's." So the stack was never meant to change.

## The donors (donor files exported as plain files; 386BSD has none)

| | MAXTSIZ | DFLDSIZ | MAXDSIZ | DFLSSIZ | MAXSSIZ |
|---|---|---|---|---|---|
| Lite2 (this tree before) | 6 MB | 6 MB | 32 MB | 512 KB | = MAXDSIZ (32 MB) |
| 386BSD 0.0, 0.1 | no `vmparam.h` for i386 in either (the export is a 65-byte error stub) | | | | |
| NetBSD 1.0, 1.1, 1.2 | 8 MB | 16 MB | 256 MB | 512 KB | 8 MB |
| FreeBSD 2.0 | 16 MB | 64 MB | 128 MB | 8 MB | 64 MB |

NetBSD 1.0 (October 1994, from memory, not re-checked here) is earlier
than FreeBSD 2.0 (November 1994), and `precedent.md` lists NetBSD first
among the contemporaries. So if a donor line were wanted for the stack,
it would be NetBSD 1.0's 8 MB, not FreeBSD 2.0's 64 MB, and the plan's
wording does not follow the source order. The same goes for the data
limits #442 took: NetBSD 1.0's 16 MB / 256 MB is the earlier fit if
16 MB is enough for cc1plus. That was not tested and is not changed
here; it is raised for the maintainer.

## Why no donor line

By the source order Lite2 comes first, and Lite2 has a stack maximum:
32 MB. Nothing in this tree needs another value, and the 2026-10-18
decision says it stays. A donor's number is for where Lite2 has no fit.
So the value is Lite2's own 32 MB, written as a literal because the
`MAXSSIZ = MAXDSIZ` coupling is what carried the change in.

## Why not 8 MB or 64 MB, and what a decoupled limit breaks

Any `MAXSSIZ` below `MAXDSIZ` has a second effect. `dosetrlimit` caps
RLIMIT_STACK at `maxdmap` (= `MAXDSIZ`), and `exec_aout_setup_stack`
(`exec_subr.c`) maps the PROT_NONE part of the stack as
`(USRSTACK - rlim_cur) - (USRSTACK - MAXSSIZ)`. The super-user may set
a limit above `MAXSSIZ`; then that size wraps, the exec's `vm_allocate`
fails and the process is killed. With `MAXSSIZ` equal to `MAXDSIZ`, as
in Lite2, this cannot happen. Measured, with only `MAXSSIZ` set to
NetBSD 1.0's 8 MB: the root test case (limit 128 MB, then exec) was
killed by signal 6. Every later exec of a root shell that had said
`unlimit stacksize` would die. (The 32 MB case without the cap was not
run; the mechanism is the same.) Hence the second change below. Whether
NetBSD 1.0's or FreeBSD 2.0's `kern_resource.c` caps the stack at
`MAXSSIZ` could not be read (the donor trees are git-only and the brief
forbade git there); so the cap was first this tree's own line, flagged.
(Superseded: NetBSD 1.0's own lines were found and replace it; see the
integrator's section at the end.)

Layout (`exec-stack.md`): `ep_maxsaddr = USRSTACK - MAXSSIZ` and the
data/mmap area start far below either value (USRSTACK is 0xEFBFE000),
so 32 MB or 128 MB makes no clash. A smaller `MAXSSIZ` only shrinks the
PROT_NONE reservation below the stack, from 128 MB to 32 MB of address
space, which costs no memory. The `- NBPG` gap of `round_page(MAXSSIZ)`
in 261's quotation is gone from the code; nothing else uses it.

## The change

- `sys/i386/include/vmparam.h`: `MAXSSIZ (32*1024*1024)`, comment
  updated. `DFLSSIZ` stays 512 KB (Lite2 and NetBSD 1.0; FreeBSD 2.0's
  8 MB is not taken).
- `sys/kern/kern_resource.c` and `sys/vm/vm_glue.c`: RLIMIT_STACK is
  capped at `maxsmap` instead of `maxdmap`, with `unsigned maxsmap =
  MAXSSIZ` beside `maxdmap`: NetBSD 1.0's lines (the integrator's
  change, below; the worker's first form was two lines of its own).

## Other users of the old value

`grep MAXSSIZ DFLSSIZ` over the tree: `kern/exec_subr.c` (maxsaddr),
`vm/vm_glue.c` (`rlim_max` of a new process), and the gdb/binutils
`bfd/hosts/i386bsd.h` files (`HOST_STACK_END_ADDR = USRSTACK - MAXSSIZ`,
which are user-space headers that compile against the target's
`<machine/vmparam.h>`; they follow whatever it is). `trap.c` and
`vm_unix.c` compare against `rlim_cur`, not `MAXSSIZ`. Other ports'
`vmparam.h` are not touched. Nothing depends on 128 MB.

## Test

`build/test/stacklimit.c` / `.want` (run by `sh build/test/ktest.sh
stacklimit`). It reads the stack maximum, then execs itself with the
soft limit at the maximum and touches a byte N MB below the stack
pointer (one page, not N MB: QEMU has no swap).

The test sizes its cases from the maximum it reads, so the old kernel
ran the same cases at its own 128 MB:

| case | old kernel | new kernel |
|---|---|---|
| stack maximum | 128 MB | 32 MB |
| max - 1 MB | 127 MB ok | 31 MB ok |
| max + 1 MB | 129 MB SIGSEGV | 33 MB SIGSEGV |
| 129 MB (past the old maximum) | SIGSEGV | SIGSEGV |
| root, limit 128 MB, then exec | exec ok | exec ok (limit capped to 32 MB) |

The old kernel fails on the maximum line, which the `.want` file fixes
at 32 MB, and so on the cases sized from it.

Results: the test **FAILS on the old kernel** (maximum 128 MB, grew
127 MB, 129 MB killed) and **PASSES on the new** (10 lines). `execve`
PASSES on the new kernel (63 lines). With `MAXSSIZ` alone at 8 MB (no
cap), the root case is killed by signal 6, as above.

Not tested: the C++ compiler or any real program at 32 MB; FreeBSD 2.0's
`kern_resource.c` (NetBSD 1.0's was read by the integrator); the 16 MB / 256 MB data limits;
other ports (not built); the release dates above.

## Ledger and imports

Ledger row: `sys/i386/include/vmparam.h` `MAXSSIZ`: Lite2's 32 MB value
restored (not a donor line); `sys/kern/kern_resource.c` and
`sys/vm/vm_glue.c`: the stack cap at `maxsmap`, NetBSD 1.0's
(`kern_resource.c` rev 1.20, `vm_glue.c` rev 1.39). Imports entry: none for a donor; the table
above records that NetBSD 1.0 (8 MB) and FreeBSD 2.0 (64 MB) were
measured and not taken.

## Integrator's change (2026-11-01)

The worker capped RLIMIT_STACK at MAXSSIZ with two lines of its own,
having not read the donors' `kern_resource.c`. NetBSD 1.0's has the
same fix as donor text: `case RLIMIT_STACK` caps at `maxsmap`, declared
beside `maxdmap` (`extern unsigned maxdmap, maxsmap;`), and
`vm/vm_glue.c` defines `unsigned maxsmap = MAXSSIZ;` beside Lite2's
`maxdmap`. Those lines replace the worker's, so nothing here is own
writing but the comments. Lite2 itself caps the stack at `maxdmap`
(checked in the tree's first commit of `kern_resource.c`), which was
right only while MAXSSIZ was MAXDSIZ. The value, 32 MB, is Lite2's
MAXSSIZ as Lite2 had it (MAXDSIZ, 32 MB), written as a number now that
MAXDSIZ is 128 MB; NetBSD 1.0's 8 MB and FreeBSD 2.0's 64 MB were
measured and not taken, per the decision of 2026-10-18 that the stack
limits stay Lite2's (`deferred.md`). The test was run again on the
integrated kernel.

The test's root line now reads the limit back after asking for 128 MB,
so it checks the cap (`gets: 32 MB`); before, it passed on the old
kernel too (found by the stage's reviewer). Binaries built against the
old `MAXSSIZ` in `bfd/hosts/i386bsd.h` (gdb, binutils) take the new
value when rebuilt.
