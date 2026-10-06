# Working on this tree

Notes from the sessions that linked and then ran the i386 kernel.
`docs/reference/test_mk7.3/METHODOLOGY.md` is the general method; this
is what is specific to 4.4BSD-Lite2, and most of it is a record of
mistakes that were made and the checks that caught them.

The short version. **Run it, then check the lineage agrees.** The code
decides; donors corroborate. Every serious error recorded below came
from doing those in the other order.

---

## 1. The three questions, in order

Every change to the tree answers these before it is written:

1. **Is it a hack?** Code whose correctness depends on something not
   guaranteed by the language, the ABI or the hardware specification:
   reading uninitialised storage, inline assembly that assumes
   register allocation or omits constraints, depending on struct
   layout or evaluation order that is not specified, a magic constant
   with no derivation, or working around a broken tool instead of
   fixing the tool. Old-fashioned, verbose or slow is not a hack. A
   documented hardware workaround is not a hack. The test is "is this
   guaranteed to work", not "is this pretty".

2. **Would splicing have been more canonical?** Could the result be
   reached by adding to Berkeley's own file, or by combining what two
   donors agree on and writing it in this tree's idiom? A wholesale
   import is the last resort.

3. **Is Berkeley's code sound?** This governs both. If it depends on
   undefined behaviour, the splice question does not arise -- carrying
   a hack forward because it is ours is the wrong trade.

The worked example is `lib/csu`. Berkeley's `i386/crt0.c` does most of
what the imported ELF startup does, so it looked like an obvious
splice. It reads `kfp` uninitialised, clobbers `%ebx` where gcc keeps
the GOT pointer, and reads `%ebp` where `-O2` emits no frame pointer.
Question 3 ends it: the import was right.

---

## 2. What kept going wrong

### Verdicts written before reading

Five separate batches of the `REDO.md` audit were marked complete with
commits unopened, grouped with their neighbours by resemblance. Every
time this was caught, the skipped commits held something: a false
citation, an unadopted convention, a second `NOPIC`.

**No commit gets a verdict unopened. Resemblance between two commits is
not evidence about either.**

### Counts written without running them

Fourteen figures in commit messages and documents did not reproduce.
"six Makefiles" was twenty-five. "twenty-seven files" was thirty-eight.
"353 references" was 359. Three of the fourteen were this audit's own,
including one published inside a `Revision:` commit whose whole subject
was correcting a count.

The rule that would have caught them is not just "measure":

> **A figure in a note is a measurement. Run it against the tree the
> note will ship in, say what command produced it, and check that the
> arithmetic closes.**

The last clause matters. One miscount here came from a grep that *was*
run: it returned 16 and 6, and 16 − 6 = 4 conversions is visibly
impossible, and it was published anyway. Three pairs had been deleted,
not converted.

### Citations without a date

Thirteen citations named a tree with no release, or named a modern tree
in a context that reads as a contemporary -- "where OpenBSD keeps its
own copy" turned out to be a 2016 commit. Every one was in a commit
written before `7e626c7c`, which diagnosed exactly this and made dating
mandatory. The branch found its own failure; the backlog is unswept.

The contemporaries are **4.4BSD-Lite, NetBSD 1.0 and 1.1, FreeBSD
2.0.5, OpenBSD 1996**. An undated citation in this tree reads as one of
those.

### Instrument errors reported as findings

Three times a broken check was reported as a defect in the thing under
test.

- `lorder` "returned 0" -- it was not on `PATH` in that shell.
- `delay(10000)` "crashed the guest" -- it was still running; the
  tracing `puts_` calls were slower than the delay and QEMU hit the
  timeout.
- "the GDT register still reads the wrong base" -- the grep took the
  first `GDT=` in the log, which is SeaBIOS's SMM dump, not the
  exception.

**When a check reports something surprising, suspect the check first.**

---

## 3. The precedent method

### Where to look, in order

`docs/provenance/precedent.md` has the rule. In practice:

1. **This tree's own other ports.** `i386: define register_t, as hp300
   and sparc do' is the model: one line moved between Berkeley's own
   ports to fill a gap in one of them, with the donors checked and
   found to have nothing. `chrtoblk` is the same shape.
2. **This tree's own disabled code.** `#ifdef notdef`, `#ifdef
   cgd_notdef`, `#ifdef garbage`. The CPU detection sat disabled for
   thirty-three years; 386BSD's `copyout` page-walk is still sitting
   there.
3. **The contemporaries**, dated.
4. **Later releases**, stopping at the earliest that has the thing,
   and saying it is later.

### `sys/i386` is special

Berkeley took NetBSD's changes into their own i386 port on **11 June
1993** -- CSRG `3298e079f31`, Chris Demetriou, "update with newer
changed from NetBSD", touching `locore.s` and `machdep.c`. So **"NetBSD
1.0 writes it this way" is not independent confirmation anywhere in
that directory**: it may be the same text arriving by another route.

Before citing NetBSD there, check whether the text predates that
commit. Where it does not, Berkeley's pre-merge file is the better
authority and NetBSD is the source rather than a witness.

This is the trap the `copyout` deletion fell into: five trees counted
as agreeing, when what they shared was 386BSD's lineage and a decision
about which CPUs to support. The warning was already in the tree.

### The donors' apparently redundant details are usually load-bearing

Three times in one session a detail that looked like belt and braces
turned out to be the thing that works:

- `-fno-toplevel-reorder` in the csu Makefile. Without it gcc collapses
  the ordered `.init` fragments and `_init` calls itself.
- `ldexp`'s `"=t"` and `"u"` x87 constraints. NetBSD 1.0's extra `: "u"`
  clobber is invalid and gcc rejects it; the constraints themselves are
  the same in both donors.
- `__attribute__ ((packed))` on `rd_base`. NetBSD's bare bitfield does
  not pack on gcc 13 and the padding returns.

**Ask why they would have written that.** It has paid better than
reasoning about what ought to work.

### Read the donor before designing anything

The rule above is about details. This one is about whole approaches,
and it has been broken more often than any other rule in this file.

Moving `KERNBASE` is the clearest case. The constant is written out
in twenty-one places across `sys/i386`, in three unconnected copies,
and the question was how to reduce them to one. Three approaches were
designed and argued for in turn: derive everything from `KERNBASE` in
the headers; include `machine/param.h` from `locore.s` so the
assembler can see the macro; and only then, on the third pass, read
what NetBSD 1.0's `locore.s` actually does.

It computes every address from the page directory slots:

	.set	_PTmap,(PTDPTDI << PDSHIFT)
	.set	_PTD,(_PTmap + PTDPTDI * NBPG)
	.set	_Sysmap,(_PTmap + KPTDI * NBPG)

and gets the slots through `assym.s`, which `genassym.c` writes from
`machine/pmap.h`. One source, no second copy, and `KERNBASE` falls
out as `KPTDI << PDSHIFT` rather than being the thing everything else
derives from. The machinery is already in this tree -- `genassym.c`
exports `UPAGES`, `NBPG` and `CLSIZE` by the same mechanism and
simply does not include `pmap.h`.

A single careful reading of that file at the start would have given
the answer and saved all three designs. The same mistake, in the same
shape, produced the `copyout` error: five trees were counted as
agreeing without reading why.

**So: before designing an approach, read the donor's version of the
same file end to end.** Not grep for the symbol -- read the file. The
answer is usually there, and it is usually smaller than whatever was
about to be designed.

The second case was `trap()` dereferencing a null `curpcb`. Reading
NetBSD 1.0's `trap.c` gave not only the fix but a comment explaining
it -- "can't use curpcb, as it might be NULL; and we have p in a
register anyway" -- and `curpcb` appears exactly once in their whole
file, in that comment. Grep for `curpcb` would have found the five
sites in ours; reading theirs found the reason there are none.

What the failed attempts are good for is diagnosis rather than cure.
Breaking the kernel by moving twelve of the twenty-one constants is
what found `UPTDI`, found `pmap.c:1278`'s hardcoded `for(x=0x3f6; x <
0x3fA; x++)`, and showed that `locore.s` holds a second copy of the
whole map. None of that would have come from reading headers. So
attempt things -- but attempt them after reading the donor, not
instead of it.

### Source from this tree first, and check the guard is needed

Two rules, and they failed together often enough to belong together.

**First.** `docs/provenance/precedent.md` puts this tree's own ports
at the head of the search order. In one stretch of work that order was
skipped three times running, and each time hp300 already had the
answer:

- `genericconf`'s driver field was removed as a type collision,
  following FreeBSD. It is `caddr_t` in hp300, pmax, vax, luna68k and
  news3400, and each casts its own driver in. There was no collision;
  one word of the `extern` was wrong.
- `wd.c`'s controller queue was going to be flattened, following
  FreeBSD. hp300's `sd.c` keeps the per-drive buffer queue in `struct
  buf` and the controller's device queue in its own `struct devqueue`
  in the softc -- two levels, two structures, which is what this
  driver needed.
- `trap()`'s pcb was derived from the process, credited to NetBSD.
  hp300's `trap.c` has no `curpcb` in it at all, and pmax and luna68k
  likewise.

In all three the donors agreed, so the code was right and only the
reasoning was wrong -- which is the dangerous case, because nothing
fails and the next person inherits a citation pointing at the wrong
tree.

**Second, and it is how the third was caught.** A donor's version
often carries a guard or a fallback the tree's own version lacks.
Before taking it, check whether the condition it guards can occur
here.

`trap()` was given NetBSD's `if (p == 0) p = &proc0;` on the reasoning
that `curproc` is null before the first context switch. It is not:
`kern/init_main.c:84` is `struct proc *curproc = &proc0;`, a static
initialiser. What was null was `curpcb`, which is a different variable
set in a different place. One `grep` for `curproc =` would have shown
it, and removing the line changed nothing -- the kernel reaches the
same place.

**So: grep this tree for the condition before importing the guard
against it.** A donor's extra line is evidence that the condition
occurs in *their* tree. Whether it occurs here is a separate question
with an answer in the source.

**Third, from the sweep those two produced.**
`docs/provenance/audit.md` read all eighty-seven commits that cite a
donor without naming a port of this tree, and found five. Four are
i386 kernel work, which is where the pull toward NetBSD and FreeBSD is
strongest, because they are the only trees with an i386 port -- and
exactly where this tree's *other* ports still hold the
machine-independent half of the answer. `hardclock` takes a
`clockframe` on every port. `DELAY` is defined on every port. A kernel
is linked at an address on every port.

So, narrowly: **before citing a donor for anything in `sys/i386`,
`grep -r` the other six ports for the same identifier.** Four of the
five findings are one such grep away.

And one more test, which the sweep turned up and none of the rules
had: **"this tree already has it" must mean Berkeley has it.** The
kernel's `_C_LABEL` change cited `lib/libc/i386/DEFS.h` as this tree's
own precedent; `DEFS.h` had the macro only because a commit of this
branch two days earlier had taken it from NetBSD 1.5. Check the file's
history before citing it as inward.

### When a donor guards repeatedly, check whether they missed one

A fault can be fixed structurally, so it cannot recur, or by guarding
each place it shows up. Where the donors split on that, the ones who
guarded are worth counting rather than reading.

`curpcb` in `trap()` is the case. It is null until the first context
switch, and `initclocks()` enables interrupts before that. NetBSD 1.0
and OpenBSD 1996 both stopped using it in `trap()` and derived the
pcb from the process. FreeBSD 2.0.5 kept it and guarded each use --
`if (curpcb && curpcb->pcb_onfault)` at its `trap.c:321`, `:496` and
`:606` -- **and missed one, at `:426`**. 4.4BSD-Lite2 guarded one of
five.

So two trees guarded and both have a bare dereference left. That is
evidence about the approach, not a view about style: guarding means
remembering at every site, and the record says nobody does.

**Count the guards in the donor that guards.** If the count is short,
the structural fix is the one with the argument behind it.

### What the markers settle

`docs/provenance/missing.md` and the marker test in `b8596dbf`: every
file in Berkeley's tree carries a redistribution marker, the Lite
releases kept `%sccs.include.redist.c%` and dropped
`%sccs.include.proprietary.c%`. So "why is this missing" is a lookup,
not an argument. Verified on seven samples.

Note the exception it turns up: `zopen.c` is marked redistributable and
is still absent, because `usr.bin/compress` was removed whole for the
LZW patent rather than for its licence.

### XNU is read-only and partial

`precedent.md` allows findings, not code. And it is silent on more than
expected: `bsd/kern/subr_prof.c` is not in that tree at all, because
Apple replaced Berkeley's profiling with Mach's. It is a witness for
the syscall and VFS layers and silent elsewhere.

---

## 4. The checks that actually find things

### Did the code reach the binary?

A false `#if` deletes code with no error and no warning. `nm` is how
you tell:

	nm trap.o   | grep trapwrite      # T = defined here
	nm locore.o | grep trapwrite      # U = referenced, needs linking

`trapwrite` was `T` in `trap.o` and absent from `locore.o` -- defined,
dead, because `I386_CPU` was not in `IDENT`. The build was clean both
ways.

### Does the arithmetic close?

Before publishing any count, check the numbers are consistent with each
other. Sixteen `movw` becoming six, with four conversions, needs six
more to have gone somewhere.

### Does the struct have the layout the hardware wants?

`sizeof` is not the test for anything a CPU instruction reads. Dump the
bytes:

	struct x v; v.limit = 0x53; v.base = 0xfe07cf40;
	for (i = 0; i < 8; i++) printf("%02x ", ((unsigned char *)&v)[i]);

All three candidate forms of `region_descriptor` are eight bytes. Only
one puts the base at offset 2.

### Does the kernel get further?

	sh build/kernel.sh
	sh build/shim/run.sh -d int -D /tmp/q.log
	grep -m1 'v=' /tmp/q.log                 # the first exception
	grep -A14 'v=0d' /tmp/q.log              # full register state

Read the fault code. `v=0d e=0010` is a general protection fault
against selector 0x10; `v=0e` with `CR2` is a page fault and `CR2` is
the address. `GDT=` in the register dump is what `lgdt` actually
loaded -- it told us the base was `0xcf400000` when `gdt` is at
`0xfe07cf40`, which named the padding bug exactly.

### Is the hardware doing what you think?

For anything timing- or hardware-shaped, a multiboot stub under QEMU
settles it in minutes where reasoning does not. `build/shim` is the
pattern; a throwaway copy with the code under test in `kmain` and
output over the serial port at 0x3f8 works for arithmetic too. That is
how `delaycount` was found to be 271473, which is what made the `DELAY`
overflow concrete rather than theoretical.

---

## 5. Delivery

`AGENTS.md` has the procedure. Two things it does not warn about, both
of which bit repeatedly:

**Check the patch's commit count before handing it over.**

	git format-patch origin/dev3..HEAD --stdout > p.patch
	grep -c '^From ' p.patch      # must equal:
	git rev-list --count origin/dev3..HEAD

The maintainer applies with `git am`, which re-commits with new SHAs.
Local originals then look unmerged and land in the next patch as
duplicates. The tell is the count; the fix is to compare trees and
rebase them away:

	[ "$(git rev-parse origin/dev3^{tree})" = "$(git rev-parse HEAD~1^{tree})" ] \
	  && git rebase --onto origin/dev3 HEAD~1 dev3

This happened on six consecutive deliveries.

**Dry-run on a fresh clone, and build in it.** Not just `git am` -- run
`sysroot.sh`, `kernel.sh`, and `shim/run.sh`, and compare the tree hash
against the local one. A patch that applies and does not build is worse
than no patch.

---

## 6. Commit messages

The ones that have held up say four things:

- **what was wrong**, concretely, with file and line
- **what was measured**, with the command or the numbers
- **what was declined and why** -- the alternative donor, the larger
  change, the thing that would have been more invasive
- **what this does NOT do**, which is usually the most useful part

The last one matters most here because almost nothing has run. Say so.
"It compiles and the symbols resolve" is not "it works", and the
difference should be in the message rather than discovered later.

When a commit corrects an earlier one, say which and say what the
earlier one got right. `Revision: what the 1993 merge changed in
locore.s, counted properly' corrects both the original commit and this
audit's first attempt at correcting it, and says so.

---

## 7. Standing facts worth not rediscovering

- The kernel runs at **physical 0**. `locore.s` reaches variables as
  `symbol-SYSTEM` before paging; `boot.c:148` masks the a.out entry
  with `& 0x000fffff`. No multiboot loader will place an image below
  1 MB -- measured, QEMU answers `invalid load_addr address` even with
  the a.out kludge fields set.
- **Every descendant moved to physical 1 MB before Lite2 shipped**:
  NetBSD 1.0 `KERNTEXTOFF 0xf8100000`, OpenBSD 1996 `0xf0100000`,
  FreeBSD 2.0.5 `LOAD_ADDRESS F0100000`. Lite2 has no `KERNTEXTOFF`.
  Moving is the genetic answer and deletes `build/shim`.
- The ELF program headers say `PhysAddr == VirtAddr`. **This is normal**
  -- NetBSD 1.6's kernel says the same and its loader subtracts
  `KERNBASE`. The offset is the loader's convention in every BSD.
- A kernel linker script does **not** fix load addresses. NetBSD's
  exists to put `_etext` after the read-only sections, and first
  appears in NetBSD 1.6 (2001), six years after Lite2.
- `-DLOCORE` is passed for exactly one compile in the whole build:
  `locore.o`. Guarding a header's C with `#ifndef LOCORE` affects
  nothing else -- but the guard must sit above the includes, since
  `machine/frame.h` declares `struct trapframe` and has no guard of
  its own.
- `#ifdef cgd_notdef`, `#ifdef garbage`, `#ifdef notdef` and `#ifdef
  notyet` in `sys/i386` are 386BSD-era code inherited disabled and
  never resolved. Five instances found so far: `mkglue.c`'s vector
  generator, `rawintr`, the `copyout` duplicate, the CPU detection,
  and `setconf`. Look there before importing anything.
