# `execve`: the quoted `getxfile` lines, and the fit for what replaces it

Stage 3, item 2; audit station A2 (`roadmap-audit.md`): commits #204
and #206 ("execve, written: the kernel execs" and "execve: the stack
goes a page below USRSTACK, as Berkeley's i386 does"), measured by an
earlier reviewer at 53% line-match to 4.4BSD's `kern_exec.c`.
`plan-4.5.md`'s own instruction for this item: get the `getxfile`
lines quoted from 4.4 out, then find the first canonical fit for
`execve` itself -- 386BSD 0.1's own, if clear and if it closes the
ELF gap; otherwise NetBSD 1.0's exec framework, with 1.1 for ELF.

## What was quoted, and why it had to come out

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

## Measuring 386BSD 0.1's own `execve`

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

## NetBSD 1.0 (the framework) and 1.1 (ELF)

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

## What this patch does, and what it does not yet do

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

## What was not established

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
