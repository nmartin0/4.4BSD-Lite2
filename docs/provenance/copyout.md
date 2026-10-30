# copyin and copyout: the bound, and the 386 path

Done 2026-10-30 (patch 243, reviewed twice; see the commit message), plan-4.5.md stage 2, item 1; the first
of the best-fit sweep's defects (`audit-bestfit.md`, defect 1).

## The defect

`copyout` and `copyin` in `i386/i386/locore.s` did not check that the
user address they were given was in user space. They copy through the
kernel's flat segment, which reaches all of memory, so the segment
limit does not stop them:

- `read(fd, kernel_address, n)` overwrote kernel memory;
- `write(fd, kernel_address, n)` copied kernel memory out to the file.

(Write protection is a separate matter: CR0_WP, on a 486 and later,
makes the kernel honour read-only user pages; a 386 has no such bit,
which is why the 386 needs the page-by-page check in `copyout`.)

Measured on the kernel before this patch (QEMU, `-cpu 486`, an init
that tries it; the old kernel is not kept, so these figures are
**not reproducible from the commit**): `read` into 0xF0100000 returned
6 and wrote there; `write` from it returned 16; a read ending one byte
past USRSTACK returned 8 (the first of the kernel stack pages, which
sit above it). The commit holds the test and the way to run it on any
kernel; to see the before column, build the tree at patch 242 and run
`sh build/test/copyio.sh` on it.

How it got there: Lite2 had a C `copyout` in trap.c that refused any
range at or above USRSTACK, and an assembly one in locore.s that did
not; the two collided at link time. Patch 126 kept the assembly one;
patch 145 put back the 386 page walk that C `copyout` had, but not the
bound. `copyin` in Lite2's assembly never had one, and relied on
nothing: the fetch and store routines (`fuword` and the rest) use the
user segment through %gs, whose limit bounds them, but `copyin` and
`copyout` do not.

## The fit (4.5BSD, first canonical fit) and the alternatives

What the new code is, line by line, so the claim can be checked
(`diff` of locore.s against NetBSD 1.0's `copyout`/`copyin`):

- **NetBSD 1.0's structure**: the three-register prologue and argument
  offsets, the zero-length exit, the add-with-carry-then-compare bound,
  the page count (`PGOFSET`, `PGSHIFT`), the call to `trapwrite` with
  the two registers saved around it, the tail of the copy, and both
  epilogues (`done_copyout`, `copyout_fault`).
- **Lite2's** (`trap.c`, C `copyout`): the constant, USRSTACK, and the
  order of the checks on the page directory entry before the PTE. The
  PDE test is written afresh in assembly (`testb $1,PTD(,%eax,4)`).
- **New here**: the bound in `copyin` (NetBSD's `copyin` relies on the
  %gs limit and has none); using USRSTACK instead of NetBSD's
  VM_MAXUSER_ADDRESS; the test knob `FORCE_386_COPYOUT`.

The alternatives, measured or weighed, in source order:

| # | option | result |
|---|---|---|
| 1 | Lite2's own C `copyout` restored (trap.c) | closes `copyout` only: it has no counterpart for `copyin`, which needs its own fix anyway; reinstating it undoes patch 126's choice of the assembly routine and replaces the copy loop with page-at-a-time `bcopy`. Larger than adding a check. Not measured; judged. |
| 2 | **the existing assembly with Lite2's bound (USRSTACK) in NetBSD 1.0's form, plus a `copyin` bound** | smallest change that closes both holes. Taken. |
| 3 | NetBSD 1.0's bound as it stands (VM_MAXUSER_ADDRESS) | measured: every `execve` of init failed (error 13); the cause is read from the layout (`execve` builds the arguments and `ps_strings` in the page between 0xEFBFD000 and USRSTACK), not traced through the exec path. Rejected by test. |
| 4 | shorten the user segment as NetBSD 1.0 does | would also bound the %gs routines, but touches the signal trampoline and `ps_strings`; larger, and not needed to close this hole. Deferred (deferred.md). |
| 5 | FreeBSD 2.0.5's `support.s`, NetBSD 1.1, 1.2 | not measured: the first candidate that closes the gap with the least change is taken. |

Option 1 against option 2 is a judgement under the first-fit rule,
not a measurement: the rule says the earliest continuation that closes
the gap, and ties go to the smallest change. Lite2's C routine closes
half of it (no `copyin`); the existing assembly with the bound added
closes all of it with fewer lines changed. The maintainer can overrule.

One difference from Berkeley's C `copyout`: a zero-length copy to a
kernel address now returns 0 (NetBSD's early exit; nothing is copied),
where Berkeley's returned EFAULT.

## Faults the test found in my first attempt

- **The bound was VM_MAXUSER_ADDRESS, as NetBSD's.** On this port that
  is 0xEFBFD000, one page below USRSTACK (0xEFBFE000), and `execve`
  builds the arguments and `ps_strings` in that page. Every exec failed
  (`exec /sbin/init: error 13`; the link between that code and
  `copyout`'s EFAULT was not traced, the error came out of the exec
  path). Bounding at USRSTACK made it go away. **Not reproducible from
  the commit.**
- **The 386 path read the PTE of a page whose page table did not exist
  yet.** That read is itself a fault in the page-table window, which is
  not a user mapping. The path now looks at the page directory entry
  first and calls `trapwrite` when the table is missing. Reproducible:
  `build/test/copyio386.sh` with the `jz 4f` removed fails (below).
- **Two older faults in the 386 path** (the sweep's "two bugs": the walk
  continuing from `trapwrite`'s return value instead of the address;
  the jump to the handler with extra words on the stack) are gone with
  the rewrite. The locore.s comment calls the three together "faults".

## The tests

`build/test/copyio.sh` builds a 20 MB disk with the test as
`/sbin/init`, boots a kernel under QEMU (`-cpu 486`) and compares the
lines printed with the 22 expected ones. `build/test/copyio386.sh`
builds a kernel with `-DFORCE_386_COPYOUT`, which makes a 486 core run
the 386's page-by-page check, checks that the option reached the
Makefile, and runs the same test.

| case | before (patch 242; unreproduced) | after |
|---|---|---|
| A `read` of a file into a kernel address | 6, wrote it | EFAULT |
| B `write` of a kernel address to a pipe | 16, read it | EFAULT |
| G `write` from 8 bytes below USRSTACK, length 16 | 16 | EFAULT |
| D `read` into USRSTACK - 7, length 8 | 8, wrote the stack pages | EFAULT |
| M `write` ending exactly at USRSTACK | not run | 8 |
| N `write` ending 1 byte past USRSTACK | not run (by code: 8) | EFAULT |
| K `read` ending exactly at USRSTACK | 8 | 8 |
| C, C2 pipe and file copies, data compared | ok | ok |
| I `read` across two fresh anonymous pages | ok | ok |
| H `read` into a touched read-only page | EFAULT, unchanged | EFAULT, unchanged |
| L fork and execve with arguments and environment | intact | intact |
| E, F address arithmetic wraps | EFAULT | EFAULT |
| J length 0 with a kernel address | 0 | 0 |

**Cases that do not discriminate**, said plainly: E and F fail
neither on the old kernel nor with the carry test removed (a mutation
run: `jc` replaced by `nop` in both routines still PASSes), because
the copy then ends in a hardware fault at 0xFFFFFFF8. A wrapping range
starting on a mapped kernel page needs a length no `read`/`write`
passes down. The carry test is defence in depth, kept because
NetBSD's has it and a caller inside the kernel can pass such a length.
J probably never reaches `copyout` (the file layer can return 0
first; not traced). Cases H and I pass without the 386 walk on a 486
core, since the hardware refuses and faults in by itself; they show
that the walk does not break them, not that the walk is needed.

**Mutation runs** (this tree, 2026-10-30; each reverted; each
kernel built and run under `copyio.sh`):

| mutation | result |
|---|---|
| `ja` to `jae` in `copyin` alone (end exactly at USRSTACK refused) | FAIL: M returns EFAULT, 8 expected |
| `ja` to `jae` in `copyout` alone | FAIL: nothing printed (inferred: `execve` of init is refused, its stack ends at USRSTACK; K was not seen failing on its own) |
| both `ja` to `jae` | FAIL: nothing printed |
| `jc` removed in both | PASS (E, F do not discriminate, above) |
| `jz 4f` removed in the 386 path (forced build) | FAIL: `panic: init died (signal 6)` |
| the patch not applied (patch 242 kernel) | FAIL on A, B, G, D (earlier run; unreproduced now) |

**Install regression** (unreproduced like the other measurements). A kernel from this tree installed the
distribution from the CD (MFS install kernel, `Extract base`,
`Configure`), and the installed disk booted multi-user on the
patched GENERIC kernel (fsck clean, daemons started, no panic). A login
prompt was not reached on the shim's serial console; no commands were
run in the booted system.

## What is not established

- **A real 386.** QEMU has none; the forced run is the 386 code on a
  486 core, where the processor also enforces write protection. What
  is shown is that the path does not break cases H and I and that its
  page-directory test is needed (removing it fails); that it refuses a
  read-only page is not shown, since the 486 core refuses it anyway; that a 386 would otherwise write through is Berkeley's and
  NetBSD's statement, not measured here.
- **The `%gs` routines.** `fuword`, `suword`, `fubyte`, `subyte`,
  `copyinstr` and `copyoutstr` are bounded only by the user segment's
  limit, whose last byte is 0xEFC00FFF (deferred.md): they reach the
  two `struct user`/kernel-stack pages above USRSTACK and the first
  page of the page-table window. Not changed.
- **soreceive.** An error from `copyout` in a stream or pipe read makes
  `soreceive` consume the data, go round again and sleep for more; a
  read from a pipe into a bad address never returns until a signal.
  Not touched; deferred.md.
- **A huge length** (high bit set) cannot be sent from user level:
  `read` and `write` chunk it before `copyin`/`copyout`. The code
  handles it (the unsigned carry and compare), by reading, not by test.
