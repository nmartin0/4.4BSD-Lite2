<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# `ptrace`: NetBSD 1.0's `sys_process.c`, and the first fix of its `PT_WRITE_U`

Stage 4, item 1 of `plan-4.5.md` ("`ptrace` (#380)"); the A2 item for
`kern/sys_process.c` (`a2-files.md`). Written 2026-11-01 (patch 269).
`kern/sys_process.c` was FreeBSD 2.2's rev 1.24 (Sean Eric Fagan, 1994,
four-clause with the advertising clause). 4.5BSD's rule is the earliest
release that has the thing, and NetBSD 1.0's Lite-based
`sys_process.c` (Christopher Demetriou, 1994) comes before FreeBSD
2.2's. It is now the tree's file, imported and shrink-wrapped, not
written: the donor's notice and RCS tag are kept
(`$NetBSD: sys_process.c,v 1.33.2.4 1994/09/25 05:46:15 cgd Exp $`),
and each change of ours is labelled `AI-ONLY NOTE`
(`precedent.md`, "No original works").

## What was done, in order

1. **The donor file, cut to what this tree needs.** Taken whole;
   changed only to compile and work here:
   - `ptrace()` takes `<sys/syscallargs.h>`'s argument structure and
     `SCARG()`, as this tree's other system calls do, in place of
     1.0's own `struct ptrace_args` (the tree's header already defines
     one; `uap->req` would not compile);
   - NetBSD's machine-dependent `process_sstep` and `process_set_pc`,
     which this port has not got, are this tree's `procfs_sstep`
     (Lite2's own) and `ptrace_set_pc` (`i386/i386/machdep.c`, from
     FreeBSD 2.2, already in the tree);
   - `FIX_SSTEP` is used only where the port defines it, as
     `procfs_ctl.c` does;
   - five `#include` lines (`<sys/systm.h>`, `<sys/vnode.h>`,
     `<sys/mount.h>`, `<sys/syscallargs.h>`) and
     `<miscfs/procfs/procfs.h>`, for the above and for `procfs_domem`
     and its kin. `<sys/vnode.h>` before `procfs.h` is needed or the
     kernel gains 16 new "declared inside parameter list" warnings.
   Nothing else of 1.0's body is changed. `trace_req()` is 1.0's own.
   Not taken, because they are later changes and not the first fix of
   the hole: 1.1's `PHOLD`/`PRELE`, its merging of `PT_DETACH` into
   `PT_CONTINUE`, its `p_oppid` set at `PT_TRACE_ME` and `PT_ATTACH`;
   1.2's `securelevel` check on init; 1.3's `P_FSTRACE`; 1.4's
   `P_INEXEC` and `proc_isunder`; 1.5's `tmp` for `PT_READ_D`.
2. **The hole.** 1.0's `PT_WRITE_U` stores a word at any aligned offset
   up to `ctob(UPAGES) - 4` of the traced process's user area, and
   `PT_READ_U` reads from any. The user area holds the saved code
   segment and flags, the kernel stack and the pcb. A tracer could set
   its tracee's `%cs` to ring 0 or change `PSL_USERSTATIC` bits in
   its flags, and so run in the kernel. (The file this replaces
   confined the write to the register frame and the FPU save area
   with `ptrace_write_u`, refusing a code segment that was not user
   privilege and a flags change in `PSL_USERSTATIC`; that is FreeBSD's
   fix, 1994.)
3. **The first donor fix, found by comparing consecutive releases.**
   The six files (`sys_process.c` as each release
   branch has it, NetBSD 1.0 to 1.5) **were exported by the integrator**, with no git on
   the donors, and diffed pairwise:

   | from, to | what changed in `PT_READ_U`/`PT_WRITE_U` |
   |---|---|
   | 1.0 (rev 1.33.2.4, Sep 1994) to **1.1 (rev 1.52, 7 Oct 1995, mycroft)** | **both requests removed**: the two `case` labels in the permission switch and the two bodies are gone; they fall to `default:` and answer `EINVAL` |
   | 1.1 to 1.2 (rev 1.55, May 1996, tls) | none (init's `securelevel`, `<miscfs/procfs/procfs.h>`) |
   | 1.2 to 1.3 (rev 1.57, Apr 1997) | none (`P_FSTRACE`, `initproc`) |
   | 1.3 to 1.4, 1.4 to 1.5 | none (`P_INEXEC`, `proc_isunder`, UVM headers; `tmp`, `procfs_validregs(t, NULL)`) |

   The first fix of the unchecked `PT_WRITE_U` is therefore **NetBSD
   1.1's, rev 1.52: it is not made safe, it is removed**, together with
   `PT_READ_U`, and stays out through 1.5. Applied here as the first
   change to the 1.0 text, as 1.1 does it. The request numbers stay in
   `<sys/ptrace.h>`. The consequence for the rest of the tree:
   FreeBSD's second machine hook, `ptrace_write_u`, has no caller and
   is removed from `i386/i386/machdep.c` and `<sys/ptrace.h>`
   (`ptrace_set_pc` stays).
4. **Not checked.** OpenBSD's 1995 import (forked from NetBSD in
   October 1995, after 1.1) and FreeBSD's releases were not read for
   this: on the precedent order NetBSD 1.1 (October 1995) is no later
   than OpenBSD's fork, and FreeBSD 2.0's `ptrace_write_u` (1994) is
   FreeBSD's own, a different file lineage and not a fix in NetBSD's.
   Whether an earlier fix exists in a Lite-based tree not examined
   (Lites 1.1.u3, FreeBSD 2.0.5) is **not established**; the integrator
   can ask for it.

## Measurements

Warnings: `sh build/kernel.sh` before and after, sorted `warning|error`
lines compared: 266 and 266, identical (all pre-existing).
The kernel is 740073 bytes before, 739498 after.

`build/lineage.py` as `a2-files.md` ran it, against Lite2's stub and
386BSD 0.0 and 0.1's `sys_process.c` (the full body of 0.1, 0.0's
partial copy):

| file | lines | in Lite2's stub | in the Net/2 text, not Lite2's |
|---|---|---|---|
| **this tree's now** (NetBSD 1.0, shrink-wrapped, with our notes) | 221 | 37 (16%) | **8 (3%)** |
| the FreeBSD 2.2 file it replaces | 182 | 21 (11%) | 16 (9%) |
| NetBSD 1.0, whole | 205 | 38 (18%) | 10 (4%) |
| OpenBSD's 1995 import (for comparison, `a2-files.md`'s) | 185 | 37 (20%) | 8 (4%) |

The eight lines are `case PT_ATTACH:`, `PT_CONTINUE`, `PT_DETACH`,
`PT_GETREGS`, `PT_SETREGS`, `PT_WRITE_D`, `PT_WRITE_I` and
`#ifdef PT_GETREGS`: request names from Lite2's own `<sys/ptrace.h>`,
the interface, as in `a2-files.md`. (Our note adds lines; the 221 includes
them.) Clear on this test, for what it can see; it is a lower bound.
The notice still carries the Regents' paragraph about material
"licensed to the University of California by ... Unix System
Laboratories", as NetBSD's 1.0 does for a Lite-based file; that is the
donor's notice kept as it is, and whether that wording needs the
settlement's list is for the clearance ledger, not decided here.

## The test, and what it showed

`build/test/ptrace.c` and `ptrace.want`, run with
`sh build/test/ktest.sh ptrace`. As `/sbin/init` it forks tracees and
does `PT_TRACE_ME`, `PT_ATTACH`, `PT_READ_D`, `PT_WRITE_D`,
`PT_CONTINUE`; the refusals for a process not traced, an attach to
self and a second attach; and `PT_WRITE_U` at eleven offsets (0, 4,
0x800, 0xfc0, 0x1000, 0x2ffc, 0x3000, 0x10000, 0x7ffffffc, 0xfffffffc
and the unaligned 3), and again on an attached process, and
`PT_READ_U` at 0. The want file is the new behaviour: every one of the
`PT_WRITE_U` offsets and `PT_READ_U` answers `EINVAL`, and a
`PT_WRITE_D` still reaches the tracee (its exit status shows it).

- **Old kernel** (FreeBSD 2.2's file; the same tree before this patch):
  FAIL. Everything but `PT_WRITE_U`/`PT_READ_U` is as wanted; each
  `PT_WRITE_U` is refused with `EFAULT` (14), not `EINVAL`, and
  `PT_READ_U` at 0 **succeeds** (errno 0). The old tree already
  refuses the write; what the test shows failing is the request's
  answer.
- **New kernel:** `ptrace: PASS (32 lines as expected)`.
- `sh build/test/ktest.sh execve` on the new kernel: `execve: PASS (63
  lines as expected)`; nothing else moved.

## What was not tested

- **`PT_KILL` and `PT_DETACH`.** A first version of the test used
  `PT_KILL` on a child stopped by `kill(getpid(), SIGSTOP)`: the call
  returned 0 but the child stayed stopped (`wait` status `0x117f`)
  and never died, on the old kernel and the new. Not looked into (it
  is kernel signal code, `issignal`, not this file); the test leaves
  `PT_KILL` out. 1.0's `PT_DETACH` finds the original parent with
  `pfind(p_oppid)`, and `p_oppid` is not set by `PT_TRACE_ME` in 1.0
  (1.1 sets it); `PT_DETACH` of a `PT_TRACE_ME` child is untested and
  may reparent wrongly. Fixing that is 1.1's second change and was
  not part of this patch.
- **`PT_STEP`, `PT_GETREGS`/`PT_SETREGS`, `PT_GETFPREGS`/`PT_SETFPREGS`**
  are not exercised.
- **`gdb`.** `contrib/gdb-4.16`'s generic `infptrace.c` and the
  `gdbserver` `low-fbsd.c` use `PT_READ_U` and `PT_WRITE_U` for the
  registers; with them gone they will get `EINVAL`. i386b-nat.c uses
  `PT_GETREGS`. None was built or run here; whether the tree's `gdb`
  works is unknown. `lib/libc/sys/ptrace.2` (FreeBSD 2.2's) still
  documents `PT_READ_U`/`PT_WRITE_U`: not edited.
- **Only i386, only the GENERIC kernel under QEMU (486).** The other
  ports' `procfs_sstep`/`ptrace_set_pc` are not built here.
- OpenBSD's and FreeBSD's files for an earlier fix (item 4 above).

## Revised by the integrator (2026-11-01)

**Read this section and those after it as superseding the sections
above** where they differ: the removal of `PT_READ_U`/`PT_WRITE_U` and of
`ptrace_write_u` described above is not what was committed, and the
test described above (eleven offsets, 32 lines) was replaced (34 lines).

**NetBSD 1.1's removal of `PT_READ_U` and `PT_WRITE_U` is not taken.**
This tree's `gdb` is FreeBSD 2.2.8's configuration
(`gnu/usr.bin/gdb/gdb/nm.h`), which does not define
`FETCH_INFERIOR_REGISTERS`. So `infptrace.c` reads every register with
`PT_READ_U` and writes them with `PT_WRITE_U`, and continuing from a
breakpoint writes the PC back that way. With both requests gone, `gdb`
could show no registers and could not continue past a breakpoint.
`i386b-nat.c`'s `PT_GETREGS` path is not a drop-in replacement: it
copies `struct reg` straight into gdb's register array, and this tree's
`struct reg` (cgd's 1993 order, `es, ds, edi, ...`) is not gdb's order.

Instead:

- **`PT_READ_U` and `PT_WRITE_U` are NetBSD 1.0's own**, range and
  alignment test included, with `SCARG` for `uap->`.
- **`PT_WRITE_U`'s hole** is closed by FreeBSD 2.1.0's and 2.2's
  `ptrace_write_u` (`i386/i386/machdep.c`), which this tree already had
  and which is restored: a write only into the register frame (a code
  segment of user privilege, the `PSL_USERSTATIC` flag bits unchanged)
  or the FPU save area, and `EFAULT` elsewhere. That is a fix from
  another line, FreeBSD's, where NetBSD's own line's fix would have
  broken a shipped program. It is the departure from the first-fit
  rule in this patch, and it is recorded as one.
- **1.1's parent-on-detach change is not taken.** A first revision
  took it, saying that 1.0's `PT_DETACH` after `PT_TRACE_ME`
  reparented the child to `pfind(0)`, the swapper. The stage's
  reviewer showed that is false: process 0 is never put in the PID hash
  (this tree's `init_main.c` and NetBSD 1.0's insert it in `allproc`
  only), so `pfind(0)` returns NULL and 1.0's `if (pp)` reparents
  nothing; the child stays with its tracer, which after `PT_TRACE_ME`
  is its parent. What 1.1's change really adds is the fallback to
  `initproc` when the original parent has gone, a hole not shown here.
  Under the rule that a later fix is taken only for a hole shown, the
  three lines were dropped.

**The test was revised to match** (`build/test/ptrace.c`, 34 lines):
`PT_READ_U` reads `md_regs` and a register; `PT_WRITE_U` may set `esi`
and reads back the same; it refuses a kernel code segment and IOPL 3
in the flags with `EINVAL`, offsets outside the frame and FPU area with
`EFAULT`, and offsets past the user area or unaligned with `EINVAL`;
`PT_DETACH` after `PT_TRACE_ME` leaves the child with its own parent
(a check of the detach path, not of a fix: 1.0 gives the same answer).
It passes on the integrated kernel. `gdb` itself was run on that kernel
(scratch harness): see below.

## Ledger row and imports.md entry

clearance.md, replacing the FreeBSD 2.2 `kern/sys_process.c` row:

| NetBSD | 1.0, rev 1.33.2.4 (Demetriou 1994), shrink-wrapped; `PT_WRITE_U` confined by FreeBSD's `ptrace_write_u` | `kern/sys_process.c` | **measured, 2026-11-01** (`ptrace-firstfit.md`): 9 of 253 lines (3%) in the Net/2 text and not Lite2's stub (re-measured by the integrator with the U requests back), all `case PT_...:` labels and one `#ifdef PT_GETREGS`. Notice carries the Regents/USL paragraph as the donor's | Demetriou 1994 and the Regents, four-clause, advertising clause | #380 |

imports.md: `kern/sys_process.c  NetBSD 1.0, shrink-wrapped;
PT_WRITE_U through FreeBSD's ptrace_write_u`. (Line counts: the table
above, 221, is the worker's version; 253 is the committed file's, both
lineage.py's distinct lines; `wc -l` gives 466.)

## gdb, run (2026-11-01)

Scratch harness (not in the tree): the integrated kernel booted
single-user under QEMU on the release root, a three-line program
compiled in the guest with `cc -g`, and `gdb -batch` given `break main`,
`run`, `info registers`, `set $esi = 0x1234`, `p/x $esi`, `continue`.

- **Integrated kernel:** the breakpoint is hit; the registers are
  printed and sane (`cs 0x1f`, `ss 0x27`, `eip` at the breakpoint);
  `$1 = 0x1234`; "Program exited with code 06." So `PT_READ_U`,
  `PT_WRITE_U` of an ordinary register, and the PC written back after
  the breakpoint all work.
- **The worker's first version, with 1.1's removal** (same image, same
  commands): `reading register eip (#8): Invalid argument.` and gdb
  stops there. That is the failure this revision avoids, and it shows
  the check can fail.

Not run: `gdbserver`, `info float` (it reads the FPU area through
`PT_READ_U`, and `struct fpreg` is not filled, `deferred.md`), kernel
debugging with `gdb -k`.
