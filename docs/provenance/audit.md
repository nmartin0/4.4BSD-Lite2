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

**And `imports.md` already knew.** Its entry E says the file *"is a
shim that implements re_comp and re_exec in terms of regcomp and
regexec, over the engine already in this tree."* So the record is not
wrong about what the file does; it simply never draws the conclusion
that a shim over our own engine could have been written here instead
of imported.

So this finding is weaker than the other four and is recorded as such.
The outcome is identical -- the file is Berkeley's own copyright and a
correct shim would look like it -- and the only thing missing is the
sentence saying the import was avoidable. Added to `imports.md`
rather than acted on: rewriting a correct 93-line file to avoid an
entry in a list would be churn.

#### `i386 kernel assembly: name C symbols through _C_LABEL` — a
#### citation that launders a donor through my own earlier commit

A third kind, not in the original list and worth adding to it.

That commit says the macro is *"already in this tree's
`lib/libc/i386/DEFS.h` and `SYS.h`, put there for the same reason when
libc was made to link as ELF."* True on 25 September. But `DEFS.h` had
**zero** occurrences of `_C_LABEL` before 23 September, when
`libc/i386: name C symbols through _C_LABEL, for ELF` put seven there
— and that commit says plainly where they came from: *"The block here
is NetBSD 1.5's `<machine/asm.h>`."*

So the kernel change cited an import of this branch's own making as
though it were Berkeley's. Nothing about the code is wrong — this tree
has no i386 `<machine/asm.h>` at all, so there was no inward
alternative — but the citation points at `lib/libc` when it should
point at NetBSD 1.5.

**"This tree already has it" has to mean Berkeley has it, not that an
earlier commit of this branch put it there.** Every other `this tree`
citation in the sweep was checked against that test and passes:
`NKPDE`'s `APDRPDROFF`, `delay()`'s reference to `ldexp.c`, the `LINK`
configuration's `config`, `sys_nerr`'s `gen/errlst.c` and `stdio.h`'s
are all Berkeley's files.

### Clean

| commit | why |
|---|---|
| `Import NetBSD 1.6's i386 ELF startup code, verbatim` | question 3 of the three questions ends it: Lite2's `crt0.c` reads `kfp` uninitialised, clobbers `%ebx` where GCC keeps the GOT pointer, and reads `%ebp` where `-O2` emits no frame pointer. Splicing onto unsound code is the wrong trade, and the commit says so |
| `libkvm: a cast is not an lvalue` | forced; the construct is invalid C and no tree accepts it |
| `librpc: declare cache_get and cache_set at file scope` | forced; `static` inside a function body |
| `libc: declare four static helpers at file scope` | forced; same |
| `libc: resolve sys_nerr the way this tree's own files do` | sourced inward, to `gen/errlst.c`, which is Berkeley's |
| `lib/csu/i386_elf: a Makefile in Lite2's own idiom` | the title says it |
| `Revision: makelist's sed line was not NetBSD 1.1's after all` | already a correction of a citation |
| `libc/i386: ldexp's inline asm` | already corrected in `REDO.md`: donor was NetBSD 1.0, not OpenBSD |

**Batch 3 result: two findings in eighteen commits** — the `regex.c`
import that was a shim over code already here, and the `_C_LABEL`
citation that points inward at an outward import.

---

---

## Batch 4 — userland (39 commits)

### No findings, and one worth recording anyway

#### `Import zopen.c from NetBSD 1.0 for sbin/savecore` — clean, but
#### the tree does hold a compressor

`savecore` writes compressed dumps through `zopen()`, and its Makefile
names `zopen.c` with a `.PATH` into `usr.bin/compress` -- a directory
Berkeley removed from the Lite releases for the LZW patent while
leaving the `SUBDIR` entry behind.

The sweep's question is whether this tree could have supplied it.
There *is* a compressor here:

	contrib/mh-6.8.3a/miscellany/compress-4.0/compress.c   1407 lines

It is the standalone Thomas and Orost utility with a `main()`, and it
has **zero** `zopen`, `zread` or `zwrite` symbols -- no library
interface at all. Building one over it would be this project's own
invention, where Berkeley's `zopen.c` is Berkeley's own file that the
Lite releases dropped for a patent that expired in 2003. The import is
right.

Recorded because the next person grepping for a compressor will find
`contrib/` and wonder, and the commit does not mention it.

### The rest

The remaining 38 are compiler-conformance fixes forced by GCC, and no
tree's version differs because no version can:

- `static` declared inside a function body -- `librpc`, `tip`,
  `window`, `routed`, `XNSrouted`, four files in `libc`
- a cast used as an lvalue -- `libkvm`
- adjacent string literals and unclosed strings -- `ps`, `w`, `more`
- headers found in the source directory rather than `/sys` --
  `ftpd`, `mklocale`, `kdump`, `lfs_cleanerd`, `bad144`
- Makefile variables naming libraries that exist -- `bsd.prog.mk`'s
  `${LIBTERM}` and `${LIBRPC}`
- Kerberos made optional, which is a configuration decision this tree
  records in `docs/deferred.md` rather than a donor's code

Where a donor is named in these it is as corroboration -- *"every BSD
of the period does this"* -- not as a source. Two are already
corrections of their own citations: `Revision: date the lorder
citation, which pointed at the wrong decade` and `Revision: Kerberos
can be built; we chose not to, and why`.

**Batch 4 result: no findings in thirty-nine commits.**

---

---

## Batches 5 and 6 — `build/`, `share/mk` (8 commits)

**No findings, and none possible.** These are cross-build
accommodations, and 4.4BSD-Lite2 does not cross-build: it assumes the
compiler, the libraries and the headers of the machine it is running
on.

| commit | why this tree cannot supply it |
|---|---|
| `bsd.prog.mk: find libraries under DESTDIR, as NetBSD 1.0 does` | Lite2 has no `DESTDIR`; every library variable names an absolute path on the running system |
| `share/mk: add HOSTCC, for programs the build runs itself` | `sys.mk` has no variable for a program compiled and then run during the build, because Lite2 never needs one |
| `bsd.lib.mk: archive with ar cq, not cTq` | GNU `ar`'s `T` means a thin archive where 4.4BSD's meant truncate names; a host-tool difference |
| `share/mk: end a SUBDIR entry when its cd fails` | a host-shell behaviour, not a donor's code |
| `bsd.prog.mk: ${LIBTERM}`, `${LIBRPC}` | naming libraries that exist in this tree — sourced inward |

---

## Result

	batch            commits   findings
	1  i386 kernel        25      3
	2  sys/ elsewhere      4      0
	3  lib/               18      2
	4  userland           39      0
	5  build, share/mk     8      0
	                     ---    ---
	                      94      5

Ninety-four readings over eighty-seven commits — several touch more
than one area and were read in each.

**Five findings, none of them a code defect.** In every case the code
is right, because the donors and this tree agree about what the code
should be. What is wrong in five places is where the note sends the
next reader.

Four kinds, of which the first three were anticipated and the fourth
was not:

1. **Cited outward, available inward** — the clockframe, `-Ttext`,
   `DELAY`. Three.
2. **Taken wholesale where a splice would serve** — `regex.c`. One.
3. **Guard imported without checking the condition** — `trap()`'s
   `proc0` fallback. Already corrected before the sweep.
4. **Cited inward at an outward import** — `_C_LABEL`. One. The
   kernel change cited `lib/libc`, which had the macro only because
   an earlier commit of this branch took it from NetBSD 1.5.

### Sparc's `delay()` — read, and the answer is no change

The one item flagged for code rather than notes. Read, and sparc's own
comment settles it: *"This is easy to do on the SparcStation since we
have freerunning microsecond timers -- no need to guess at cpu speed
factors... if we calculated a limit, we might overshoot, and precision
is irrelevant here---we want less object code."*

Sparc's timer ticks once per microsecond, so waiting n ticks is
waiting n microseconds. The 8254 ticks at 1193182 Hz, 1.193 per
microsecond, so the conversion is forced by the hardware; and it
cannot be one multiply, because `n * 1193182` overflows a signed int
above about 1800 microseconds where this port's call sites ask for
four seconds. **Sparc's simplicity is a property of its timer, not of
its design.** The note now says so; the code stands.

### A sixth finding, and the largest — `pmap_enter`

Found while scanning the other ports for anything else this tree
already had. It is not a citation error but a piece of work about to
be started from the wrong place.

`i386/i386/pmap.c` has, where a page directory entry is invalid:

	if (!pmap_pde_v(pmap_pde(pmap, va))) {
		pg("ptdi %x", pmap->pm_pdir[PTDPTDI]);
	}
	pte = pmap_pte(pmap, va);

It prints and walks into `pmap_pte` with an invalid entry. hp300 has
the same test in the same position in the same function:

	if (!pmap_ste_v(pmap, va))
		pmap_enter_ptpage(pmap, va);
	pte = pmap_pte(pmap, va);

`pmap_enter_ptpage()` is the missing piece, and OpenBSD 1996's twenty
lines of comment about how hard this is on the i386 -- *"in the m68k
pmap's this is easy since all PT pages live in one global vm_map
(pt_map)"* -- turn out to be **describing hp300's solution**, which is
in this tree. It was read as "nobody solved this".

Not copyable as it stands: hp300's is 243 lines built on m68k's
two-level segment tables, `st_map`, `pmap_ste` and `pmap_ste2`. But
the machinery it rests on is partly here already, and inconsistently:

	hp300/hp300/pmap.c:265   vm_map_t  st_map, pt_map;
	hp300/hp300/pmap.c:466   pt_map = vm_map_create(...)
	i386/i386/pmap.c:1679    vm_map_lookup_entry(pt_map, va, &entry)

**i386 uses `pt_map` and never creates one.** Another thing inherited
from a port that does, like `setconf`'s tahoe body and `crt0.c`'s vax
register name.

CSRG's own history settles it. The Unix History Repo dates hp300's
`pmap_enter_ptpage` to **6 December 1990**, Kirk McKusick, *"adopted
from Mach 2.5"*. The i386 pmap's VM work begins eleven months later --
*"1991-11-14 William Nesheim: changes to fix new vm on i386"* -- and
its last substantive commit is `1993-06-11`, Bostic's 8.1 snapshot.
Nothing after that but release tags.

And in between, this:

	1992-05-11 Keith Bostic: disable pageing in basemem until
	someone understands what's going on; loop variable error;
	from Pace Willison

CSRG saying out loud what this project keeps finding. The i386 pmap
was work in progress that stopped, and the `pg("ptdi")` line is not a
decision -- it is a debugging print standing where the function should
be, in a file nobody touched again.

**And the lineage is closer than "another port".** Both files say
*"contributed to Berkeley by the Systems Programming Group of the
University of Utah"*, the i386's adding *"and William Jolitz of UUNET
Technologies Inc."* The i386 pmap is hp300's Utah/Mach code forked for
the 386 and left unfinished -- which is why the tests match and only
the bodies differ. `docs/provenance/precedent.md` now carries what
follows from that for the whole VM subsystem.

So the next piece of work starts at hp300, at Mach 2.5 behind it, and
at this port's own unfinished references -- not at a donor.

### Formerly: where the code should be revisited

This section listed sparc's `delay()` as the one item needing code
rather than a note. It has been read, and the answer is above: no
change, because sparc's timer counts microseconds and the 8254 does
not.

The other four are citations to correct and nothing more.

### What the sweep says about the method

The failures cluster in one place. All three batch-1 findings and the
`_C_LABEL` one are i386 kernel work, where the pull toward NetBSD and
FreeBSD is strongest because they are the only trees with an i386
port — and that is exactly where this tree's *other* ports still have
the machine-independent half of the answer. `hardclock` takes a
`clockframe` on every port; `DELAY` is defined on every port; a kernel
is linked at an address on every port.

The rule that would have caught all five, stated as narrowly as it
can be: **before citing a donor for anything in `sys/i386`, grep the
other six ports for the same identifier.** Four of the five findings
are one `grep -r` away.

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
