# REDO: a commit-by-commit audit

Every commit on this branch, reviewed in batches of ten from the
first, against two questions:

1. **Is it a hack?** A hack is code whose correctness depends on
   something not guaranteed by the language, the ABI, or the hardware
   specification: reading uninitialised storage; inline assembly that
   assumes register allocation or omits constraints; depending on
   struct layout, padding or evaluation order that is not specified; a
   magic constant with no derivation; or working around a broken tool
   instead of fixing the tool. Code that is merely old-fashioned,
   verbose or slow is not a hack. A documented hardware workaround is
   not a hack. The test is "is this guaranteed to work", not "is this
   pretty".

2. **Would splicing have been more canonical?** Where a file was taken
   wholesale, could the same result have been reached by adding to
   4.4BSD-Lite2's own file, or by combining what two donors agree on
   and writing it in this tree's idiom? A wholesale import is the last
   resort, not the first.

A third question governs both, and came out of the `crt0.c` finding:
**if Berkeley's own code is unsound, the splice question does not
arise.** Carrying a hack forward because it is ours is the wrong
trade. Importing or rewriting is correct there.

Each entry records the verdict and the evidence. Where the answer is
"redo", the proposed shape is given but nothing is changed by this
file.

Searches use the Unix History Repository (849,161 commits, 195
branches) rather than release snapshots, so that "when did this
change and why" is answerable from commit messages rather than
inferred from diffs.

---

## Batch 1 — commits 1 to 10, 23 September

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 1 | `93777fa1` share/mk: `set -e` | no | already a splice | **keep** |
| 2 | `1937eace` build/make.sh | pending | n/a, ours | pending |
| 3 | `2e19a013` include/Makefile copies | no | already a splice | **keep** |
| 4 | `6900305d` build/sysroot.sh | pending | n/a, ours | pending |
| 5 | `962b0279` libc syscall.h depend | pending | — | pending |
| 6 | `44781479` libc static helpers | no | already a splice | **keep** |
| 7 | `bc09abe1` ldexp inline asm | **no — verified** | already a splice | **keep** |
| 8 | `081a0e6c` sys_nerr `__const` | — | — | superseded by `17271a25` |
| 9 | `1da1369b` bsd.lib.mk `ar cq` | pending | — | pending |
| 10 | `3edd3cdc` lorder | pending | — | pending |

### 1. `93777fa1` — share/mk: `set -e`

One token added to Berkeley's own `for` loop in `bsd.subdir.mk` and
`bsd.prog.mk`. Taken from NetBSD 1.0, which reads `(set -e; if test
-d ...`; OpenBSD has had the same since 1996.

The commit records that it chose this over the other thing both
donors did -- rewriting the loop, as OpenBSD has since done around
`SKIPDIR` -- "because one token leaves Lite2's loop as it is."

Not a hack: `set -e` is specified shell behaviour. **This is the
pattern this audit is looking for**, reached on the first commit of
the branch.

### 3. `2e19a013` — include/Makefile, the `copies` target

Berkeley's target is kept and its body replaced: `cd ${.CURDIR}/../sys`
instead of `/sys`, which on a Linux host is sysfs; `pax` instead of
`tar Hcf`, which means something else to GNU tar; `find` so the
headers under `ufs/ufs`, `ufs/ffs`, `ufs/lfs` and `ufs/mfs` are not
missed. Both contemporaries write it the same way.

Not a hack. A splice, not an import. **Keep.**

One cosmetic leftover: the `@-for i in ${LDIRS}` loop now contains
only the `rm -rf`, the `cd`/`tar` lines having moved out of it. It
works and matches the donors; worth a note if that file is touched
again.

### 6. `44781479` — four static helpers moved to file scope

`getttyent.c`'s `static char *skip(), *value();` and `sleep.c`'s
`static void sleephandler();` were declared at block scope, which GCC
14 rejects outright with no option to accept it.

Taken from OpenBSD 1996, which has these at file scope with `__P`
prototypes. The commit notes NetBSD rewrote `sleep.c` around
`nanosleep` instead, "which is more than this needs" -- correctly
declining the larger donor change.

Not a hack: a block-scope `static` declaration was always poor style
and is now invalid; moving it is the specified fix. A splice.
**Keep.**

### 7. `bc09abe1` — ldexp's inline assembly

The one to watch in this batch, since inline assembly is the hack
category. Berkeley's original:

```c
	asm ("fscale ; fxch %%st(1) ; fstp%L1 %1 "
		: "=f" (temp), "=0" (temp2)
		: "0" (texp), "f" (value));
```

`"=0"` ties an output to an input, which modern GCC rejects:
"matching constraint not valid in output operand". Replaced with
OpenBSD 1996's single-output form, which OpenBSD still ships
unchanged:

```c
	asm ("fscale" : "=t" (temp) : "0" (value), "u" ((double)exp));
```

**Not a hack.** `t` and `u` are x87 stack-position constraints with
defined meaning (top, and next-to-top); the input is tied to the
output explicitly; nothing is assumed about allocation. The commit
also verified it the right way -- "x87 constraint mistakes compile and
then return wrong numbers" -- eight cases against the host libm, every
result forced through a 64-bit store, at `-O0`, `-O1`, `-O2` and
`-O3`, no mismatch.

Already a splice: only the `asm` statement changed, Berkeley's
signature and return kept, three locals reduced to one. The commit
records why NetBSD 1.0's form was rejected (it names `"u"` as a
clobber, which is not a register name) and why FreeBSD 2.0.5's was
(three locals for two results). **Keep.**

### 8. `081a0e6c` — superseded

`17271a25` later reversed this: `const` was dropped from the
definition and the header returned to Berkeley's line. No separate
verdict needed.

### Pending in this batch

`1937eace` (make.sh) and `6900305d` (sysroot.sh) are this project's
own files and want reading line by line against the hack definition
rather than the splice question. `962b0279`, `1da1369b` and
`3edd3cdc` are short and unexamined.

### Batch 1 observation

Three of the five examined are already splices, and the branch's very
first commit states the principle explicitly. The wholesale imports
are not spread through the branch -- there are only four in all 127
commits, and they cluster in `lib/csu`, `libkvm`, `libcompat` and
`savecore`. Batch 1 suggests the edits to Berkeley's files were done
the right way from the start, and that the audit's yield will be
concentrated in those four imports and in the kernel work of 26-28
September.

---

## The four wholesale imports, surveyed ahead of their batches

Recorded here because they were found by enumerating `--diff-filter=A`
across the branch, before the batches reach them. Each has Berkeley's
own version of the same thing already in the tree.

### `dac5a892` — 8 files from NetBSD 1.6, `lib/csu`

**The splice question does not arise, and the import was right.**

Berkeley ships `lib/csu/i386/crt0.c`, 130 lines, which does most of
the same work. But its `start()` is unsound:

```c
	register struct kframe *kfp;	/* r10 */
	...
	asm("lea 4(%ebp),%ebx");	/* catch it quick */
	for (argv = targv = &kfp->kargv[0]; *targv++; ) ;
```

`kfp` is never assigned -- it is read uninitialised. The assembly is
meant to load it, assuming the compiler placed `kfp` in `%ebx`, and
the comment says `/* r10 */`, a VAX register never updated for the
i386. Compiled with GCC 13 at `-O2`, `%ebx` holds the GOT pointer
(`__x86.get_pc_thunk.bx`) and the assembly silently destroys it; and
`%ebp` is read although `-O2` emits no frame pointer.

Three unsound things at once. This is the clearest hack found so far
and it is Berkeley's own. `b2b31bae` reached the right conclusion for
the right reason -- "ELF startup tracks the compiler and not the tree,
so here later is closer."

Still open: whether 8 files were needed. `crtbegin.c` (205 lines) and
`crtend.c` (26) have no Berkeley ancestor at all -- a.out `crt0` had no
constructor support -- so they must be imported. `common.c` carries
`_rtld_setup`, `Obj_Entry` and `RTLD_MAGIC` for a dynamic linker this
tree does not have. Worth asking whether the `DYNAMIC` path should be
present.

### `fd425062` — `regex.c` from NetBSD 1.0

`b8596dbf` established NetBSD's is a 93-line shim, not Berkeley's
407-line engine. Lite2 ships two engines of its own:
`lib/libc/regex/` (POSIX `regcomp`, `regexec`, `engine.c`) and the
older `regexp`. So `re_comp`/`re_exec` could have been a shim over
Berkeley's own POSIX engine. **Splice candidate, unexamined** --
depends which engine NetBSD's shim targets.

### `b748de94` — `kvm_i386.c` from NetBSD 1.0

Berkeley ships `kvm_hp300.c`, `kvm_luna68k.c`, `kvm_mips.c` and
`kvm_sparc.c`; no i386. These files are mostly `_kvm_initvtop` and
`_kvm_kvatop`, which are pmap-shaped and short. **Splice candidate,
unexamined** -- an i386 one derived from Berkeley's own, against this
tree's `i386/include/pmap.h`.

### `b52de00b` — `zopen.c`, 741 lines, from NetBSD 1.0

`savecore.c:355` calls `zopen()`, so Berkeley expected it; the
library lived in `usr.bin/compress`, dropped for the LZW patent. The
tree does contain LZW --
`contrib/mh-6.8.3a/miscellany/compress-4.0/compress.c` and
`contrib/file-3.12/compress.c`. **Splice candidate, unexamined.**
`7cb61884` already calls this import the branch's one inversion: 741
lines for crash-dump compression while ten lines of `b_actf`
adaptation that would give the kernel a disk were declined.

---

## The three open imports, reviewed

### `fd425062` — `regex.c` from NetBSD 1.0 — **keep the import**

The splice question has no target. Berkeley did not ship a cut-down
`re_comp`; it shipped `4.3/re_comp.3`, the manual page, **and no code
at all**. There is nothing of Berkeley's to add to.

NetBSD's file is 93 lines and is a shim over `<regexp.h>` --
`regcomp(s)` returning `regexp *`, then `regexec(re_regexp, s)`. That
is Henry Spencer's early engine, which **Lite2 already ships** at
`lib/libcompat/regexp/regexp.c`. So the import does exactly what a
splice would have done: it bridges `re_comp`/`re_exec` onto this
tree's own engine. Writing those 93 lines ourselves would produce the
same file with a worse provenance.

Not a hack. **Keep.**

Recorded as a trap, not a defect: `regcomp` and `regexec` are defined
in both `libc.a` (POSIX, `regcomp(preg, pattern, cflags)`) and
`libcompat.a` (Spencer, `regcomp(exp)`), with incompatible
signatures. This is **Berkeley's own arrangement**, not something the
import created -- `usr.bin/grep/egrep/Makefile:11` reads `LDADD=
-lcompat # must search compat to get spencers early regexp package`,
deliberately searching libcompat first. Nothing in this tree links
`-lcompat` while including `<regex.h>`, so it is currently
unreachable. A future program that did both would silently get the
wrong one. The Makefile already notes the parallel `regerror`
collision and why an archive may carry two definitions.

### `b748de94` — `kvm_i386.c` from NetBSD 1.0 — **redo as a splice**

The strongest splice candidate of the four.

Berkeley ships four machine files -- `kvm_hp300.c` (286 lines),
`kvm_sparc.c` (236), `kvm_mips.c` (207) -- and no i386. The imported
file is 221 lines and defines `_kvm_freevtop`, `_kvm_initvtop`,
`_kvm_kvatop`, `_kvm_uvatop`: the same four as `kvm_hp300.c`, which
has those plus `_kvm_vatop`.

**Only about 30 of its 221 lines mention anything i386-specific** --
`PG_`, `PTD`, `NBPG`, page-table walking. The remaining ~190 are the
`kvm_t` plumbing, error handling and the vatop structure that
Berkeley's own four files already share between them.

So the canonical form is Berkeley's `kvm_hp300.c` with its hp300
page-table walk replaced by the i386 one, against this tree's
`i386/include/pmap.h` -- not a file from another tree. The result
would sit beside its three siblings in the same idiom rather than
alongside them in NetBSD's.

Not a hack either way. **Redo: splice.** Worth building both and
diffing before committing, since NetBSD's i386 walk may be the better
source for the 30 machine-specific lines even if the frame is
Berkeley's.

### `b52de00b` — `zopen.c`, 741 lines, from NetBSD 1.0 — **keep the import**

The splice candidate does not survive contact.

`savecore.c` calls `zopen(path, "w", 0)` at lines 355 and 417, so
Berkeley expected a stdio-like compressing stream: `zopen`, `zread`,
`zwrite`, `zclose`, built on `funopen`. The imported file provides
exactly that.

`contrib/mh-6.8.3a/miscellany/compress-4.0/compress.c` is 1407 lines
and is **a program, not a library**: `main()` at line 305, `output()`
and `getcode()` writing to globals and file descriptors, no cookie,
no stream abstraction. `contrib/file-3.12/compress.c` is a file-type
prober, not a compressor. Neither is Berkeley's own code -- both are
third-party contributions carried in `contrib/`.

Turning either into `zopen` would mean writing the stream layer from
scratch and reparenting the LZW core onto it: more invention than the
import, from a worse ancestor, for the same result.

Not a hack. **Keep.** `7cb61884`'s self-criticism stands on different
grounds -- the inversion is that 741 lines were imported for
crash-dump compression while ten lines of `b_actf` adaptation that
would give the kernel a disk were declined. That is a question of
ordering, not of whether this import was the right shape.

### Import scorecard

| import | verdict |
|---|---|
| `dac5a892` csu, 8 files | **keep** -- Berkeley's `crt0.c` is unsound; splice question does not arise. Open: whether the `DYNAMIC` path and `common.c`'s `_rtld_setup` belong in a tree with no dynamic linker |
| `fd425062` regex.c | **keep** -- no Berkeley code to splice into; the import already bridges onto this tree's own engine |
| `b748de94` kvm_i386.c | **redo as splice** -- ~190 of 221 lines duplicate what `kvm_hp300.c` already has |
| `b52de00b` zopen.c | **keep** -- the tree's LZW is a program in `contrib/`, not a library, and not Berkeley's |

One of four wants redoing. The audit's yield is in the kernel work,
not here.

---

## Batch 1, re-audited

The batch was reviewed a second time because its weakest claim rested
on a bad instrument. One verdict changed.

### CORRECTION: `b748de94` kvm_i386.c — **keep the import**

The first pass said "only about 30 of its 221 lines mention anything
i386-specific; the other ~190 duplicate plumbing." That came from
`grep -c` on a pattern of i386 tokens, which counts lines matching a
regex, not machine-specific lines, and 221 counted comments and
licence. Measured properly, comments and blanks stripped:

	kvm_hp300.c   194 code lines
	kvm_i386.c    118 code lines
	in common      81
	differing     166

and the 81 "common" lines are `#include` directives, the `KREAD`
macro, `btop`/`ptob` and error strings -- not shared logic. The bodies
differ because the hardware does:

	_kvm_initvtop   hp300 35 lines, nlist of 4 (_lowram, _mmutype,
	                _Sysseg); i386 31 lines, nlist of 2 (_IdlePTD)
	_kvm_kvatop     hp300  7;  i386 19
	_kvm_uvatop     hp300 43;  i386 42

hp300 resolves through an HP MMU segment table; i386 walks a page
directory. There is no body of shared plumbing to splice into. Writing
an i386 file from `kvm_hp300.c` would mean keeping ~20 lines of
includes and replacing everything else, which is a rewrite with a
borrowed header block, not a splice.

**All four imports are correct as they stand.** The audit's yield is
not here.

### Donor citations, verified

Four citations were repeated from commit messages rather than checked.
Closed:

**`93777fa1` set -e — verified exactly.** NetBSD 1.0
`share/mk/bsd.subdir.mk:11` reads `(set -e; if test -d
${.CURDIR}/$${entry}.${MACHINE}; then \`.

**`bc09abe1` ldexp — verified, and the donor moves.** NetBSD 1.0's
`lib/libc/arch/i386/gen/ldexp.c`, under `#if __GNUC__ >= 2` and so
live for any modern compiler:

	asm ("fscale" : "=t" (temp) : "0" (value), "u" ((double)exp) : "u");

The **constraints are identical** to what landed. Only the trailing
clobber differs, and it is genuinely invalid -- GCC 13 rejects it with
`error: unknown register name 'u' in 'asm'`. OpenBSD master ships the
same constraints without it.

So the reasoning in the commit is right and the attribution is wrong
under `precedent.md` rule 5. The earliest tree with this form is
**NetBSD 1.0, October 1994**, not OpenBSD 1996. Owed: a `Revision:`
restating the donor as NetBSD 1.0's constraints with its invalid
clobber dropped, the form OpenBSD corroborates.

**`1da1369b` tsort -q — the attribution is wrong.** The note says
"-q is BSD tsort's". `usr.bin/tsort/tsort.c:123` reads
`getopt(argc, argv, "dl")` -- no `q`. It is OpenBSD's own addition,
not Berkeley's. Small, but the same class of error: a later tree's
feature credited to the era. Owed: a `Revision:`.

### A stated blind spot

**Every "OpenBSD (1996)" citation on this branch is unverifiable from
this host.** `OPENBSD_2_0`, `2_1`, `2_6`, `3_0` all 404 on
raw.githubusercontent, for `lib/libc`, `lib/libedit` and `share/mk`
alike; only `master` resolves. OpenBSD master corroborates the *shape*
of some claims -- `getttyent.c:39` does declare `static char
*skip(char *);` at file scope -- but not the date, and where OpenBSD
has since rewritten a file the claim cannot be touched at all:
`sleep.c` is now built on `nanosleep` with no `sleephandler`.

`3edd3cdc` already carries a self-correction for exactly this failure:
its note originally said "OpenBSD's lorder" meaning OpenBSD's current
tree, which in this project's idiom reads as the 1996 tree. The branch
has been bitten by it once and caught it.

Until an OpenBSD 1996 source is reachable, citations to it are taken
on trust and should be marked as such rather than treated as checked.

### Host accommodations inside the tree proper, verified

Checked against this tree's own tools rather than reasoned about.
These exist only because the build host is Linux and must be reverted
when 4.5BSD hosts itself:

| commit | claim | verified against |
|---|---|---|
| `1da1369b` `ar cq` not `cTq` | host-only | `usr.bin/ar/ar.c:135` has `case 'T':`; `archive.c:224` warns "truncated to". `T` is meaningful to Berkeley's ar |
| `3edd3cdc` lorder names files itself | host-only | `usr.bin/nm/nm.c:101` sets `print_file_each_line` for `-o`. Berkeley's nm does emit per-line file information |
| `2e19a013` `copies` reads `../sys` not `/sys` | host-only | `/sys` exists on a BSD system; on Linux it is sysfs |

And these are **not** host accommodations -- they are correct for any
cross build and stay:

| commit | why it stays |
|---|---|
| `93777fa1` `set -e` | a failed `cd` must end the entry; NetBSD 1.0 and OpenBSD both have it |
| `9995dc2e` `HOSTCC` | `mkinit`, `mknodes`, `mksyntax` generate source and must run on the build machine; OpenBSD has carried it since 1996, NetBSD reached `HOST_CC` in 1.4 |
| `ad7131e0` `LIB*` under `DESTDIR` | NetBSD 1.0 does the same; absolute `/usr/lib` paths cannot work for any staged build |

---

## Tree diff against pristine 4.4BSD-Lite2

Baseline `b3d2855f`, the imported tarball; head `origin/dev3`.

**Upstream modified no Lite2 source.** Between the tarball and where
this project's work starts (`279f5819`), nothing under `usr/src` was
changed -- only added.

**This project's changes: 14 files added, 110 modified**, every one
inside `usr/src`:

	usr.bin 34   lib 26   sys 20   sbin 10   libexec 6
	usr.sbin 5   share 4  bin 4    include 1

Outside `usr/src`: `.gitignore`, `build/` (three bootstrap scripts),
`docs/`. Those are the two permitted categories -- scratch tooling and
patches to the tree.

### Foreign material in the tree proper: 60 PDFs

`64b39d64`, "PostScript docs converted to PDF", added 60 files, about
13 MB, inside the tree rather than beside it:

	usr/share/doc/{psd,smm,usd}/*.pdf   beside Berkeley's own *.ps
	usr/src/usr.bin/bdes/bdes.pdf
	usr/src/usr.sbin/amd/doc/amdref.pdf

Pure additions; nothing was removed, and the PostScript sources are
still there. Pristine Lite2 contains no PDF at all.

These are neither 4.4BSD-Lite2 nor this project's work. They are
inherited from the fork point and are the only thing in the tree that
fits neither category. **Not removed here; recorded for a decision.**

### The imported tarball is not the canonical release

Our import has 20,490 files. The Unix History Repository's
`BSD-4_4_Lite2` tag has 20,001. **489 files we have and it does not;
275 it has and we do not.**

Most of its 275 are its own packaging (`LICENSE`,
`Caldera-license.pdf`, `Domestic/files`), but not all:
`usr/src/usr.sbin/sendmail/` has `FAQ`, `Files.base`, `Files.cf`,
`Files.misc` and `Files.xdoc` there and not here.

Our 489 extra concentrate in:

	usr/src/contrib/emacs-18.57/lisp              48
	etc                                           46
	usr/src/contrib/X11R5-hp300/.../mfb, cfb      27
	usr/src/etc/etc.tahoe                         12
	usr/src/etc/etc.hcx9                          10

Direction unknown: whether the `alge.anart.no` tarball is more
complete than the UHR's reconstruction, or carries files from a
different distribution.

**This matters beyond bookkeeping.** `docs/provenance/missing.md` is
an inventory of what 4.4BSD-Lite2 does and does not contain, and
`b8596dbf`'s marker test turns "why is this absent" into a lookup
against this tree. If this tree is not the canonical Lite2, some of
those absences may be artefacts of the tarball. **Unresolved, and it
should be settled before any further claim rests on "Lite2 does not
contain X".**

## The CSRG history: Lite2 is the end of the line

Checked in the Unix History Repository, branch
`BSD-4_4_Lite2-Snapshot-Development`, 59,488 commits.

The last real delta is `65f5942e278`, Jan-Simon Pendry, **23 June
1995**, and its own message records the fact:

> bandaid to avoid unlock panic until we do it right; trivia: this was
> the final delta done by CSRG

The release snapshot is dated **24 July 1995**, a month later, with no
development in between. The only later commit on the branch is
Spinellis adding licence files in 2026.

So there is **no post-release CSRG development to recover**. Lite2 is
the end of the lineage, not a cut with work continuing behind it.
6,659 commits separate Lite1 from Lite2.

---

## Batch 2 — commits 11 to 20, 23 September: the ELF phase

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 11 | `ffb70e0d` libc/i386 `_C_LABEL` | no — verified | already a splice | **keep** |
| 12 | `fc4a51a0` libc.tags under DESTDIR | no | — | **keep** |
| 13 | `2ed28a1e` sysroot.sh builds libc | n/a, bootstrap | n/a | **keep, bootstrap** |
| 14 | `dac5a892` NetBSD 1.6 ELF csu, 8 files | no | splice question does not arise | **keep** (see import survey) |
| 15 | `bbfefd8c` crtbegin.c: drop ident note | no | n/a | **keep** |
| 16 | `c5fd6177` csu Makefile | **see below** | n/a, ours | **keep, with a flag recorded** |
| 17 | `641a79ec` lib/Makefile ELF csu | no | — | **keep** |
| 18 | `f2bdfedb` sysroot.sh startup files | n/a, bootstrap | n/a | **keep, bootstrap** |
| 19 | `ad7131e0` bsd.prog.mk DESTDIR | no | already a splice | **keep** — not host-only |
| 20 | `9e6bb751` make.sh ELF settings | n/a, bootstrap | n/a | **keep, bootstrap** |

### 11. `ffb70e0d` — `_C_LABEL` through libc's i386 assembly

Ten files. Lite2's assembly writes C names as a.out spells them --
`ENTRY(x)` pastes `_/**/x`, and several files reference `_errno`,
`_sigblock` directly. Under ELF a C name carries no underscore, so
none of it would link.

**Not a hack, and verified rather than assumed.** The macro rests on
`__ELF__` being defined; `gcc -m32 -dM -E` confirms `#define __ELF__
1`, and preprocessing a test `ENTRY(foo)` through this tree's own
`DEFS.h` with `-traditional-cpp` yields `.globl foo; foo:` -- the
right arm is taken in a real compile.

The block is NetBSD 1.5's `<machine/asm.h>`; OpenBSD's i386 `asm.h`
says the same. A splice: Berkeley's `ENTRY` is kept and only its
expansion routed through the new macro, Berkeley's files otherwise
untouched apart from the specific symbol references.

**One improvement worth noting for the hack rule.** `Ovfork.s`
carried `.set vfork,66` -- a magic constant with no derivation, which
is on the hack list. It became `movl $(SYS_vfork),%eax`, the generated
number. The commit's stated reason was the ELF name collision, but it
also removed a hardcoded syscall number. NetBSD 1.0 writes it the same
way.

The commit records a deferral honestly: NetBSD and OpenBSD keep
`_C_LABEL` in the kernel's `<machine/asm.h>` and reduce libc's
`DEFS.h` to a single include; this tree has no i386 `asm.h`, so the
block is duplicated in `DEFS.h` and `SYS.h` until the kernel work
brings one. **That duplication is a known debt, recorded at both
sites.**

### 16. `c5fd6177` — the csu Makefile, and `-I-`

Ours, in Lite2's idiom. Two things in it are worth recording.

`-fno-toplevel-reorder` is load-bearing and measured: without it GCC
14 moves `_init`'s own code into `.init` where `init_fallthru` also
starts, "so `_init` calls itself and the program dies at startup
(measured: segmentation fault)". That is a real finding, not a
precaution.

`-I- -I${.CURDIR}` is NetBSD's include order, needed so `dot_init.h`
resolves to this directory's rather than the empty one in
`../common_elf`. **GCC calls `-I-` obsolete and warns once per
compile**, and the note says so. It is not a hack by the definition --
the behaviour is documented and honoured -- but it is a dependency on
a deprecated flag, and GCC could remove it. The modern spelling is
`-iquote`, which GCC's own diagnostic suggests. **Recorded as a flag
to revisit, not a redo.**

### 19. `ad7131e0` — `LIB*` under `DESTDIR`

Replaces eighteen absolute `/usr/lib/libX.a` definitions with
`DESTDIR`-relative ones. **Not a host accommodation** -- absolute
paths cannot work for any staged or cross build, and NetBSD 1.0 does
the same. This stays after self-hosting. A splice: the variable list
is Berkeley's, only the prefix changed.

### Batch 2 observation

Six of the ten are either bootstrap tooling outside the tree or
splices into Berkeley's files. The one wholesale import in the batch
is `dac5a892`, already surveyed and already correct.

No redo arises from this batch. Two things recorded for later:
`_C_LABEL` duplicated in `DEFS.h` and `SYS.h` pending an i386
`<machine/asm.h>`, and the `-I-` dependency.

---

## Batch 3 — commits 21 to 30, 23 September

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 21 | `1e3ffe96` mman.h includes sys/types.h | no | already a splice | **keep** |
| 22 | `f6de38bb` libutil drops `-I/sys` | no | — | **keep** — *not* host-only |
| 23 | `457aefb5` sysroot.sh library list | n/a, bootstrap | n/a | **keep, bootstrap** |
| 24 | `9995dc2e` share/mk HOSTCC | no | already a splice | **keep** — not host-only |
| 25 | `ecf7b9b3` bin/sh tools with HOSTCC | no | already a splice | **keep** |
| 26 | `d28d2352` mknodes `infp` | **the original was** | already a splice | **keep — see below** |
| 27 | `d3f0c40a` make.sh host compiler | n/a, bootstrap | n/a | **keep, bootstrap** |
| 28 | `eb097bf3` make.sh byacc not bison | n/a, bootstrap | n/a | **keep, bootstrap** |
| 29 | `64490f54` sysroot.sh libterm, libedit | n/a, bootstrap | n/a | **keep, bootstrap** |
| 30 | `4fc8837e` lib/libl Makefile | no | n/a, absent from Lite2 | **keep** |

### 22. `f6de38bb` — `-I/sys` is dead, not host-specific

I had this filed as a probable host accommodation, since `/sys` on
Linux is sysfs. **It is not.** Verified: the five sources
(`login.c`, `login_tty.c`, `logout.c`, `logwtmp.c`, `pty.c`) include
only `<errno.h>`, `<fcntl.h>`, `<grp.h>`, `<stdio.h>`, `<stdlib.h>`,
`<string.h>`, `<sys/file.h>`, `<sys/ioctl.h>`, `<sys/param.h>`,
`<sys/stat.h>`, `<sys/time.h>`, `<sys/types.h>`, `<termios.h>`,
`<unistd.h>`, `<utmp.h>` -- all ordinary, none from the kernel tree.

The flag was dead in Lite2 itself, and the lineage agrees: NetBSD 1.0,
NetBSD 1.1 and OpenBSD 1996 all carry this Makefile without it.
FreeBSD 2.0.5 keeps it. **Stays after self-hosting.** The
host-accommodation list remains three items: `ar cq`, `lorder`,
`copies`' `/sys`.

### 26. `d28d2352` — where the two rules disagree, and which wins

`mknodes.c` had:

	static FILE *infp = stdin;

which fails on a build host whose `stdin` is not a link-time constant
-- "initializer element is not constant". On 4.4BSD `stdin` is
`&__sF[0]`, a constant address, so it initialises a static happily.
The fix assigns `infp` in `main` instead. NetBSD 1.6 splits it the
same way.

This is the first commit where the **host-accommodation rule and the
no-hacks rule point in opposite directions**, and it is worth setting
down how they resolve.

By the host rule this looks revertible: `mknodes` is built with
`HOSTCC` because the build runs it, so once 4.5BSD hosts itself the
compiler is its own, `stdin` is `&__sF[0]` again, and Berkeley's line
would compile.

By the hack rule it is not. C does not guarantee that `stdin` is a
constant expression -- it is permitted to be, and on 4.4BSD it
happens to be, but the original depends on something the language
does not promise. That is the definition. **The no-hacks rule wins**:
the fix stays after self-hosting, and the line is correct C rather
than correct-on-this-libc C.

**General form, for the rest of the audit:** where a change is both a
host accommodation and a correctness fix, it is a correctness fix and
does not go on the reversion list. Only changes that are *purely*
host accommodations -- correct code made different for a foreign
tool -- are reverted.

### 30. `4fc8837e` — a Makefile for a library Lite2 ships without one

Not an import of code: Lite2 carries the lex run-time sources under
`contrib/flex-2.5.2` and no Makefile to build them. `bin/sh` links
`-ll` for the `yywrap()` its generated scanner calls, as NetBSD 1.0,
FreeBSD 2.0.5 and OpenBSD 1996 all do; every one of those built
`libl`.

The Makefile is NetBSD 1.0's with `.PATH` changed, and the note
records why: "contrib/flex-2.5.2 rather than usr.bin/lex, since Lite2
carries flex under contrib and has no usr.bin/lex". NetBSD's `LINKS`
for `libfl.a` left out, nothing here asks for `-lfl`.

Worth noting against the splice question: there was no Lite2 file to
splice into, and the thing added is 7 lines of Makefile around
Berkeley's own sources. **The library is Berkeley's; only the recipe
is borrowed.** That is the right shape.

### Batch 3 observation

Half the batch is bootstrap tooling outside the tree. Of the five
that touch it, four are splices and one adds a Makefile around
sources already present.

No redo. One classification corrected (`f6de38bb` is not host-only),
and one general rule established: a change that is both a host
accommodation and a correctness fix counts as a correctness fix.

---

## Batches 2 and 3, re-audited

Eleven claims repeated from commit messages rather than checked. Ten
hold exactly; one omission found.

### Verified exactly

| claim | evidence |
|---|---|
| `_C_LABEL` block is NetBSD 1.5's `<machine/asm.h>` | `nb15-asm.h:64-72`, same three-arm structure and text; ours differs only in not indenting inside the nested `#ifdef` |
| OpenBSD's i386 `asm.h` has `#define _C_LABEL(name) name` | OpenBSD master `asm.h:61` |
| NetBSD 1.0 writes `ENTRY(vfork)` / `movl $(SYS_vfork),%eax` | `Ovfork.S:54,56` |
| NetBSD 1.0's `libutil/Makefile` has no `-I/sys` | reads `CFLAGS+=-DLIBC_SCCS`, exactly what we now have |
| NetBSD 1.6 splits `mknodes`' `infp` | `mknodes.c:103` declares, `:132` assigns in `main` |
| `bin/sh` links `-ll` | `bin/sh/Makefile:9`, `LDADD+= -ll -ledit -ltermcap` |
| only `libyywrap.o` is used | `libl.a` holds `libmain.o` (defines `main`, needs `yylex`) and `libyywrap.o` (defines `yywrap`); only the latter resolves anything `sh` wants |
| `stdin` is a constant address on 4.4BSD | `include/stdio.h:212`, `#define stdin (&__sF[0])` |

### `-fno-toplevel-reorder` -- verified, and sharper than claimed

The strongest assertion in batch 2, taken on trust until now.
Compiling `crtbegin.c` both ways on this host:

	with    -fno-toplevel-reorder   .init 0x60, three symbols:
	                                _init@0x00, __ctors@0x26,
	                                init_fallthru@0x5d
	without                         .init 0x51, one symbol: _init

Without the flag the ordered `.init` fragments collapse and
`init_fallthru` -- which must sit at the end of `.init` -- stops being
a separate symbol. **Load-bearing, demonstrated rather than quoted.**

### Finding: `4fc8837e` dropped `NOPIC=` unmentioned

NetBSD 1.0's `lib/libl/Makefile`:

	LIB=	l
	SRCS=	libmain.c libyywrap.c
	NOPIC=
	LINKS=	/usr/lib/libl.a /usr/lib/libfl.a
	.PATH:	${.CURDIR}/../../usr.bin/lex

Ours drops `LINKS` **and `NOPIC=`**. The commit accounts for `LINKS`
("nothing in this tree asks for -lfl") and says nothing about
`NOPIC`.

Harmless in substance -- Lite2's `bsd.lib.mk` has no `NOPIC`, no
`PICFLAG` and no `.so` rule, so the variable means nothing here. But
the commit claims to say what it left out and does not say all of it,
and this project's method rests on those claims being complete.
**A `Revision:`, not a redo.**

---

## Corrections owed, running list

None of these change code. All three correct a provenance claim that
was checked and found wrong.

| commit | correction |
|---|---|
| `bc09abe1` ldexp | donor is **NetBSD 1.0** (Oct 1994), whose constraints are identical; only its invalid `: "u"` clobber was dropped. The commit credits OpenBSD 1996, two years later |
| `1da1369b` bsd.lib.mk | `tsort -q` is **OpenBSD's own addition**; `usr.bin/tsort/tsort.c:123` reads `getopt(argc, argv, "dl")`. The note says "-q is BSD tsort's" |
| `4fc8837e` libl | `NOPIC=` was dropped from the cited NetBSD 1.0 Makefile without being mentioned |

---

## Blind spot closed: OpenBSD 1996 is reachable

The GitHub mirror `openbsd/src` has **zero tags**, which is why every
`OPENBSD_2_x` URL returned 404. But `master` carries the full
converted CVS history -- **247,510 commits back to 18 October 1995**,
"initial import of NetBSD tree". Dates serve where tags do not. The
tree at `be8fde6bb24578eed718626d4e2b2c04c14aee59` (1 November 1996)
is OpenBSD 2.0-era.

All eight OpenBSD 1996 citations on this branch verified against it:

| claim | 1996 tree |
|---|---|
| `bsd.subdir.mk` has `set -e` | `:12` -- `(set -e; if test -d ${.CURDIR}/$${entry}.${MACHINE}; then \` |
| `sys.mk` has carried `HOSTCC` since 1996 | `:27` -- `HOSTCC?= cc` |
| `getttyent.c` statics at file scope with `__P` | `:46-47` |
| `sleep.c` declares `sleephandler __P((int))` | `:42`, and still signal-based |
| `ldexp.c` is the `"=t"/"0"/"u"` form | confirmed, no clobber |
| `tsort` has `-q` | `:128` -- `getopt(argc, argv, "dlq")` |
| `lorder` reads `nm -go $*` | `:76`, confirming `3edd3cdc`'s own self-correction |
| `libutil/Makefile` has no `-I/sys` | `:5` -- `CFLAGS+=-DLIBC_SCCS` |

The `sleep.c` claim is the one previously marked unverifiable because
OpenBSD master now uses `nanosleep`. The 1996 tree has
`sleephandler`; the doubt was an artefact of reading the wrong
decade.

### Corrections list revised to two

**`1da1369b` drops off.** I flagged "-q is BSD tsort's" as wrong
because Lite2's `tsort.c:123` reads `getopt(argc, argv, "dl")`.
OpenBSD 1996 reads `"dlq"`. It is a BSD tsort, just not Berkeley's.
Defensible phrasing, not an error.

**`bc09abe1` stands and tightens.** OpenBSD 1996 has the same
constraints under the same `#if __GNUC__ >= 2`, but NetBSD 1.0 had
them two years earlier, so under `precedent.md` rule 5 the donor is
NetBSD 1.0 with its invalid `: "u"` clobber dropped; OpenBSD 1996 is
corroboration, not source. One real difference: OpenBSD writes
`__asm`, NetBSD 1.0 and this tree write `asm`.

Remaining: `bc09abe1` donor attribution, `4fc8837e` unmentioned
`NOPIC=`.

Every batch from here checks OpenBSD 1996 directly.

---

## Batch 4 — commits 31 to 40, 24 September

First batch checked against the real OpenBSD 1996 tree rather than on
trust.

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 31 | `676c9af0` sysroot.sh libl | n/a, bootstrap | n/a | **keep, bootstrap** |
| 32 | `f2eb4e19` sbin kernel headers | no | already a splice | **keep** — *not* host-only, verified |
| 33 | `1e98a429` sbin LFS sources | no | already a splice | **keep** — same reasoning |
| 34 | `e7c4c061` routed `iftraceinit` | no | already a splice | **keep** |
| 35 | `3101ef91` docs precedent rules | n/a, docs | n/a | **keep** |
| 36 | `3a30baa0` libcompat source list | no | — | **keep** |
| 37 | `e8301cf1` sysroot.sh libcompat | n/a, bootstrap | n/a | **keep, bootstrap** |
| 38 | `17271a25` sys_nerr | no | already a splice | **keep** — verified, donor citation exact |
| 39 | `cd2df4dd` librpc file-scope statics | no | already a splice | **keep** |
| 40 | `c9bc53f4` librpc header ownership | no | — | **keep, with a note** |

### 32/33. `f2eb4e19`, `1e98a429` — `-I/sys` in sbin, and they are NOT host-only

I had both filed as probable host accommodations, since `/sys` on
Linux is sysfs. **Verified and they are not.** OpenBSD 1996's
`sbin/mount_null/Makefile:9`:

	CFLAGS+= -I${.CURDIR}/../../sys -I${MOUNT}

**Character-for-character what landed here.** FreeBSD 2.0.5 writes
the same at its line 8. NetBSD 1.0 writes `-I${DESTDIR}/sys`, which
needs the kernel sources inside the target root. All three claims in
the commit message are exact.

Two contemporaries independently reached this spelling for a native
build, so it stays after self-hosting. The host-accommodation list
remains **three**: `ar cq`, `lorder`, and `include/Makefile`'s
`copies`.

### 38. `17271a25` — the contradiction is Berkeley's, and both donors resolved it the same way

Lite2 writes the two sides of `sys_nerr` differently -- `const int`
in `gen/errlst.c`, `extern int` in `<stdio.h>` -- in Lite2 and in
4.4BSD-Lite alike. GCC 14 rejects the pair: "conflicting type
qualifiers for 'sys_nerr'".

Verified: **NetBSD 1.0's `errlst.c:139`** reads `int sys_nerr = {
sizeof sys_errlist/sizeof sys_errlist[0] };` -- no `const`.
**OpenBSD 1996's `include/stdio.h:240`** reads `extern int
sys_nerr;` -- no `const`, matching this tree's header exactly; its
definition lives in `lib/libc/gen/errlist.c:143` as `int _sys_nerr`,
by then renamed behind an alias.

So both contemporaries dropped `const` from the definition rather
than adding it to the header, which is what this commit does. The
earlier `081a0e6c` had gone the other way and is correctly superseded.

`sys_errlist` left alone, since Berkeley says `const` on both sides
there. **Right call, right reason, citation exact.**

### 40. `c9bc53f4` — and a deprecated separator inherited

`chown bin.bin` became `chown ${BINOWN}.${BINGRP}`, matching this
tree's own `include/Makefile` in all four of its header loops. The
original fails for anyone but root and the line carries no `-`
prefix, so the whole install stopped.

Not a hack and not purely host-driven -- it makes Berkeley's file
consistent with Berkeley's other files.

**Recorded:** the `owner.group` separator is deprecated in GNU
coreutils, which warns `chown: warning: 'user.user' should be ':'` on
every run of `sysroot.sh`. The modern spelling is `owner:group`.
Berkeley's own files use the dot throughout, so changing it is a
tree-wide question, not a one-line fix, and it is cosmetic until
coreutils removes the form.

### 32/33 revisited — `1e98a429`'s citation is false

**REDO CANDIDATE.** `f2eb4e19` and `1e98a429` were verdicted together
on resemblance; only the first had been opened. Reading the second:

The note on both changed lines in `sbin/dumplfs/Makefile` says
"OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile that way."
The three files in full:

	OpenBSD 1996              FreeBSD 2.0.5             ours
	PROG=   dumplfs           PROG=   dumplfs           PROG=   dumplfs
	(no CFLAGS line)          CFLAGS+=-I/sys/ufs/lfs    CFLAGS+=-I${.CURDIR}/../../sys/ufs/lfs
	SRCS=   dumplfs.c ...     SRCS=   dumplfs.c ...     SRCS=   dumplfs.c ...
	.PATH:  ${.CURDIR}/..     .PATH:  ${.CURDIR}/..     .PATH:  ${.CURDIR}/../../sys/ufs/lfs

The `.PATH` claim is exact -- both donors write it that way. **The
`CFLAGS` claim is false in both directions.** OpenBSD 1996 has no
`CFLAGS` line at all; it deleted the `-I` and relies on `.PATH`.
FreeBSD 2.0.5 keeps the literal `-I/sys/ufs/lfs` this commit is
replacing.

So **no donor writes the line that landed.** It is this project's own
construction presented as two trees' practice. The note appears to
have been reused from `f2eb4e19`, where the identical-looking change
to `sbin/mount_null/Makefile` genuinely is verbatim OpenBSD.

**OpenBSD's answer is better than ours.** `.PATH` already locates
`lfs_cksum.c` and `misc.c`; the `-I` is redundant, which is why they
dropped it. Deleting a dead flag beats rewriting it, and that is
precisely the splice this audit exists to find.

### 34. `e7c4c061` — exact

NetBSD 1.1 `sbin/routed/trace.c:62` reads `static int iftraceinit
__P((struct interface *, struct ifdebug *));` -- character-identical
to what landed. Lines 63-64 are the `dumpif`/`dumptrace` prototypes
the note says it declined. NetBSD has no `XNSrouted` (404). All three
claims hold.

### 36. `3a30baa0` — citations hold, the donors' convention was not taken

Verified: both donors write `SRCS= gtty.c ftime.c stty.c` for the 4.1
sources, and NetBSD 1.0 names `cfree.c lsearch.c regex.c rexec.c` for
4.3.

But both record the absences in a one-line idiom:

	# compat 4.1 sources
	# missing: getpw.c tell.c vlimit.c vtimes.c
	SRCS=	gtty.c ftime.c stty.c

Ours writes an eight-line prose note instead, naming the missing
files inside it. The information survives; the form does not. The
donors invented a one-line convention for exactly this situation and
it was passed over, in a project whose premise is adopting the
lineage's own forms. **Worth adopting tree-wide** -- `missing.md`
records many such absences and none of them is marked at the site.

Also: `NOPIC= nopic` appears in **both** donors' `libcompat/Makefile`.
That is the second time `NOPIC` has turned up as something the donors
carry and this tree does not; the first was `4fc8837e`'s `libl`.
Worth checking whether it matters here rather than assuming it is as
inert as it was there.

### 39. `cd2df4dd` — exact

OpenBSD 1996 `lib/libc/rpc/svc_udp.c:59-60` is character-identical to
what landed. NetBSD 1.0 and FreeBSD 2.0.5 both keep the declarations
inside the functions at `:180` and `:214`, as the note says.

### Batch 4 observation

Ten commits, all opened on the second pass. Seven splices into
Berkeley's files, three bootstrap or documentation.

**The method failure is the finding.** The first pass marked ten
verdicts having opened five, grouping the unread ones with the read
ones by resemblance. Every one of the five skipped contained
something: a false citation (`1e98a429`), an unadopted convention
(`3a30baa0`), a second `NOPIC`. **Resemblance between commits is not
evidence about either. No commit gets a verdict unopened.**

Four classification corrections across four batches, all in the same
direction: changes that looked Linux-specific turn out to be what the
contemporaries did for their own native builds. Check the donor
before calling anything host-driven.

---

## Corrections owed, running list

| commit | correction |
|---|---|
| `bc09abe1` ldexp | donor is **NetBSD 1.0** (Oct 1994), whose constraints are identical; only its invalid `: "u"` clobber was dropped. The commit credits OpenBSD 1996, two years later |
| `4fc8837e` libl | `NOPIC=` dropped from the cited NetBSD 1.0 Makefile without being mentioned |
| `1e98a429` dumplfs | **no donor writes the `CFLAGS` line.** Possible redo: delete the flag as OpenBSD 1996 does rather than rewrite it |
| `3a30baa0` libcompat | the donors' `# missing:` convention was not taken; and `NOPIC= nopic` is in both donors' Makefile |

---

## Batch 5 — commits 41 to 50, 24 September

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 41 | `44eb87a9` `${LIBRPC}` is librpc.a | no | — | **keep** — verified |
| 42 | `c80d57ef` include/Makefile isofs | no | already a splice | **keep, citation wrong** |
| 43 | `8cd784da` sysroot.sh librpc | n/a, bootstrap | n/a | **keep, bootstrap** |
| 44 | `b52de00b` zopen.c import | no | no target | **keep** (see import survey) |
| 45 | `6cf0849b` savecore `.PATH` | no | — | **keep, citation wrong** |
| 46 | `24518c8e` sysroot.sh libcurses | n/a, bootstrap | n/a | **keep, bootstrap** |
| 47 | `a59e0563` ps, w time formats | **removes a hack** | already a splice | **keep** |
| 48 | `d96ec787` tip static helpers | no | already a splice | **keep** — verified exactly |
| 49 | `86455ce0` fpr gettext | no | already a splice | **keep, donor off by a release** |
| 50 | `e2598954` netstat ns_nfiles | no | already a splice | **keep** — verified |

### 41. `44eb87a9` — exact

`${LIBRPC}` was `sunrpc.a`, a library this tree does not build.
Verified: `lib/librpc/rpc/Makefile:3` reads `LIB= rpc`, and all six
Makefiles naming `${LIBRPC}` link `-lrpc` -- `portmap`, `showmount`,
`mount_nfs`, `nfsd`, `umount`, `mountd`. Neither NetBSD 1.0 nor
OpenBSD 1996 has a `LIBRPC` line; both carry `LIBRPCSVC`, RPC having
moved into libc. "None of them has a line to copy" holds.

### 42. `c80d57ef` — citation dated four days wrong; the real precedent is stronger

The note says OpenBSD "adds scsi and gnu/ext2fs to the nine here".
OpenBSD at 1 November 1996 reads:

	LDIRS=	dev net netinet netccitt netiso netns netipx nfs sys ufs vm ddb scsi

`gnu/ext2fs` is not there. It entered on **1996-11-05** -- four days
later, paired with `scsi` exactly as the note says -- and left again
on **1997-05-30**, the pair removed together. So the note describes a
real OpenBSD state in a real four-day window, just not the 2.0
release. A dating slip, not a fabrication.

The stronger precedent was missed: **OpenBSD master's `LDIRS`
contains `isofs` itself**, the very directory this commit adds.

### 45. `6cf0849b` — citation twenty years out

The note says `zopen.c` now sits beside `savecore.c`, "which is where
OpenBSD keeps its own copy."

In OpenBSD 1996, `zopen.c` is at **`usr.bin/compress/zopen.c`** --
exactly where 4.4BSD-Lite2 expects it, through the `.PATH` this
commit deletes. `sbin/savecore/zopen.c` was added on **3 September
2016**, with the message: "deraadt found that savecore also uses a
source file from compress. add (6.0 versions) of the needed files to
permit progress elsewhere."

The change is right -- this tree has no `usr.bin/compress` to `.PATH`
into, verified: `usr.bin/Makefile` names `compress` in SUBDIR and no
such directory exists. But the precedent offered is twenty years off,
and OpenBSD's contemporary answer was to keep the directory.

### 47. `a59e0563` — a Berkeley hack removed, with Berkeley's own comment as evidence

Berkeley wrote `static char fmt[] = __CONCAT("%l:%", "M%p");` with the
comment `/* I *hate* SCCS... */` -- the split stops SCCS expanding
`%M%` as a keyword.

Tested rather than asserted:

	gcc -std=gnu89:   error: pasting ""%l:%"" and ""M%p"" does not
	                  give a valid preprocessing token
	-traditional-cpp: static char fmt[] = "%l:%" "M%p";

`sys/cdefs.h:59` defines `__CONCAT(x,y) x ## y` under `__STDC__` and
`:75` defines `x/**/y` otherwise. The string pasting works only under
the **non**-ANSI arm.

And `cdefs.h:54-55` carries Berkeley's own claim:

> `__CONCAT` can also concatenate double-quoted strings produced by
> the `__STRING` macro, but this only works with ANSI C.

**That comment is false and backwards.** Under ANSI it is exactly
what fails. Berkeley documented an assumption the standard does not
support, and it held for twenty years only because nobody compiled
the tree as ANSI. A clean instance of the hack definition, with the
tree's own documentation as the evidence.

### 49. `86455ce0` — donor is NetBSD 1.3, not 1.4

Walked the releases:

	netbsd-1-2   132: gettext();   218: gettext()      no declaration
	netbsd-1-3    90: void gettext __P((void));        present
	netbsd-1-4    90: void gettext __P((void));        identical

The line is identical in 1.3 and 1.4, so nothing about the code is
wrong, but under `precedent.md` rule 5 the donor is the earliest
release carrying it. **NetBSD 1.3.**

### 48/50 — verified exactly

`d96ec787`: NetBSD 1.2 and 1.3 have no file-scope prototype in
`aculib/biz22.c`; 1.4 does. "The earliest release with them at file
scope and prototyped" is literally true.

`e2598954`: NetBSD 1.6 has no `ns_nfiles`; trunk has it. Worth noting
this is a genuine Berkeley defect rather than compiler fashion --
`unix.c` defines `KERNEL` before `<sys/file.h>` to get `struct file`,
and that block declares `extern int nfiles`, so Berkeley's `static
int nfiles` is a static declaration following a non-static one of the
same name. Older compilers allowed it; it was always wrong.

### Batch 5 observation

Ten commits, all opened. No code redo. Two citation corrections.

---

## The pattern worth fixing once, not seven times

**Four of the seven corrections owed cite a modern tree in an idiom
that reads as the contemporary:**

| commit | cites | actually |
|---|---|---|
| `1e98a429` | "OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile that way" | no donor writes the `CFLAGS` line at all |
| `c80d57ef` | "OpenBSD ... adding scsi and gnu/ext2fs" | a four-day window in Nov 1996, not the release |
| `6cf0849b` | "where OpenBSD keeps its own copy" | since 2016; in 1996 it was in `usr.bin/compress` |
| `86455ce0` | "NetBSD 1.4's, the earliest release with it" | NetBSD 1.3 |

`3edd3cdc` already carries a self-correction for exactly this, and
`6f0d9b44` is titled "Revision: date the lorder citation, which
pointed at the wrong decade". **The branch knew about this failure
mode and it kept recurring**, because nothing made dating mandatory.

Proposed, in place of seven separate fixes: **every citation names a
dated release**. "OpenBSD" alone is not a citation; "OpenBSD 2.0
(1996)" or "OpenBSD master (2026)" is. `precedent.md` should say so,
and the seven corrections become one convention plus a sweep.

---

## Corrections owed, running list

| commit | correction |
|---|---|
| `bc09abe1` ldexp | donor is NetBSD 1.0, not OpenBSD 1996 |
| `4fc8837e` libl | `NOPIC=` dropped unmentioned |
| `1e98a429` dumplfs | no donor writes the `CFLAGS` line; OpenBSD 1996 deletes it. Possible code redo |
| `3a30baa0` libcompat | donors' `# missing:` convention not taken; `NOPIC= nopic` in both |
| `c80d57ef` include/Makefile | `gnu/ext2fs` is 1996-11-05, not the 2.0 release; `isofs` in OpenBSD master is the better precedent |
| `86455ce0` fpr | donor is NetBSD 1.3, not 1.4 |
| `6cf0849b` savecore | "where OpenBSD keeps its own copy" is 2016; in 1996 it was `usr.bin/compress/zopen.c` |

---

## Batch 6 — commits 51 to 60, 24 September

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 51 | `1da402c0` make.sh object dirs | no — shortcut verified sound | n/a, bootstrap | **keep, bootstrap** |
| 52 | `02feb631` usr.bin SUBDIR | no | no donor list exists | **keep** |
| 53 | `dd3de94e` docs: what the tree names | n/a, docs | n/a | **keep** |
| 54 | `b748de94` kvm_i386.c import | no | no splice target | **keep** (import survey) |
| 55 | `2e57a473` libkvm cast-as-lvalue | **removes a hack** | already a splice | **keep** — verified |
| 56 | `bb24d2e4` `${LIBTERM}` | no | — | **keep, count wrong** |
| 57 | `7dc30e7d` sysroot.sh libm, libkvm | n/a, bootstrap | n/a | **keep, bootstrap** |
| 58 | `4efbc14d` kdump ioctl headers | no | already a splice | **keep** — verified |
| 59 | `0380029e` kdump syscalls.c | no | already a splice | **keep** — verified |
| 60 | `b9a2b44f` i386 `register_t` | no | **spliced from this tree's own ports** | **keep** — the best shape in the batch |

### 55. `2e57a473` — a cast used as an lvalue

Berkeley wrote `(char *)cp += cc;`. A cast is not an lvalue in C; this
was a GCC extension, gone since GCC 4.0. **A hack by the definition,
and Berkeley's own.**

Verified: OpenBSD 1996 `lib/libkvm/kvm.c:917` and NetBSD 1.1 `:512`
both read `cp = (char *)cp + cc;` -- character-identical to what
landed, from two independent trees.

### 56. `bb24d2e4` — the count is wrong by a factor of four

The note leaves the variable named `LIBTERM` rather than renaming it
to `LIBTERMCAP` as the donors did, "since six Makefiles in this tree
use `${LIBTERM}`." Measured:

	files using ${LIBTERM} as a variable      26
	  excluding contrib/                      25

`usr.bin/msgs`, `usr.bin/more`, `libexec/telnetd`, `old/more`, and
**twenty-one games**. The games were not counted. The decision is
unaffected and in fact better supported -- twenty-five is a stronger
reason to leave the name alone than six -- but a measured claim was
not measured.

Everything else verifies: `lib/libterm/Makefile:3` reads `LIB=
termcap`; NetBSD 1.0 `bsd.prog.mk:33` and FreeBSD 2.0.5 `:56` both
carry `LIBTERMCAP?= ${DESTDIR}/usr/lib/libtermcap.a`.

### 58/59 — verified exactly

`4efbc14d`: FreeBSD 2.0.5's `mkioctls` ends
`' $DESTDIR/usr/include/sys/ioctl.h $DESTDIR/usr/include/sys/ioctl_compat.h`
-- character-identical. The note is honest that NetBSD 1.0 and
OpenBSD 1996 took the larger route of passing paths as arguments,
"which changes both files rather than one line."

`0380029e`: OpenBSD 1996 `kdump.c:87` is
`#include "../../sys/kern/syscalls.c"`, exactly the line that landed;
NetBSD 1.4 the same at `:88`. The donors extend the pattern to seven
more compat syscall tables, so `../../sys` is their settled
convention. Path arithmetic confirmed by `ls` from
`usr.bin/kdump`.

### 60. `b9a2b44f` — the best-shaped commit in the batch

`register_t` is defined for hp300 and sparc and not i386.
`sys/hp300/include/types.h:65` and `sys/sparc/include/types.h:76` both
read `typedef int32_t register_t;`, identical and in the same place,
and the line was taken from them.

**NetBSD 1.0's i386 `types.h` does not define it either**, so there
was nothing of the era to take, and the note says plainly the line is
"this tree's own". One line moved between Berkeley's own ports to fill
a gap in one of them: the most genetic answer available to a gap, and
the right model for every similar case.

Small incompleteness: the note lists `<sys/ktrace.h>`, `<sys/systm.h>`
and `usr.bin/kdump` as the users. A sweep finds a fourth --
`sys/isofs/cd9660/cd9660_vnops.c:985`, `register_t *a_retval;`. The
verdict is unaffected; the enumeration is not complete.

### 51. `1da402c0` — a shortcut, checked rather than trusted

The loop skips directories whose Makefile sets `NOOBJ`, and the
comment admits the method: "grepping for it is enough here and costs
one pass rather than a make per directory."

Checked whether that is true of this tree: **12 Makefiles contain
`NOOBJ` and all 12 have it at line start.** None sets it indented,
conditionally, or through an include. Sound as written, and the
comment is honest that it is a shortcut rather than a general
solution.

### 52. `02feb631` — and the `set -e` consequence resurfacing

The donors have nothing to offer here and the note says so: "NetBSD
1.0 and FreeBSD 2.0.5 removed this directory and build grep as one
program, so neither has a list to copy."

It also records a mechanical reason for deleting the entries rather
than commenting them out: "Each one ends its entry with `can't cd',
and the loop's status is the last entry's, so the whole directory
failed after building what it has." That is the same `set -e`
behaviour commit 1 of this branch introduced, surfacing as a
consequence twenty-one commits later.

### Batch 6 observation

Ten commits, all opened. No code redo. One corrected count.

`b9a2b44f` is the model the rest of the audit should measure against:
a gap filled from this tree's own other ports, with the donors
checked and found to have nothing, and the note saying plainly that
the line is ours.

---

## Batch 7 — commits 61 to 70, 24 September

Claims below are marked **verified** (checked against a tree or run
here), **confirmed-here** (true on this host; the general claim is
wider than what was tested), or **unverifiable** (nothing in a tree
can settle it).

| # | commit | hack? | splice? | verdict |
|---|---|---|---|---|
| 61 | `36c29877` Kerberos optional | no | already a splice | **keep** — reasoning sound |
| 62 | `b4b737d6` ftp, systat cmdtab | no | already a splice | **keep** — but see `242b5b8a` |
| 63 | `6d2429a6` mail, window drop `-R` | no | already a splice | **keep** — verified in four trees |
| 64 | `242b5b8a` docs correction | n/a, docs | n/a | **keep** — the branch auditing itself |
| 65 | `97cf3278` window `rub` | no | already a splice | **keep** — verified exactly |
| 66 | `6698fb08` vmstat names.c | no | already a splice | **keep** — the best-reasoned deferral on the branch |
| 67 | `0079efe9` more copyright string | **fixes a build failure** | no donor, and none claimed | **keep** |
| 68 | `3aca1616` mklocale `ldef.h` | no | this tree's own idiom | **keep** — verified |
| 69 | `fd425062` regex.c import | no | no splice target | **keep** (import survey) |
| 70 | `79ebe5ea` include/Makefile miscfs | no | already a splice | **keep, but propagates a bad citation** |

### 63. `6d2429a6` — `-R` verified across four trees

	4.4BSD-Lite2    CFLAGS+=-R
	NetBSD 1.0      CFLAGS+=-R -DUSE_OLD_TTY      still has it
	FreeBSD 2.0.5   CFLAGS+=-DUSE_OLD_TTY         dropped -R, kept the rest
	OpenBSD 1996    (no CFLAGS line at all)       dropped the line

Exactly as the note says, all four. **Confirmed-here:** `gcc -R` on
13.3.0 gives `error: unrecognized command-line option '-R'`. The
note's "no version of GCC has ever had it" is wider than anything
testable here.

### 66. `6698fb08` — the best-reasoned deferral on the branch

Verified: OpenBSD 1996's `usr.bin/vmstat/` holds `Makefile`,
`dkstats.c`, `dkstats.h`, `vmstat.8`, `vmstat.c` -- no `names.c`.
NetBSD 1.2 has `dkstats.c` and no `names.c` (200 / 404). FreeBSD kept
`names.c` through 2.2.0 and dropped it by 3.0.0 (200, 200, 404). All
three lineage claims exact.

It takes NetBSD's **stub**, knowingly accepting `??0`, `??1` drive
labels, and records in `missing.md` what it declined and the
condition under which the better answer becomes available: FreeBSD
2.0.5's real implementation "depends on a kernel symbol this tree has
not been shown to export, which cannot be checked until a kernel
built from this tree runs."

That is the correct handling of an unverifiable donor -- take the
lesser thing, name the better one, state the condition. The condition
is Tier 1 of `roadmap.md`.

### 67. `0079efe9` — a build failure, and no donor exists

	 char copyright[] =
	 "@(#) Copyright (c) 1988 Mark Nudleman.\n\
	-@(#) Copyright (c) 1988, 1993
	+@(#) Copyright (c) 1988, 1993\
	 	Regents of the University of California. ...

Verified it is a failure and not a diagnostic:

	nl.c:1:12: warning: missing terminating " character
	nl.c:2:9: error: missing terminating " character

**Correction to this audit's own first pass:** I wrote that "every
tree that still has the file fixed it or rewrote it". Neither half is
true. OpenBSD 1996 has no `usr.bin/more/main.c` at all -- the
directory is `Makefile`, `more.1`, `more.c`, `more.help`,
`pathnames.h`. NetBSD 1.0 has `main.c` but a different, shorter
Berkeley copyright block whose continuations are correct throughout;
it never had the three-line form and so never had the defect.

**No tree fixed it, because no tree had it.** The broken line is
Lite2's own and the fix is this project's. The commit claims no donor,
which is right; the audit's summary invented one.

### 62/64. `b4b737d6`, and the branch out-auditing the audit

`b4b737d6` credits NetBSD 1.6 with keeping `cmdtab`'s declaration in
`ftp_var.h`. `242b5b8a` had already corrected this to NetBSD 1.5.
Verified:

	NetBSD 1.4   ftp_var.h absent   extern.h 1 occurrence
	NetBSD 1.5   ftp_var.h:310      extern.h 0

**1.5 is the earliest, as `242b5b8a` says.** This audit's own check
had stopped at 1.6, confirmed the original claim, and would have
missed the better answer the branch had already found.

`242b5b8a` also settled the splice alternative before it was asked:
moving `#include "extern.h"` below the struct rather than moving the
declaration. "Not one tree of the fourteen does that... It compiles,
and that is the whole of the case for it." And it found that `mail`
and `window` were one act rather than two -- Lite2's `CFLAGS` line
held `-R` alone, having already lost the `-DUSE_OLD_TTY` that
4.4BSD-Lite and NetBSD 1.0 carry, so dropping the flag left an empty
line.

**Unverifiable:** its claim of a sweep "across all fourteen trees" is
about work done outside the repository. Nothing in a tree settles it;
it can only be trusted or redone.

### 70. `79ebe5ea` — the first propagated error

`include/Makefile:41` still reads "and gnu/ext2fs to the nine here."
The `c80d57ef` citation error found in batch 5 was **carried forward
verbatim** when this commit extended the same note for `miscfs`.

This is the strongest argument for fixing the citation corrections
now rather than at the end of the audit: **a loose claim in a note
gets copied the next time anyone edits near it.**

### 61. `36c29877` — opened late, and sound

Verdicted "it stands" before being opened -- the third time in this
audit that a verdict preceded a reading. The verdict happens to be
right.

The point that looked like an unstated reduction: `-DENCRYPTION`
moves inside the `KERBEROS` guard, which would disable telnet
encryption generally and not only Kerberos. The message covers it at
line 25, and the tree bears it out: `encrypt.c` is 1000 lines with
`#ifdef ENCRYPTION` at line 58 wrapping essentially all of it; its
only other users are `auth.c:149` and `kerberos5.c:356,469`; and the
DES cipher behind it is `enc_des.c`, itself inside the guard. Without
`KRB4` it would compile a framework with no cipher. The pairing is
correct.

OpenBSD 1996's `lib/libtelnet/Makefile:6` has the same base `SRCS`
and no `ENCRYPTION` or `KRB` lines at all.

### Batch 7 observation

Ten commits, all opened. No code redo. One propagated citation
(`79ebe5ea` inheriting `c80d57ef`'s), and one correction to this
audit's own text (`0079efe9`).

Three commits in this batch -- `6698fb08`, `97cf3278`, `b4b737d6` --
walk several releases and name which has what, rather than citing a
tree flatly. That is the dating discipline proposed after batch 5,
already practised here. The citation errors cluster in the commits
that did not do it.

---

## Batch 10 — commits 91 to 100, the kernel phase begins

All three count claims in the i386 code changes verified exactly --
the first batch where every enumeration held.

| # | commit | verdict |
|---|---|---|
| 91 | `adb8a806` LINK.i386 added | **keep** -- ALL, DISKLESS, GENERICAHA confirmed in NetBSD 1.0 |
| 92 | `4326a65b` driver research | **keep** -- seven sites in wd.c, two in fd.c, none in wt.c: exact |
| 93 | `a6055c95` if_ether.c `time` | **keep** -- the five remaining files named exactly |
| 94 | `3ef48144` pmap `/* static */` | **keep** -- hp300 and luna68k each 6, sparc 0: exact |
| 95 | `5428e935` vm headers | **keep**, count off by one (93 not 92) |
| 96 | `93fe6eec` newvers.sh here document | **keep** -- FreeBSD 4.0 earliest: exact |
| 97 | `03528be6` sendsig `u_long` | **keep** -- every port's type verified |
| 98 | `dcb04936` register sizes | **keep** -- 6 correct, 4 slipped: exact |
| 99 | `a50b8e5a` `-Ttext` | **keep** -- OpenBSD 1996's line quoted character for character |
| 100 | `12a85ca2` `_C_LABEL` sweep | **keep**, counts wrong (232 not 226; NetBSD 1.4 has 266 not 222) |

### Why this batch's counts are better

These are the first changes where **the tree's own other ports are
the donor**, and the commits reach for them first. `3ef48144` takes
hp300's and luna68k's `/* static */` over NetBSD's (which drops
`static`) and FreeBSD's (which moves forty lines). `dcb04936` says
outright "this is the file's own spelling rather than another tree's"
-- six correct sites in the same file settle it, no donor needed.

When the evidence is in the file being edited, it gets counted.

### 97. `03528be6` -- and two ports left knowingly broken

	signalvar.h:167  void sendsig __P((sig_t, int, int, u_long code));
	hp300  u_long    sparc     u_long    i386  u_long (fixed here)
	luna68k unsigned news3400  unsigned            <- still disagree

Exact, including the forward-looking half. Two ports still carry
`unsigned` and "will want the same change when they are built" -- a
real latent defect recorded at the site, in ports `roadmap.md` says
have never been compiled.

`a6055c95` and `5428e935` do the same: fix what the configuration
reaches, name what it does not. Five `time`-without-`volatile` files
and `luna68k/luna68k/mem.c` respectively.

### 100. `12a85ca2` -- wrong numbers around a right proof

	claimed   353 references: 226 locore.s + 127 icu.s
	measured  at that commit: 232 + 127 = 359
	claimed   NetBSD 1.4 has 222 _C_LABEL uses
	measured  266

But the claim the commit rests on is the right kind:

> the result was proved rather than inspected: with `_C_LABEL`
> defined the a.out way the converted source assembles to an object
> byte for byte identical to the original's, 15084 bytes

Attempted to reproduce. The converted source assembles to **15024
bytes** here -- but `vector.s` is generated by `config`, and the
current one is post-`3201007b` and emits `_C_LABEL(...)`, so the
pre-conversion `locore.s` cannot even assemble against it. The exact
object depends on a `vector.s` from that era that cannot be
reconstructed without re-running that era's `config`. **15084 can
neither be confirmed nor refuted here.** The mechanism is verified:
the `#ifdef __ELF__` / `#else _/**/x` pair does let the converted
source be assembled both ways, which is what makes the proof
possible.

---

## Batch 11 — commits 101 to 110

| # | commit | verdict |
|---|---|---|
| 101 | `3201007b` vectors through `_C_LABEL` | **keep** -- every donor claim exact; understates NetBSD by a release |
| 102 | `69e450bd` last a.out names | **keep** |
| 103 | `983934d4` console is pc0 | **keep**, history misdated |
| 104 | `7935854e` `OBJECT_FMT` | already corrected by `56ada15e` |
| 105 | `b8596dbf` the marker test | **keep** -- seven of seven verified |
| 106 | `7cb61884` kinds A-G | not yet opened |
| 107 | `46e0f065` Kerberos revision | **keep** -- correction applied at all five sites |
| 108 | `741c7a27` .gitignore | **keep** -- fixes a pattern that never matched |
| 109 | `291dffc7` the 1993 merge | **the most important finding on the branch**; count corrected |
| 110 | `5a0ade80` mixed ancestry | not yet opened |

### 109. The 1993 merge, verified and recounted twice

`3298e079f31`, **11 June 1993**, Chris G. Demetriou, "update with
newer changed from NetBSD", touching `i386/i386/locore.s` and
`i386/i386/machdep.c` and nothing else. Verified in the CSRG history,
where before it rested on the commit message.

The segment loads come in `%ds`/`%es` pairs:

	before   8 pairs, every one movw, no movl anywhere
	after    5 pairs -- 3 movw, 2 movl

**Four lines changed instruction. Six no longer exist in that form**,
dissolved into the restructuring that merge also did; the file grew
from 1584 lines to 1679. The byte store is one site: `movb %al,(%edx)`
became `movb %eax,0(%edx)`.

`291dffc7` said "eight movw %ax,%ds became five movw and two movl",
counting only the %ds half, and it does not add up. **This audit's
first attempt at correcting it said sixteen lines became six, ten
sites changed -- which counted the deletions as conversions and was
wrong in the other direction.** Neither was run against the file.
Corrected in `0e61db4b`.

The argument does not depend on the count and is untouched: Berkeley's
file used `movw` throughout and no `movl` at all; a merge from a
descendant introduced the `movl`.

**What follows is the rule that matters for the rest of the kernel.**
"NetBSD 1.0 writes it this way" is not independent confirmation
anywhere in `sys/i386`, because Berkeley took NetBSD's changes into
their own port two years before this release. Before citing NetBSD
there, check whether the text predates `3298e079f31`. Where it does
not, Berkeley's pre-merge file is the better authority and NetBSD is
the source rather than a witness.

**This is the trap the `copyout` error fell into**: five trees
counted as agreeing, when what they shared was 386BSD's lineage and a
decision about which CPUs to support.

### 103. `983934d4` -- right conclusion, misdated history

Verified exactly: ARGO and BLITZ both read `device pc0 at isa? port
"IO_KBD" tty irq 1 vector pcrint`, character for character the line
adopted. GENERIC.i386 and CIRCE both still say `cn0`/`cnrint`.

But the note says a "1992 reorganisation" moved the driver to
`pccons.c` as `optional pc device-driver`. `files.i386` in **May
1991** already reads exactly that, identical in 1992, and `pccons.c`
was created **24 April 1990**. There was no 1992 rename.

So `cn0` was not left behind by a move -- it was **already stale in
1991**, naming a driver `files.i386` had not declared for at least a
year. The defect is older and plainer than the note says, and the
correction is better supported for it.

The commit also adds to `precedent.md` that the CSRG history is
read-only on XNU's terms, "most of what it holds is the encumbered
4.4BSD that the USL settlement removed". Right rule, third policy the
branch has written for itself, and it correctly anticipates the
licensing hazard of having the full CSRG tree to hand.

### 105. `b8596dbf` -- the strongest document on the branch

Every file in Berkeley's tree carries a redistribution marker; the
Lite releases kept `%sccs.include.redist.c%` and dropped
`%sccs.include.proprietary.c%`, with `.man` and `.roff` variants.
Sampled against the CSRG history, **seven of seven exact**:

	ed.c      proprietary.c      m4.1     proprietary.man
	bc.y      proprietary.c      ching.6  proprietary.roff
	regex.c   proprietary.c      rexec.c  redist.c
	                             zopen.c  redist.c

`kvm_i386.c`: **zero commits** in the CSRG history -- never written,
not removed. `sys/vm/lock.h`: absent from the Lite2 snapshot, present
in the 4.3 Net/2 and 4.4 ones, so removed by Berkeley between 4.4 and
Lite2 and not by the settlement; and `simple_lock_data_t` is now at
`sys/vm/vm.h:62` and nowhere else, confirming `5428e935`.

It turns "why is this missing" from an argument into a lookup. And it
carries a finding easy to miss: **`zopen.c` is marked redistributable
and is still absent**, because `usr.bin/compress` was removed whole
for the LZW patent rather than for its licence -- a different reason
from everything else in `missing.md`, and why `b52de00b` was needed.

### 101. `3201007b` -- exact, and understates itself

	OpenBSD 1996 mkglue.c           absent        exact
	FreeBSD 2.0.5 vector.s          298 lines     exact to the line
	FreeBSD 2.0.5 register_intr     7 occurrences exact
	NetBSD 1.2 mkglue.c             404           exact
	FreeBSD 4.0 mkglue.c            404           exact
	NetBSD 1.1 mkglue.c             404           **also gone**

The note credits NetBSD 1.2 with deleting the generator; **1.1 had
already deleted it** -- the same release it credits with the static
`vector.s`. NetBSD replaced and deleted in one move, a release
earlier than recorded, four months after Lite2 shipped.

The case for replacing the build-time vector table is therefore
stronger than `roadmap.md` Tier 2 records: every tree abandoned it
before any of them went ELF.

---

## The counting problem, stated properly

Thirteen miscounts now, **three of them this audit's own**. They
straddle `7e626c7c`, because that rule governs citations and says
nothing about enumerations.

The rule I first proposed -- *a figure in a note is a measurement;
run it* -- **would not have caught my own error.** I did run a grep.
It returned 16 and 6, and I wrote them down without asking whether
16 - 6 = 4 made sense.

So it needs both halves:

> **A figure in a note is a measurement. Run it against the tree the
> note ships in, say what command produced it, and check that the
> arithmetic closes.**

The middle clause is what makes it checkable by the next reader:
"353 references, by `grep -c '_C_LABEL('`" can be rechecked in a
second; "353 references" cannot. The last clause is what would have
stopped me: sixteen to six with four conversions is visibly
impossible, and it was published anyway.

## Corrections owed -- revised

The thirteen citation findings are **all** in commits before `#86`,
where `7e626c7c` introduced the dating rule. They are not thirteen
lapses but one, already diagnosed and fixed by the branch, with a
backlog nobody swept. **One sweep commit applying the branch's own
rule retroactively**, not thirteen corrections.

The miscounts are the live problem, and the rule for them does not
exist yet.
