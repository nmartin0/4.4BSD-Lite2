# Provenance: ELF startup code (lib/csu/i386_elf, lib/csu/common_elf)

The i386 ELF startup code in `usr/src/lib/csu/i386_elf/` and
`usr/src/lib/csu/common_elf/` is imported from NetBSD 1.6. This file
records where each file came from, under what notice, and what has
been changed since the import. It is a record of facts, not legal
advice.

## Source

`github.com/NetBSD/src`, branch `netbsd-1-6`, commit
`fa27a766022cec98a794a44a4ad8683457207950` (branch tip, 2006-06-01).
NetBSD 1.6 is the earliest NetBSD release whose i386 ELF `crt0.c`
compiles with a modern GCC: no `static` on `___start`, no multi-line
`__asm` string, and a `_start` alias.

Imported verbatim; each file byte-identical to the source commit when
imported.

| file | NetBSD revision | copyright notice |
|---|---|---|
| `i386_elf/crt0.c` | 1.12, 2001-12-30, thorpej | 1998 Christos Zoulas; 1995 Christopher G. Demetriou |
| `i386_elf/dot_init.h` | 1.2, 2002-01-14, drochner | 2001 Ross Harvey |
| `common_elf/common.h` | 1.7.2.1, 2004-05-28, tron | 1995 Christopher G. Demetriou |
| `common_elf/common.c` | 1.11.2.1, 2004-05-28, tron | 1995 Christopher G. Demetriou |
| `common_elf/crtbegin.c` | 1.18, 2002-04-08, skrll | 1998, 2001 The NetBSD Foundation, Inc. |
| `common_elf/crtend.c` | 1.9, 2001-12-30, thorpej | none; see below |
| `common_elf/dot_init.h` | 1.1, 2001-05-11, ross | 2001 Ross Harvey |
| `common_elf/dwarf2_eh.h` | 1.1, 2001-08-03, thorpej | 2001 The NetBSD Foundation, Inc. |

`common.h` and `common.c` are revisions on the 1.6 release branch,
made after the 1.6 release; the other files are at their 1.6
revisions. No file carries a UNIX System Laboratories notice; all are
post-4.4BSD-Lite work. The licences are Berkeley-style; some keep the
advertising clause, as 4.4BSD-Lite2's own Regents licence does.

Not imported: NetBSD's `Makefile` and `common_elf/Makefile.inc`, which
use make features 4.4BSD-Lite2 lacks, and `sysident.h`, NetBSD's
binary identification note, which `crtbegin.c` includes but which is
not part of this directory in NetBSD either.

## crtend.c carries no copyright notice

`common_elf/crtend.c` is 26 lines: the terminating entries of the
`.ctors`, `.dtors`, `.eh_frame` and `.jcr` lists and the epilogues
that close `_init` and `_fini`. It has a NetBSD revision id and no
copyright or licence text. What was found about it:

- It never carried a notice in NetBSD: every revision checked, from
  the 1.4 branch (1.6, 1998, cgd) through the netbsd-5 branch (1.12,
  2006, christos), has none. NetBSD 6 (2012) replaced it with
  per-architecture `crtend.S` files, which carry notices.
- It originates in a single change by Christopher G. Demetriou. The
  NetBSD revision ids that OpenBSD's copies keep date `crtbegin.c`
  revision 1.1 1996-09-12 16:59:03 and `crtend.c` revision 1.1
  1996-09-12 16:59:04, both by cgd. The companion `crtbegin.c`
  carries a Berkeley-style notice (1993 Paul Kranenburg).
- OpenBSD carries its own copies, citing that 1996 NetBSD origin, as
  `lib/csu/crtend.c` and `lib/csu/crtendS.c`. Every revision of
  OpenBSD's `crtend.c` checked, 2001 to 2017, has no notice, and both
  files were still in OpenBSD's tree on 2026-09-22.
- NetBSD's licensing page (netbsd.org/about/redistribution.html) says
  the Berkeley licence does not cover every file and that individual
  files should be checked; OpenBSD's policy page says copyright is
  implicit in creation. Neither publishes a grant covering files that
  carry no notice.
- No discussion of this file's licensing was found online.
- No equivalent with a notice fits this pairing: FreeBSD 3.0's
  `crtend.c` (Polstra) lacks the `.init`/`.fini` epilogues this
  `crtbegin.c` needs, and NetBSD 10's `crtend.S` belongs to a
  different `crtbegin`.

The maintainer decided to import it unchanged and record this
provenance, rather than seek written confirmation from the NetBSD
Foundation or write a replacement. No notice is added to it here: it
is not ours to add.

Switching donors would not avoid the question. OpenBSD maintains its
own csu, whose crt0.c is in some ways more modern -- it marks
___start `__used' rather than relying on a compiler flag, and it runs
`.init_array', which NetBSD 1.6's does not -- but its crtend.c has no
notice either, in every revision from 2001 to 2017, and it is still
shipping today. OpenBSD's startup contract is also its own kernel's,
while the kernel this tree is being built for is NetBSD 1.0-derived,
whose exec passes ps_strings in %ebx and the cleanup function in
%edx, which is what NetBSD 1.6's crt0 expects. OpenBSD's crt0 is
recorded here as the model for when `.init_array' support is needed.

## Changes since the import

Each change is marked at its site with an `AI-ONLY NOTE`.

- `common_elf/crtbegin.c`: the includes of `<sys/exec_elf.h>` and
  `"sysident.h"` are removed. They emit NetBSD's `.note.netbsd.ident`
  section, which labels every program as NetBSD; neither header is in
  this tree.
