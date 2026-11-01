# `execve`: the quoted `getxfile` lines, and NetBSD's exec framework in its place

Stage 3, item 2; audit station A2 (`roadmap-audit.md`): commits #204
and #206 ("execve, written: the kernel execs" and "execve: the stack
goes a page below USRSTACK, as Berkeley's i386 does"), measured by an
earlier reviewer at 53% line-match to 4.4BSD's `kern_exec.c`.
`plan-4.5.md`'s own instruction for this item: get the `getxfile`
lines quoted from 4.4 out, then find the first canonical fit for
`execve` itself -- 386BSD 0.1's own, if clear and if it closes the
ELF gap; otherwise NetBSD 1.0's exec framework, with 1.1 for ELF.

Two patches. Patch 261 took the quotation out and measured the
candidates; the next patch (262) shrink-wrapped NetBSD's exec framework
into the tree in place of the body #204 wrote. Patch 261's sections are
kept below as written, with four corrections to them recorded after
them rather than made inside them.

## Patch 261: what was quoted, and why it had to come out

`usr/src/sys/kern/kern_exec.c`'s stack-allocation comment did not
just describe 4.4BSD's `getxfile()` -- it reproduced it, as a C
fragment inside the comment:

```
size = round_page(MAXSSIZ);
#ifdef	i386
addr = trunc_page(USRSTACK - size) - NBPG;	/* XXX */
#else
addr = trunc_page(USRSTACK - size);
#endif
```

That is Net/2-era Berkeley text, the exact kind rule 2(b) and rule 3
exist to keep out, sitting in a comment rather than in the executable
code below it -- which does not make it any less a direct quotation
of the encumbered file. `precedent.md`'s XNU-reading allowance (read,
never copy, and never into a comment either) does not extend to
4.4BSD's own encumbered sources; nothing in this tree's rules permits
quoting them at all, in code or in prose. Removed in this patch,
replaced with the same technical reasoning (the `- NBPG` and the
`vm_map_protect` below it are both still needed, and still work the
same way) restated in this tree's own words, with no line of
Berkeley's text surviving in it.

The commit that introduced it (#204) said its "shape is 4.4BSD's,
read from the encumbered kern_exec.c as `precedent.md` allows XNU to
be read" -- an analogy that does not hold. XNU is a separately
licensed, separately maintained tree with its own clearance question;
4.4BSD's `kern_exec.c` is the Net/2-era text itself. Whether the
*shape* of this tree's `execve` (not just this one quoted fragment)
needs to be rederived from a clear source is the rest of this item,
below.

Checked for other quotations in the same two files
(`usr/src/sys/kern/kern_exec.c`, `usr/src/sys/kern/exec_elf.c`): none
found. This was the only one.

## Patch 261: measuring 386BSD 0.1's own `execve`

`/home/claude/386bsd/386bsd`, tag `r01` (386BSD 0.1),
`usr/src/sys.386bsd/kern/kern_execve.c` (402 lines). Licence: Jolitz
and TeleMuse's own notice (permissive, four-clause with an
advertising-style clause and a research/educational preamble) --
condition (a) is met. The file reads as an independent, deliberately
simplified implementation ("a minimal program execution facility...
Significant limitations and lack of compatibility with POSIX are
present with this version, to make its basic operation more clear"),
not a derivation of Berkeley's own `getxfile`/`exec_check_permissions`
machinery -- condition (b) is met on the reading that this is Jolitz's
own, much simpler design, not Net/2 text. Condition (c) (not carried
forward unchanged by a later BSD) also holds: this exec implementation
is one of the areas 386BSD was weakest in, and NetBSD, FreeBSD and
OpenBSD all wrote their own, more complete exec frameworks rather than
keeping Jolitz's.

It does not close this tree's gap. `kern_execve.c` reads only `a.out`
`ZMAGIC` headers (`hdr.a_magic != ZMAGIC` is the format check; there is
no second format, no probe table). This tree's binaries are ELF --
the existing commit's own reasoning for why ELF was chosen still
holds (GNU ld here has no i386 `a.out` emulation) -- so a file that
cannot read anything but `a.out` cannot be the fit regardless of its
clearance. It also has no `#!` interpreter handling and no
`ps_strings` support, though it does set the saved uid/gid on `VSUID`/
`VSGID` (`p_svuid`, `p->p_ucred->cr_uid = attr.va_uid`), so that part
of the plan's own test list is not unique to a later donor.

Per `plan-4.5.md`'s own stated rule for this item, 386BSD 0.1 being
clear but not closing the ELF gap means the fit moves to the next
candidate in source order: NetBSD's exec framework.

## Patch 261: NetBSD 1.0 (the framework) and 1.1 (ELF)

`/home/claude/donors/netbsd`, branch `origin/netbsd-1-0`: a real exec
framework, not a single ad hoc function -- `sys/kern/kern_exec.c`
(the generic `sys_execve` driver and permission/format-probe logic),
`sys/kern/exec_subr.c` (the vmcmd list that defers the actual image
building until the file is known loadable), `sys/kern/exec_conf.c`
(the table of registered formats to probe in order), and
`sys/kern/exec_aout.c` (the `a.out` probe/loader, the only format
NetBSD 1.0 itself has). Branch `origin/netbsd-1-1` adds
`sys/kern/exec_elf.c` and `sys/sys/exec_elf.h` -- ELF support, as
`plan-4.5.md` names it.

This is the shape 4.5BSD's `execve` should have: a generic driver that
probes a small, explicit table of formats (here, just ELF, since this
tree carries no `a.out` support to probe for) and defers to a vmcmd
list, rather than the current single function that does ELF-specific
and generic work in one body, inlining the sequence an earlier
commit read out of the actual encumbered 4.4BSD file. 386BSD 0.0/0.1
is checked and ruled out above, not skipped; NetBSD's framework is the
first candidate after it that actually closes the gap, matching the
source order `plan-4.5.md` sets for this item.

## What patch 261 did, and did not do

This patch removes the quoted `getxfile` text (the concrete, in-tree
exposure) and records the measurement and the fit above. It does
**not** yet rewrite `kern_exec.c` to NetBSD's vmcmd-list shape --
that is a substantial, boot-critical kernel change (every process
start goes through this code, including `init`'s own first exec) and
is tracked as the remaining work on this item, not silently deferred.
Until that rewrite lands, this tree's `execve` is still structured as
the earlier commit left it; what has changed is that the one piece of
verbatim encumbered text in it is gone, and the path forward is
measured and recorded rather than assumed.

## What patch 261 did not establish

- The rewrite itself (`kern_exec.c`, `exec_subr.c`-equivalent,
  `exec_conf.c`-equivalent reshaped to NetBSD's framework, adapted to
  this tree's own, older VM primitives rather than NetBSD's) is not
  done. Until it is, the 53% line-match A2 flagged is not resolved,
  only the one literal quotation within it.
- Whether the *rest* of the current `kern_exec.c`'s structure (beyond
  the one quoted fragment) is close enough to 4.4BSD's own text to
  itself be a clearance problem, independent of the quotation, is a
  question for the rewrite to answer by replacing the structure
  outright -- not measured line-by-line here the way `disk-wd.md`
  measured `wd.c`, because the fix decided on (a full NetBSD-shaped
  rewrite) makes that measurement moot rather than something to
  settle on the current text first.
- `kern_acct.c`, `subr_rmap.c`, `kern_physio.c`, and `sys_process.c`
  (`plan-4.5.md` item 3, the other A2 files) are not covered by this
  item and remain open.

## Corrections to patch 261

Made 2026-10-31, with patch 262; patch 261's sections above are left
as they were written.

**1. Rule 2(c), and a false fact under it.** Patch 261 said condition
(c) holds because "NetBSD, FreeBSD and OpenBSD all wrote their own ...
exec frameworks rather than keeping Jolitz's." The condition -- "not a
file that FreeBSD, NetBSD or OpenBSD carried forward from those files
unchanged" -- reads two ways: "those files" may be (b)'s Net/2- and
4.4BSD-derived text, or the 386BSD files the rule is about. (This
record first called patch 261's reading a misreading; independent
review pointed out the wording allows it, and that the other reading
leaves (c) saying little that (b) does not. It is ambiguous.) Either
way Jolitz's file passes. What was wrong was the fact: FreeBSD 1.0 did
carry it forward -- changed, not unchanged. Measured
with `build/lineage.py`'s line test (distinct lines of twelve letters
or digits or more), the share of each file's lines also found in
386BSD 0.1's `kern_execve.c`:

| file | lines | in 386BSD 0.1 |
|---|---|---|
| FreeBSD 1.0 `kern_execve.c` (Jolitz's notice, Greenman's `$Id` 1.8) | 267 | 69% |
| FreeBSD 1.1.5.1 `kern_execve.c` (Greenman's notice, 1993) | 271 | 20% |
| FreeBSD 2.0 `kern_exec.c` | 289 | 10% |
| NetBSD 1.0 `kern_exec.c` (Demetriou, Solfrank) | 279 | 5% |

Jolitz's file is still clear, so the conclusion stands; the reason
given for it did not. The
independent review of patch 261 did not catch this.

**2. The plan's wording.** `plan-4.5.md` says 386BSD 0.1's execve "if
... it closes the gap with the ELF loader this tree uses." Patch 261
read that as "if it loads ELF itself." It can as fairly be read as
"386BSD's execve together with the ELF loader already here." Read that
way it still does not close the gap -- its own header says
"Significant limitations and lack of compatibility with POSIX", and it
has no `#!` and no `ps_strings` -- and closing it would mean splicing
four sources (Jolitz's driver; Greenman's `#!` from FreeBSD 1.0, the
same file; NetBSD 1.1's ELF loader; `ps_strings` from FreeBSD 2.0 or
this tree). Both readings were put to the maintainer, who chose
NetBSD's framework (2026-10-31; see *The options*, below).

**3. Where the map ends.** The comment patch 261 wrote above the old
body's stack allocation said VM_MAXUSER_ADDRESS is "where the
vmspace's map actually ends," so that a stack running up to USRSTACK
"reaches one page past the map's own end." That is not so for any
ordinary process: vmspace0's map is made to VM_MAX_ADDRESS
(UPT_MAX_ADDRESS, 0xEFFF7000; `kern/init_main.c`), `vmspace_fork`
copies a map's bounds, and `vm_fork` marks only USRSTACK and above
uninheritable. What the `- NBPG` stepped around was an entry, not the
map's end: `start_init` allocates init's argument page at
`trunc_page(USRSTACK - PAGE_SIZE)`, 0xEFBFD000; the old body removed
only up to VM_MAXUSER_ADDRESS, 0xEFBFD000, so that page survived every
exec and every process inherited it -- and the old body wrote
`ps_strings` and the argument strings into it, PS_STRINGS being at
its top. The comment went with the body (patch 262).

**4. What precedent.md allows.** Patch 261 said `precedent.md`'s
XNU-reading allowance "does not extend to 4.4BSD's own encumbered
sources," and called #204's appeal to it "an analogy that does not
hold." Wrong: `precedent.md`'s *The CSRG history is read-only, like
XNU* puts the CSRG history, encumbered 4.4BSD and all, on the same
read-only terms, and `missing.md` says the same. What #204 broke was
the rest of that section: nothing is copied from it -- a quotation is
a copy -- and, since the breach it records, "This project no longer
writes function bodies at all. Where a body is missing, the answer is
an import ... and shrink-wrap it." Patch 262 is that rule applied. (The
maintainer's standing instruction for this work is stricter than
`precedent.md`: the CSRG tree is not read at all, and nothing for
either patch was.)

## The options (2026-10-31)

Asked to size every option before the rewrite, each checked in the
donor trees:

| option | what it is | verdict |
|---|---|---|
| OSFMK (`osfmk-mklinux.git`) | the microkernel has no Unix exec; the one `exec.c` is MkLinux's Linux server, GPL | not viable |
| Mach 3, xMach's kernel | bootstrap loaders that start the first server task, not `execve` | not viable |
| xMach's Lites | `server/kern/kern_exec.c` is Lite's stub, USL notice and all; the real exec is Helander's (1994, ELF-aware, about 3,100 lines in `server_exec.c`, `emul_exec.c`, `exec_file.c`), built on Mach task ports with half of it in a user-space emulator | read, not taken |
| 386BSD 0.1, spliced | Jolitz's driver plus three other sources (correction 2) | possible, four sources |
| FreeBSD's line | 386BSD's file carried on: 69% Jolitz's in 1.0, 10% by 2.0; ELF only from 2.2 (1997), on a VM that had moved away from Lite2's | weaker fit |
| NetBSD 1.0/1.1, shrink-wrapped | one framework, one design; ELF in 1.1 by Christos Zoulas, whose functions this tree already had; pre-UVM, the same VM as Lite2 | chosen |
| OpenBSD | forked from NetBSD 1.1: the same framework, later | the same code |

NetBSD's also keeps the two releases from doing the work twice:
OpenBSD, first in 4.6BSD's donor order, carries the same framework,
so 4.6BSD inherits it as a continuation.

## Patch 262: what came in

`precedent.md`'s rule for a missing body, applied: an import, shrink-
wrapped, cut to this tree's interfaces. Every file is the donor's text
with the changes listed in its own AI-ONLY NOTE; each was diffed
against the donor file after it was written and the diffs hold only
those changes.

| file | from | changes |
|---|---|---|
| `kern/kern_exec.c` (589 lines) | NetBSD 1.0, rev 1.55 | NetBSD 1.1's three fixes to these lines (rev 1.72): the first interesting error kept; strings aligned with or without an environment; `ps_strings` pointing at the vectors (Hannum, rev 1.69). Fitted: SCARG, as Lite2's `sys/syscallargs.h` declares the arguments; `VOP_UNLOCK(vp, 0, p)`; Lite2's `setregs` and `cpu_setstack`; the old image removed to USRSTACK. Not taken: 1.1's emulation switch. |
| `kern/exec_subr.c` (246) | NetBSD 1.0, rev 1.8 | `vmcmd_map_pagedvn` out (a.out only); `exec_aout_setup_stack` in, alone, from NetBSD 1.0's `exec_aout.c` (rev 1.12.2.1, same author, same notice), with the `<sys/resourcevar.h>` it needs |
| `kern/exec_conf.c` (66) | NetBSD 1.1, rev 1.14 | cut to `#!` and ELF; EXEC_ELF defined outright |
| `kern/exec_script.c` (313) | NetBSD 1.0, rev 1.8.2.1 | `VOP_UNLOCK(scriptvp, 0, p)` |
| `kern/exec_elf.c` (436) | NetBSD 1.1, rev 1.3, restored | out: the emulation probes, `elf_copyargs`, `elf_load_file`, the interpreter (PT_INTERP refused), the VTEXT line; in: NetBSD exec_elf32.c rev 1.61's bound on `e_phnum`; kept: this tree's earlier fixes (machine types, ELF_ROUND/ELF_TRUNC, the short read) |
| `sys/exec.h` | NetBSD 1.0, rev 1.43.2.2 | the framework, inside `#ifdef KERNEL`, without `execve_args`, `ep_setup`, `ep_emul`, `vmcmd_map_pagedvn`; with its two Copyright lines (Demetriou 1994, de Raadt 1993) |
| `sys/exec_script.h` | NetBSD 1.0, rev 1.5 | none |
| `sys/exec_elf.h` | NetBSD 1.1, rev 1.1 | `exec_elf_makecmds`'s prototype restored, with forward declarations for `ddb/db_elf.c` |
| `sys/malloc.h` | NetBSD 1.0 | `M_EXEC`, at NetBSD 1.0's number, 63 |
| `sys/param.h` | NetBSD 1.0 | `MAXINTERP` 64, where Lite2 has 32: groff's `afmtodit`, which this tree ships, has a 35-byte `#!` line |
| `kern/kern_resource.c` | NetBSD 1.3 (rev 1.42 as shipped) | one line: RLIMIT_STACK's `rlim_cur` rounded to a page when it is set, NetBSD's own fix for the overlap NetBSD 1.0's stack setup makes with a limit that is not a whole number of pages; absent in 1.0-1.2, the exact commit not recoverable from the mirrors here. Found by independent review |

**Why the old image is removed to USRSTACK.** NetBSD removes up to
VM_MAXUSER_ADDRESS, which on NetBSD's i386 is USRSTACK. On Lite2's it
is the page below, and that page is stack (correction 3). Removing to
VM_MAXUSER_ADDRESS would leave `start_init`'s page in place, and
NetBSD's stack -- mapped up to USRSTACK, as trap.c, vm_unix.c and the
core dump all assume -- would not fit beside it.

**Why VTEXT is not set.** NetBSD 1.1's loader marks the vnode VTEXT
though it never maps it (its own comment: "no demand paging"). Only
`vnode_pager_dealloc` clears VTEXT, so the flag would stay on every
program ever run, and here that is not harmless: `nfs_doio` kills any
process reading a VTEXT file over NFS that has changed ("Process
killed due to text file modification"), and the loader's own
DIAGNOSTIC check would panic on an exec of a program that has run
before and is open for writing at the time. (It would last only while
the vnode stays cached: `getnewvnode` clears `v_flag` when it recycles
one.) Nothing set VTEXT before this patch either.

**What is left of #204's body.** Counted as `build/lineage.py`
counts -- distinct lines of twelve letters or digits or more,
whitespace folded -- after dropping lines that begin a comment (`/*`)
or continue one (`*`): of the new `kern_exec.c`'s 192 such lines, 39
are also in the body it replaces -- includes
and one-line idioms (`psignal(p, SIGTRAP);`, `p->p_acflag &= ~AFORK;`)
that NetBSD's text has in its own right. Against 386BSD 0.0's
`kern_execve.c`, the Net/2-era copy we may read, the five kernel files
share 2 lines between them, both generic.

## Behaviour that changes

- A `#!` script's interpreter gets the interpreter's name as the line
  spells it (`/sbin/init`) as its zeroth argument; the old body gave
  the last component (`init`). Lite2's `execve(2)` says the zeroth
  argument "is left unchanged"; NetBSD 1.0's and FreeBSD 2.0's pages
  say the same while their kernels replace it (`exec_script.c`'s
  note). Recorded, not acted on.
- `#!` lines up to 64 bytes run (MAXINTERP); the old body read 52.
- A program open for writing is refused with ETXTBSY, as Lite2's
  `execve(2)` documents; the old body did not check.
- `/proc/N/file` works: execve now sets `p_textvp`, which Lite2's
  fork, exit and procfs already maintain and the old body never set.
- P_SUGID is cleared at each exec and set when set-id applies; the old
  body set it and never cleared it.
- A traced process loses set-id, as in NetBSD 1.0-1.5 and FreeBSD
  2.0-2.2.8. The old body instead refused to exec a file the traced
  process could not read, and honoured set-id. Neither NetBSD nor
  FreeBSD of the period has that read check, so there is no donor fix
  to take: a traced process may now exec an execute-only program it
  cannot read. Flagged for the maintainer.
- A null `argv` gives EINVAL. (The old body ran the program with argc
  0 and its environment intact, and gave EFAULT only when the
  environment was null as well.) An argv that is empty -- a pointer to
  a null -- with an empty environment now runs the program with argc
  0, where the old body gave EFAULT.
- P_EXEC is set. `setpgid` on a child that has exec'd then fails with
  EACCES (kern_prot.c), as POSIX has it; the old body never set the
  flag, so the call succeeded. This tree's sh and csh ignore the error.
- With KTRACE (not in GENERIC), a set-id exec turns off a trace that
  root did not set.
- The loader refuses more, and differently. Refused with ENOEXEC: a
  PT_INTERP segment (no dynamic linker here); more than one text or
  data segment; a PT_LOAD whose protection is neither r-x nor rw-/rwx;
  more than 128 program headers (NetBSD exec_elf32.c rev 1.61). Refused
  with ENOMEM by check_exec, because the segment it lacks is left at
  ELF32_NO_ADDR: no text segment or no data segment (a single rwx
  PT_LOAD, as `ld -N' makes, is one), and no program headers at all
  (the old body gave ENOEXEC). New limits: ENOMEM when text exceeds
  MAXTSIZ, data the RLIMIT_DATA limit, or the arguments the stack
  limit; ENOEXEC when the entry point is above VM_MAXUSER_ADDRESS. Not
  refused any more: a program-header entry size other than 32 -- the
  old body checked it; NetBSD has never checked it, through the
  current tree, so there is no fix on this line to take, and such a
  file only has its own headers misread, most likely into an ENOEXEC.
  Every one of the 515 ELF executables in the built root (473 files,
  472 distinct, one of them /vmunix) has exactly two PT_LOADs, r-x and
  rw-, 32-byte entries and no PT_INTERP, so none of this refuses
  anything this tree builds.
- A program the loader refuses is refused before the old image goes.
  Given a file claiming 200 program headers, the old body read 6,400
  bytes of whatever followed as headers, destroyed the old image, and
  killed the process with SIGABRT; given one whose first header is
  PT_INTERP, it ran an image without its text and the process died of
  SIGSEGV. Both now fail the exec with ENOEXEC, the caller intact.
- A stack limit that is not a whole number of pages is rounded up when
  it is set (NetBSD 1.3's kern_resource.c). Without it NetBSD 1.0's
  stack setup killed the next exec -- found by independent review, see
  *Testing* -- and with it `getrlimit' reports the rounded value.
- The page below USRSTACK is ordinary stack, made fresh at each exec.
- The old body gave a process whose vmspace was shared (`vm_refcnt >
  1`) a new one; NetBSD removes the old mappings in place. In this
  tree `vm_fork` always copies (`vmspace_fork`), vfork included.

## Testing

**`build/test/execve.c`**, run by `build/test/ktest.sh` as `/sbin/init`
under QEMU, with `execve.want`. As init it remounts the root writable,
makes its fixtures from its own image -- copies; a set-user-ID,
set-group-ID copy owned by uid 5 and gid 6; an execute-only one; nine
`#!` scripts; a file of text; a file without execute permission; a copy
claiming 200 program headers and one whose first header is PT_INTERP
-- mounts procfs, and runs each case in a child that sends back exec's
errno; a program that runs prints its own line. Sixty-three lines: an
ELF program with its arguments and environment; `ps_strings` (count and
vectors, read at PS_STRINGS); 300 KB of stack, and a stack past its 512
KB limit killed with SIGSEGV; a stack limit of 1025K; `#!` scripts with
and without an argument, at 60, 64, 65 and 70 bytes, run by a script,
with a missing interpreter; ENOEXEC, EACCES and ENOENT; a null argv;
an empty argv and environment; set-id program and script run as uid 7,
and the saved IDs taken back; the same program traced, and an
execute-only one traced; the e_phnum and PT_INTERP refusals; 16,000
bytes of arguments, and E2BIG past ARG_MAX; close-on-exec; signals
caught, ignored and blocked, and a signal caught in the new image;
procfs's `file`; setpgid on a child that has exec'd; ETXTBSY; opening
for writing a program that has run; six programs in one process; vfork.
Every line was read against what it should be before it went into
`execve.want`.

On this kernel all sixty-three lines are as `execve.want` has them.
Against two other kernels:

- **This patch less NetBSD 1.3's round_page in kern_resource.c** fails
  on one line, the one added for it: `T stack limit 1025K: exec
  errno=0, killed by signal 6`. Independent review found the fault
  (see below); the test now shows it, and shows the fix curing it.
- **The kernel before this patch** (patch 261, built in a separate
  tree) differs on exactly the changes *Behaviour that changes* lists,
  and on no other line: the script's zeroth argument (`init`, two
  lines); the 60- and 64-byte `#!` lines (ENOEXEC); the null argv (it
  ran the program) and the empty argv and environment (EFAULT); the
  traced set-id program (euid 5, set-id honoured) and the traced
  execute-only one (EACCES); the 200-header file (exec'd, then killed
  with SIGABRT) and the PT_INTERP one (ran, then SIGSEGV); setpgid
  (succeeded); procfs's `file` (ENOENT); the program open for writing
  (ran).

**`build/test/execsh.c`**, through `build/test/execsh.sh`, which puts
this tree's own `/bin/sh` and `ls` on the disk (a new, optional
`KTEST_FILES` in `ktest.sh`): the real shell runs a `#!/bin/sh` script
that runs `ls` by command substitution and as a command and ends by
exec'ing it, and `sh -c` finds `ls` on PATH. Eight lines, the same on
this kernel and the one before; it is there to catch a regression in
real programs. (Its `ls of a missing file: status 0` is Lite2's ls,
which exits 0 there, on both kernels.)

**Also:** the kernel builds with no warning from any of the new or
changed files beyond the tree-wide `vref` one every kernel file prints;
all 515 ELF executables in the built root have one text and one data
segment and no PT_INTERP (counted by a script reading the program
headers; an earlier count of 384 covered only bin, sbin, usr/bin,
usr/sbin and usr/libexec), so the loader's limits refuse none of them;
`build/test/exec_getxfile_quote.sh` (patch 261's test, its last check
changed because the old stack arithmetic is gone) passes on this tree
and fails on the old one; and the other kernel tests that exec --
`elfshort`, `copyio`, `bufwrite` -- pass on this kernel.

**A multi-user system.** See *The commands, on a running system*,
below.

**Independent review** (fresh context, told nothing of the author's
conclusions; it did not read the CSRG tree) diffed every file against
its stated donor revision and found the differences to be the declared
ones but for small undeclared items, since declared in the files'
notes; confirmed the USRSTACK fit step by step; confirmed the VTEXT
decision; re-derived the 1.0-to-1.1 classification and the overlap
table; ran both tests on both kernels. It found:

- **a regression, now fixed**: NetBSD 1.0's stack setup kills the exec
  of any process whose stack limit is not a whole number of pages
  (`limit stacksize 1025k` in csh, then any command). NetBSD's own fix
  is in 1.3's kern_resource.c and is taken (*Behaviour that changes*);
- behaviour changes this record did not list -- P_EXEC and setpgid, the
  empty argv, ktrace on a set-id exec, the loader's other refusals and
  limits -- now listed;
- that the "384" did not reproduce (now counted again, and the method
  stated); that the overlap counts depend on how comments are dropped
  (method now stated; its own count, dropping comments wholly, is 186
  and 41, and 334 code lines in the four `.c` files with 3 and 5 in
  386BSD 0.0 and 0.1 -- the conclusion the same); that
  `kern/init_main.c`'s comment on start_init's page gave the wrong
  reason, contradicted by correction 3 (corrected there: the old page
  overlapped vm_fork's u-area and page-table entry); that correction
  1 overstated patch 261's error (reworded); that the VTEXT note's
  panic case was put loosely (put exactly);
- what was untested, most of which the test now covers.

## The commands, on a running system

Asked by the maintainer, at the end of the work, to run the commands
and the flags of each that rest on what the kernel now does. Run on
2026-11-01 under QEMU (486, 32 MB), the same battery on two disks
that differ only in the kernel: this patch's, and the one before it
(patch 261, built in a separate tree). Each is also `/vmunix` on its
disk, since ps, w and vmstat read the kernel's symbols from there.

**The disk.** The release's base set (`base.tar`, built 27 October),
unpacked; this tree's gdb added, since that base set predates it;
vmstat rebuilt against this tree's headers, to name M_EXEC; an fstab
(`/dev/wd0a` root, `/dev/wd0b` swap) and `/etc/myname`; a `/proc`
directory; and in `/dev` the nodes init and the root mount need before
`MAKEDEV` runs (console, null, tty, mem, kmem, wd0a-c, rwd0a and
rwd0c, made with mknod). Without them the system stops silently
after `root on wd0`, init having no console; found on the way, since
the first two runs of this battery stopped there. Made with `makefs
-t ffs -B le -o version=1,bsize=8192,fsize=1024`, 376,192 sectors, put
at sector 256 of a 409,248-sector disk labelled by `build/mklabel`
with 32,768 sectors of swap, and the superblock's `fs_csmask` and
`fs_csshift` set to -512 and 9 as Lite2's mkfs sets them (see *What
was not established*).

**The run.** A script (scratch, not in the tree) boots single-user,
does what the installer's Configure would -- remounts the root
read-write, `pwd_mkdb -p /etc/master.passwd`, makes `/var/at`, `sh
MAKEDEV std` -- and leaves single-user, so that `/etc/rc` runs in full
and starts its daemons (syslogd, update, cron, routed, lpd, portmap,
mountd, nfsd, nfsiod, sendmail, inetd, amd), every one an exec; then
logs in as root on the console, through getty, login and tset, and
runs 43 steps, each in `/bin/sh`.

**What it showed.** On this kernel every command ran and gave the
right answer. Against the kernel before, the only differences are
these, each one listed in *Behaviour that changes*, and the old
kernel's output on each is what that list says it was:

| step | this kernel | the one before |
|---|---|---|
| `vmstat -m` | an `exec` line (358 requests by then, sizes 64 and 128) | no such type |
| a 64-byte `#!` line run from sh | echo prints the line and the script's name | refused; nothing printed |
| `limit stacksize 1025k` (csh), `ulimit -s 1025` (sh) | reports 1028, the child runs | reports 1025, the child runs |
| exec of a program open for writing (`exec 3>>`) | `text busy`, exit 2; runs once closed | ran |
| `ls /proc/$$`, `cmp /proc/curproc/file /usr/bin/cmp` | `file` listed; it is cmp itself | no `file`; ENOENT |

The same on both kernels, and right: `vmstat -i` (patch 258's
counters, named clk, pc0, com0, rtc0, wdc0, wdc1); `ps` with `-ax`,
`-axww`, `-axe`, `-axo` with `pid,ucomm,command` and
`pid,ppid,command`, `-l`, `-j` and `-u`, and `w`, all reading the
arguments and environment through `ps_strings`, and a background
`sleep 30` seen with its argument; a `#!/bin/sh` script with an
argument, and the same script given to sh; `exec` in `sh -c`; groff's
`afmtodit` (a 35-byte `#!` line, through perl) to its usage message; a
65-byte `#!` line refused on both; `unlimit stacksize`; `env -`, `env
FOO=bar`, `printenv`; `nice -5`, `nohup`, `time`, `find -exec`,
`xargs`; a set-user-ID copy of id owned by bin (euid 3) and a
set-group-ID one (egid 5); `at` and `atq` (set-user-ID root); gdb
running echo under ptrace to "Program exited normally"; a signal
caught by a shell after exec (`trap ... 30`); a directory, a missing
file and a file of text, refused by exec and handled by sh. `ktrace
-i` says the kernel lacks KTRACE, on both, as GENERIC has it.

Three things in ps's output look wrong and are the same on both
kernels, so none is exec's: nfsd shows as `nfsd-master master ter
(nfsd)`, most likely nfsd writing over its own argument strings; ps
lists itself as `ps -axo pid`, most likely ps splitting its own `-o`
argument in place (neither program was read to confirm it); and a child of the shell caught between fork and exec shows as
`(sh)`, which is a race, and changes run to run.

## What was not established

- **A traced process may exec a program it cannot read** (see
  *Behaviour that changes*; the test shows it). No BSD has ever refused
  this: not NetBSD 1.0 through the current tree (October 2026), not
  FreeBSD 2.0 through 12.0, not DragonFly through 6.4, not OpenBSD,
  whose current check_exec tests VREAD (Theo de Raadt, 10 October
  2026) only to pass the result to the program's dynamic linker, not to
  refuse the exec. The old body's refusal came with the shape #204 read
  from the encumbered file. Whether 4.5BSD should refuse it is the
  maintainer's call; doing so would be a correction with no donor line
  behind it.
- **MNT_NOSUID and MNT_NOEXEC are not tested**: they need a second
  mounted file system the test's bare disk does not have.
- **Demand paging is not done.** Segments are read in, as NetBSD 1.1's
  loader and this tree's old body both did; NetBSD's later loaders map
  aligned segments from the file. A performance question, left to 4.6BSD.
- **`vmstat -m`** names malloc types from `INITKMEMNAMES` compiled into
  vmstat; the vmstat in the last built root predates M_EXEC and shows
  type 63 unnamed until it is rebuilt (the multi-user run used one
  rebuilt against this tree's headers).
- **The last release build predates gdb and this patch.** Its base set
  (27 October) has no gdb and the old vmstat; the multi-user run added
  both from this tree. A full `build/release.sh` was not run for this
  patch.
- **The encumbered 4.4BSD `kern_exec.c` was not compared** against the
  new files, because it is never read. The measurements are against
  what may lawfully be read: Lite2's stub and 386BSD 0.0 and 0.1.
- **Found on the way, outside this item**: an image `makefs` makes
  leaves `fs_csmask` and `fs_csshift` zero, where Lite2's `mkfs` sets
  them (-512 and 9 at an 8K block); Lite2's `fs_cs()` indexes through
  them, so on such an image with more than one cylinder group this
  kernel faults in `ffs_nodealloccg` on the first inode allocated
  outside group 0. The release's makefs images (the boot floppy and
  the install root) are one group each and do not reach it; the
  multi-user test disk did, until the two fields were set as Lite2's
  mkfs sets them. Recorded for the build tooling, not changed.
- **Lite2's ls exits 0 for a file it cannot find** (`execsh.want`,
  both kernels). Not exec's; recorded.
