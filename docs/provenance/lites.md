# Lites, surveyed as a donor

Lites is a 4.4BSD-Lite server for the Mach microkernel, written by
Johannes Helander at Helsinki University of Technology and developed
further by the Flux Research Group at the University of Utah. Final
release 1.1.u3, 30 March 1996 -- nine months after Lite2, so a
contemporary by the standard `precedent.md` sets for NetBSD 1.0,
FreeBSD 2.0.5 and OpenBSD 1996.

It matters here for one reason: it is a 4.4BSD-Lite derivative with
its own line of development that **ran**. Wherever the Lite cut
emptied a body that a working system needs, Lites had to have one. So
it is a place to look for the settlement bodies that is neither
CSRG's encumbered text nor a later BSD's rewrite.

Source: `github.com/ryanwoodsmall/lites`, which carries the original
tarballs. The Utah and funet FTP sites it points at are the primary
source but are not reachable from this project's build container,
whose egress is limited to GitHub and the language-package mirrors.

## What it actually has, file by file

Against the twenty-three settlement bodies still empty in this tree:

| file | in Lites | copyright |
|---|---|---|
| `kern/tty_subr.c` | complete, 353 lines | Carnegie Mellon 1992 |
| `kern/vfs_bio.c` | complete, 790 lines | John S. Dyson 1994 |
| `kern/sys_process.c` | **still stubbed**, 2 | Regents |
| `kern/kern_acct.c` | **still stubbed**, 2 | Regents |
| `kern/kern_physio.c` | absent | |
| `kern/subr_rmap.c` | absent | |

And no `pmap.c` of any kind: Mach provides the VM system, so Lites
has no machine-dependent pmap at all. For the `pmap_remove`
self-map question in `status.md` it has nothing, and cannot.

## What each of those means

**`tty_subr.c` is a genuine candidate and a transplant, not a
restoration.** CMU rewrote the clist routines rather than filling in
Berkeley's -- `clcheck`, `getc`, `q_to_b`, `ndqb`, `putc`, `b_to_q`,
`nextc`, `unputc`, the same eight names this tree has stubbed, under
CMU's own copyright and in CMU's own prose. Its licence is the Mach
one:

	Permission to use, copy, modify and distribute this software
	and its documentation is hereby granted, provided that both
	the copyright notice and this permission notice appear in all
	copies ...

which is permissive and compatible, and which this tree already
carries throughout `sys/vm` -- every file there has a CMU notice. So
taking it would be kind D in `imports.md`'s terms, a transplant, and
would sit beside the Mach VM rather than beside Berkeley's kernel.

**`vfs_bio.c` is not a better donor than NetBSD 1.0.** Lites took
FreeBSD's rewrite -- John Dyson's 1994 file, the same one
`docs/provenance/missing.md` already records as a rewrite rather than
Lite's own file with the holes filled. The ten bodies written for this
tree so far came from NetBSD 1.0 for exactly that reason, and the four
remaining ones should too.

**`sys_process.c` and `kern_acct.c` are the useful negative result.**
Lites shipped, ran, and provided binary compatibility with four BSDs
and Linux, with both of those files still carrying `Body deleted`. So
neither is needed for a working system, which is worth knowing before
spending effort on them: six of the twenty-three stubs are in files a
running 4.4BSD-Lite derivative never filled.

## Status

Surveyed, not used. Nothing here has been taken into the tree. The
one candidate is `tty_subr.c`, and whether to prefer CMU's clist over
a contemporary BSD's is a decision for when that work is reached, with
`precedent.md`'s order applying as usual: this tree's own ports first,
then Lite, then the contemporaries, among which Lites now counts.
