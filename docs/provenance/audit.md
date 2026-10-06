# Where a donor was cited and this tree had the answer

An audit of every commit on `dev3` that cites NetBSD, FreeBSD, OpenBSD
or 386BSD, asking one question of each: **did this tree already have
the thing, in another port or another directory, and was it looked
at?**

`docs/provenance/precedent.md` puts this tree's own ports at the head
of the search order. `docs/method.md` records three occasions where
that order was skipped — `genericconf`'s `caddr_t` field, `wd.c`'s
controller queue, and `trap()`'s pcb — each caught only after the work
was committed. This file is the systematic sweep that follows from
those.

## What counts as a finding

**The code being wrong is not the test.** In every case so far the
code is right, because the donors and this tree agree. What is wrong
is the citation: a note or commit message that sends the next reader
to NetBSD or FreeBSD for something hp300, pmax, sparc or `lib/libc`
already does. That is a defect in the record, and the record is most
of what this project produces.

Three kinds are recorded:

1. **Cited outward, available inward.** A donor named as the source
   for a construct this tree already has.
2. **Taken wholesale, should have been spliced.** Donor code imported
   where adding to Berkeley's own file, or combining what two of this
   tree's ports agree on, would have served.
3. **Guard imported without checking the condition.** A donor's extra
   test taken on the assumption that the case it covers occurs here.

## Method

	git log --format='%H %s' origin/dev3            182 commits
	  of which cite a donor                         149
	  of which touch usr/src and name no port of
	  this tree                                      87   <- the sweep

Each is read against the construct it changes, looking for the same
construct in `hp300`, `luna68k`, `news3400`, `pmax`, `sparc`,
`tahoe`, `vax`, and in `lib/`, `usr.bin/` and `usr.sbin/` where the
change is not kernel code.

---

## Batch 1 — the i386 kernel (25 commits)

### Findings

to those.

Cited FreeBSD 2.0.5's `clkintr(struct clockframe frame)` calling
`hardclock(&frame)`, as though no example existed here.

**pmax and news3400 both do it**, building the frame as a local rather
than relying on the stack layout:

	pmax/pmax/trap.c:832    struct clockframe cf;
	                 :839   cf.pc = pc;
	                 :840   cf.sr = statusReg;
	                 :841   hardclock(&cf);
	news3400/news3400/trap.c:790  the same three lines

The code taken is still correct — this port's interrupt stack really
is a `struct intrframe`, so declaring the parameter by value and taking
its address is right and costs nothing. But the shape was Berkeley's
before it was FreeBSD's.

#### `Makefile.i386: -Ttext for the load address, and an ELF vmunix`

Cited NetBSD 1.0 and FreeBSD 2.0.5 for the `-Ttext` form.

**pmax already writes it**, and sparc writes the `-T` form beside it:

	pmax/conf/Makefile.pmax    ${LD} $$strip -N -o $@ -e start -Ttext 80030000
	sparc/conf/Makefile.sparc  ${LD} ${LDX} -p -N -e start -T f8004000 -o

So both spellings are in this tree, with a port using each, and the
commit's discussion of which to use could have been settled here.

#### `i386: delay() reads the 8254, where DELAY counted a loop`

Cited NetBSD 1.0 and FreeBSD 2.0.5 for the decomposition arithmetic
and OpenBSD for `#define DELAY(x) delay(x)`.

**sparc has both**, and has had since 4.4BSD:

	sparc/include/param.h:138   #define DELAY(n)  delay(n)
	sparc/sparc/clock.c:213     delay(n) { ... c = TIMERREG->t_c10.t_counter;
	                            while (--n >= 0) {
	                                while ((t = TIMERREG->t_c10.t_counter) == c)
	                                    continue;
	                                c = t; } }

A macro over a function, and the function **reads a hardware counter**
rather than spinning a calibrated loop. That is the whole principle of
the change, in Berkeley's own code.

What remains genuinely the donors': the overflow-safe decomposition of
microseconds into counter ticks. sparc's timer is a different device
and counts differently, so its loop cannot be copied literally — but
hp300, pmax, luna68k and news3400 all spin `cpuspeed * n`, so sparc is
the one port that had solved it, and it was not consulted.

### Clean

| commit | why it is clean |
|---|---|
| `i386 kernel assembly: name C symbols through _C_LABEL` | the commit itself says the macro is already in this tree's `lib/libc/i386/DEFS.h` and `SYS.h`. Sourced inward, just not from a port |
| `pccons.c: Ctrl-Alt-Del calls reset_cpu, not _exit` | sourced from `reset_cpu()` twenty-four lines above in the same file, and `i386/i386/machdep.c:578`. 386BSD is corroboration only |
| `icu.s: drop the dispatch to rawintr` | `rawintr` appears in no other port; nothing to compare |

### Not applicable

These change constructs no other port in this tree has, so NetBSD,
FreeBSD and OpenBSD are the only comparators available:

- `i386: the hardware descriptor is eight bytes, so pack it` — x86
  segment descriptors
- `i386: load the GDT and IDT the way locore.s expects` — x86 only
- `i386: call startrtclock, so the 8254 is programmed at all`,
  `i386: remove the calibration DELAY no longer reads` — the 8254
- `i386: the kernel's pages need the write bit, now that WP is on`,
  `i386: the kernel moves to 0xF0000000`, `i386: NKPDE...`,
  `i386: locore.s takes the page directory slots from pmap.h` — x86
  paging and its page directory
- `Revision: copyout needs the 386 path Berkeley wrote for it`,
  `Revision: locore.s restores what Berkeley wrote before a 1993
  merge`, `Revision: what the 1993 merge changed in locore.s` — all
  about Berkeley's own i386 text
- `locore.s: name the registers the size the instruction moves`,
  `config, locore.s: generate and define vectors through _C_LABEL` —
  i386 assembly syntax
- `i386: wd.c's cast used as an lvalue` — a forced change; any tree
  shows the same

### Already corrected before this sweep began

- `i386: trap() must not use curpcb` — corrected by `Revision: hp300
  derives the pcb too, and was the source` and `Revision: the proc0
  fallback was unnecessary, and its reason was wrong`
- `i386: genericconf carries no driver, and the kernel links` —
  corrected by `Revision: genericconf's driver field was never the
  problem`

### Also clean in this batch

| commit | why |
|---|---|
| `Revision: the a.out arm of OBJECT_FMT was never linked` | `OBJECT_FMT` appears in no other port's `conf/`; i386's own |
| `LINK.i386: the console is pc0, as Berkeley's own configs have it` | the title says it: sourced from this tree's own configurations |
| `i386: add a LINK configuration, without the unfinished drivers` | a new file for this port; no other port has a LINK config to copy |
| `docs: record the i386 driver research, and correct wt.c` | documentation of the research itself |

**Batch 1 result: three findings in twenty-five commits.**

---

---

## Batch 2 — `sys/` outside i386 (4 commits)

**No findings.** All four source from this tree already:

| commit | sourced from |
|---|---|
| `vnode.h: vref, vhold and holdrele are void` | the `void vref __P((struct vnode *vp));` declaration further down the same header |
| `vfs_subr.c: define vref, vhold and holdrele only under DIAGNOSTIC` | `<sys/vnode.h>`'s own `static __inline` definitions and its `DIAGNOSTIC` guard |
| `sys/mman.h: include <sys/types.h>, as Lite2's own headers do` | the title says it |
| `newvers.sh: write vers.c with a here document` | a host-shell portability fix; `sys/conf/newvers.sh` is the only one in the tree, shared by every port, so there is nothing to compare it against |

---

---

## Batch 3 — `lib/` (18 commits, in progress)

### Findings

#### `Import regex.c from NetBSD 1.0 for re_comp and re_exec`

Kind 2: taken wholesale where a splice would have served.

The reasoning given was sound as far as it went -- `usr.bin/more` and
`usr.bin/rdist` call `re_comp()` and `re_exec()`, nothing in this tree
defines them, `lib/libcompat` ships only the manual page, and the file
is Berkeley's own copyright with NetBSD 1.0 the earliest to carry it.

What was not checked is what the file *does*. All 93 lines are a shim:

	lib/libcompat/4.3/regex.c:51   #include <regexp.h>
	                        :70    re_regexp = regcomp(s);
	                        :81    rc = regexec(re_regexp, s);

That is the V8 `regexp` interface -- one argument to `regcomp`, two to
`regexec` -- not POSIX. And this tree already has both sides of it:

	lib/libcompat/regexp/regexp.c   the engine
	lib/libcompat/regexp/regexp.h   the header the import includes

So a file was imported to bridge two interfaces already present here,
in the same library directory. The shim could have been written
against our own `regexp.h` and `docs/provenance/imports.md` would
carry one fewer import.

Fairly stated: the outcome is the same. The file is Berkeley's, and a
correct shim would look like it. What differs is the record -- an
import is listed where none was needed, and the next person reading
`imports.md` sees a dependency on NetBSD that this tree does not
have.

### Still to read in this batch

	Import NetBSD 1.6's i386 ELF startup code, verbatim
	crtbegin.c: drop NetBSD's binary identification note
	libkvm: a cast is not an lvalue
	libc/i386: name C symbols through _C_LABEL, for ELF
	libc/i386: ldexp's inline asm, in the form that still compiles
	Revision: makelist's sed line was not NetBSD 1.1's after all
	libedit/makelist: build the include guard from the file's name
	Revision: drop a trailing space from two ENTRY definitions
	Kerberos is optional, as it is in every BSD of the period
	librpc: declare cache_get and cache_set at file scope
	libc: resolve sys_nerr the way this tree's own files do
	lib/libl: add a Makefile for the lex run-time library
	libutil: drop -I/sys from the compile flags
	lib/csu/i386_elf: a Makefile in Lite2's own idiom
	libc: install libc.tags under DESTDIR
	libc: declare four static helpers at file scope
	libc: depend on the target root's syscall.h

---

## Batches not yet started

	4.  usr.bin/, usr.sbin/, bin/, sbin/
	5.  build/ and the bootstrap tooling
	6.  share/mk and the Makefiles

---

## What to do with the findings

Nothing in this file is a code defect. Each entry is a citation to
correct, and they should be corrected in as few commits as the areas
allow rather than one per finding — the point is that the record sends
the next reader to the right tree, not that each mistake gets its own
headstone.

Where a finding shows that Berkeley's own version is not merely
present but *better suited* — sparc's `delay()` is the candidate so
far — the code should be revisited too, not only the note.
