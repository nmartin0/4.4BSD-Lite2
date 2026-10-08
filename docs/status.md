# Where the work stands

**The kernel execs a program and enters user mode.** With a disk image
carrying a label, an FFS filesystem and `/sbin/init`, all built by the
host tools in `build/`:

```
CPU: Pentium (586-class CPU)
wd0 at 0x1f0 irq 14 on isa
root on wd0
WARNING: no swap space found
```

and then QEMU's exception log shows

```
cpl=3 IP=001f:08049394 pc=08049394 SP=0027:efbfdfbc CR2=08075188
```

`cs` is 0x1f, the user code selector, and 0x08049394 is forty bytes
into init's text, whose ELF entry point is 0x0804936c. That is a user
instruction, executed by a program this kernel loaded from a disk it
mounted.

**Where it stops now: pmap_remove on the self-map.** The kernel runs
init, which forks; when the child is reaped the kernel panics with a
null dereference in pmap_remove:

```
vm_fault(f063a400, 0, 1, 0) -> 1
trap type 12 code = 0 eip = f004a878 cr2 0
```

The path is wait4 -> wait1 -> cpu_wait -> vmspace_free ->
vm_map_delete -> pmap_remove, and the virtual address it fails on,
read out of the registers at the faulting instruction, is 0xEFFBF000.

That is PTmap + PTDPTDI*NBPG: the page directory's own address in the
recursive self-map. i386/pmap.c's pmap_pinit installs that entry by
hand --

	*(int *)(pmap->pm_pdir+PTDPTDI) =
		pmap_extract(kernel_pmap, pmap->pm_pdir) | PG_V | PG_KW

-- so it never went through pmap_enter and has no pv entry. When
vm_map_delete hands pmap_remove a range covering it, the search loop
runs off the end of the pv list:

	for (npv = pv->pv_next; npv; npv = npv->pv_next) { ... }
	#ifdef DEBUG
		if (npv == NULL)
			panic("pmap_remove: PA not in pv_tab");
	#endif
	pv->pv_next = npv->pv_next;	<- npv is NULL

The guard is inside `#ifdef DEBUG', which is not defined, so instead
of the panic Berkeley wrote there is a null dereference. hp300's
pmap.c has the identical shape at its own pmap_remove, so the #ifdef
is Berkeley's across the tree and not an i386 defect: "PA not in
pv_tab" is written as a can't-happen.

Ruled out by measurement: nothing faults on 0xEFFBF000, so no map
entry is created there by faulting -- vm_map_delete passes
pmap_remove a range and pmap_remove walks into it. And this port's
pmap_release does free the directory, `kmem_free(kernel_map,
pmap->pm_pdir, NBPG)', the same as FreeBSD 2.0.5's.

**Earlier: not in the fault path.** init's first write to its
bss faults at 0x08075188, and that fault is handled correctly. Every
step returns KERN_SUCCESS, measured under gdb:

```
elf_load_psection            all four PT_LOAD segments     -> 0
user_page_fault(0x8048000)   a text page                   -> 0
user_page_fault(0x8075000)   the bss page                  -> 0
vm_fault(0x8075000)                                        -> 0
vm_map_pageable(0xefc20000)  the self-mapped page table    -> 0
```

So the page is mapped and init resumes. What happens instead is that
the machine then produces no exceptions at all. In QEMU's log the
user fault is entry 459 and the log ends there -- no further faults,
no system calls, and no clock interrupts, of which the whole boot
produces only six.

That is the question: after a correctly handled page fault returns to
user mode, nothing further executes.

And the measurement that probably explains it: **the clock stops, and
the disk does not.** Over a whole boot the log carries six interrupts
on vector 0x20, which is IRQ 0, the 8254. The last is well before the
user fault. Over the same stretch it carries eighty-three on vector
0x2e, IRQ 14, the disk, and seventeen page faults. Running the machine
for sixty seconds instead of thirty adds one entry to the log.

So IRQ 0 stops being delivered while IRQ 14 keeps working. The two
differ only in the mask: `biomask 4000' is bit 14 alone, so splbio
masks the disk and not the clock, and whatever is holding the clock
off is not splbio. With no clock there is no preemption, no timeout
and no wakeup, which is a sufficient explanation for a machine that
stops without a fault.

That is where to look next, and it is a different place from the exec
path this section used to point at.

An earlier revision of this file recorded that user_page_fault never
returns for the bss page. That was a bad measurement -- a conditional
breakpoint left armed, so `finish' re-entered it rather than
completing -- and it is wrong.


**The kernel runs.** As of `i386: COMCONSOLE, so the kernel can be
heard', a kernel built from this tree loads, executes, and prints:

```
bios 640K+65535K. maxmem 40ff000, physmem 40a0000
ps 100000 pe 40ff000 Copyright (c) 1982, 1986, 1989, 1991, 1993
	The Regents of the University of California.  All rights reserved.

CPU: Pentium (586-class CPU)
real mem  = 67764224
kmem_suballoc: bad status return of 3.
panic: kmem_suballoc
syncing disks... trap type 12 code = 0 eip = fe017bf1 cs = 8
  eflags = 282 cr2 fe0f1000 cpl ffffffff
panic: trap
dumping to dev 1, offset 0
dump device bad
```

So far as the CSRG history shows, neither Lite release's i386 kernel
had linked before this week, let alone run: the `copyout` duplicate
alone made linking impossible and that defect is in Berkeley's own
tree.

Run it with `sh build/kernel.sh && sh build/shim/run.sh`. QEMU and
the shim are described in `build/shim/shim.c`; the shim exists
because this kernel loads at physical 0 and no multiboot loader will
place an image below 1 MB.

## Where it stops, and it depends on how much memory it is given

**`kmem_suballoc` returns 3**, `KERN_NO_SPACE`. Not in `kmeminit`, as
first recorded, but in `cpu_startup` -- the output reaches `real mem`
and never `avail mem`, and the failing call is `machdep.c:266`, the
buffer map.

Measured, rather than reasoned:

	-m 16   past cpu_startup, into pmap_enter
	-m 32   kmem_suballoc: bad status return of 3
	-m 64   kmem_suballoc: bad status return of 3

The arithmetic is `cpu_startup`'s own. `bufpages` is derived from
`physmem`, `nbuf` is half of it, and the buffer map is
`MAXBSIZE * nbuf` -- 64 KB of **virtual** space per buffer, as the
comment above it says: "they usually occupy more virtual memory than
physical". At 64 MB that is 426 buffers wanting 26 MB, and
`VM_MIN_KERNEL_ADDRESS` to `VM_MAX_KERNEL_ADDRESS` is 0xFDFF7000 to
0xFF7FF000 -- **24 MB in total**.

So this is not a defect. It is Berkeley's sizing meeting a machine
four times larger than its address space was laid out for, and the
panic is honest.

**It is also the measured reason to move `KERNBASE`.** Lite2's
`0xFE000000` gives the smallest kernel virtual space any tree in the
lineage used, by a factor of four, and every descendant moved it
down:

	4.4BSD-Lite2    0xFE000000     24 MB
	NetBSD 1.0-1.2  0xf8000000    128 MB
	OpenBSD 1996    0xf0000000    256 MB
	FreeBSD 2.0.5   F0100000      256 MB
	NetBSD 1.3      0xf0000000    256 MB
	NetBSD 1.4-now  0xc0000000      1 GB

NetBSD moving down twice is this same pressure: machines grew, `nbuf`
grew with them, and the kernel map had to hold the buffers.
`0xf0000000` is the value two contemporaries and a third tree reached
independently.

### Moving KERNBASE: the map, computed

The whole address map is interlocked, so this is a coordinated shift
of twelve constants rather than one `#define`. `0xf0000000` is the
target: `KPTDI_FIRST` becomes `0x3c0`, which is `0xf0000000 >> 22`,
the slot OpenBSD 1996 and FreeBSD 2.0.5 both use, and NetBSD reached
at 1.3.

	constant                   now          after
	PTDPTDI (pmap.h)           0x3f7        0x3bf
	KPTDI_FIRST (pmap.h)       0x3f8        0x3c0
	KPTDI_LAST (pmap.h)        0x3fa        0x3c2
	USRSTACK                   0xFDBFE000   0xEFBFE000
	VM_MAXUSER_ADDRESS         0xFDBFD000   0xEFBFD000
	UPT_MIN_ADDRESS            0xFDC00000   0xEFC00000
	UPT_MAX_ADDRESS            0xFDFF7000   0xEFFF7000
	VM_MIN_KERNEL_ADDRESS      0xFDFF7000   0xEFFF7000
	KPT_MIN_ADDRESS            0xFDFF8000   0xEFFF8000
	KPT_MAX_ADDRESS            0xFDFFF000   0xEFFFF000
	KERNBASE (param.h)         0xFE000000   0xF0000000
	SYSTEM (locore.s)          0xFE000000   0xF0000000

A uniform shift of 56 page-directory slots, 224 MB. Two things stay
where they are, being at the top of the directory: `APDRPDROFF` at
`0x3fe`, the alternate page directory, and `VM_MAX_KERNEL_ADDRESS` at
`0xFF7FF000`.

**Kernel virtual space: 24 MB to 248 MB.** At 64 MB of RAM the buffer
map wants 26; at 512 MB about 200.

**That table is the C half and it is not enough.** Attempted, and the
kernel faulted at its own entry with `CR2` equal to `EIP` the moment
paging came on, then later in `cpu_startup` with `CR2 0xd`. A full
sweep of `sys/i386` for constants in this range finds **twenty-one
sites in three independent copies of the same map**, with nothing
connecting them:

	pmap.h      UPTDI 0x3f6, PTDPTDI 0x3f7, KPTDI_FIRST 0x3f8,
	            KPTDI_LAST 0x3fA
	vmparam.h   the eight addresses in the table above
	locore.s    SYSPDROFF 0x3F8, PDRPDROFF 0x3F7, PPDROFF 0x3F6,
	            and PTmap 0xFDC00000, PTD 0xFDFF7000,
	            Sysmap 0xFDFF8000, PTDpde and APTDpde built from
	            0xFDFF7000 -- the assembler cannot read pmap.h, so
	            this file writes the whole map out again under
	            different names
	literals    pmap.c:269, :367, :1703; trap.c:202;
	            vm_machdep.c:305, :306 -- all `0xfe000000'
	            pccons.c:79 MONO_BUF 0xfe0B0000 and :81 CGA_BUF
	            0xfe0B8000, which are KERNBASE + 0xB0000
	            locore.s:454 `subl $0xfe0a0000,%eax'
	loop        pmap.c:1278 `for(x=0x3f6; x < 0x3fA; x++)' --
	            UPTDI through KPTDI_LAST, written out

`UPTDI 0x3f6` was missed from the first table entirely.

**And the attribution here was wrong.** This said `pmap.c:1278`'s
loop was "almost certainly the `CR2 0xd` fault". It was not. Every
failed attempt failed for one reason: `assym.s` was not regenerated
when `machine/pmap.h` changed, so `locore.s` assembled with `SYSTEM`
at the old address while the C half and the linker used the new one.
`mkdep -p` has not produced a program dependency since `cc` stopped
writing `prog.o : `, so the `assym.s: genassym` rule in
`Makefile.i386` never fired. Fixed in `mkdep: -p has not worked...`
and `build: take mkdep from this tree...`, after which the move built
correctly on the first attempt.

The loop was a real find and had to be fixed -- it walks `UPTDI`
through `KPTDI_LAST` with both bounds written out -- but it was not
what caused that fault, and guessing that it was sent the next two
attempts after the wrong thing.

`machdep.c:1019` is the one site that already writes `KERNBASE +
0xa0000` rather than a literal, and is correct as it stands.

The move is therefore a sweep of all twenty-one, not an edit of
twelve, and the literals should become `KERNBASE`-relative while they
are being touched so the next person moving it has one place to look.
Measure against `sh build/shim/run.sh -m 16`.

The structure is the donors' and the move is a shift, not a redesign.
NetBSD 1.0 has `PTDPTDI 0x3df`, `KPTDI 0x3e0`, `APTDPTDI 0x3ff` --
the same three roles in the same order, 96 MB further down. Its
`vmparam.h` writes each address as a literal with the formula above
it in a comment, which is worth copying: these headers are read by
the assembler as well as the compiler.

Measure against `-m 16`, which is the only size that currently gets
past `cpu_startup`.

### With 16 MB it goes further, and stops at a debugger

	CPU: Pentium (586-class CPU)
	real mem  = 16384000
	ptdi 7e067

and waits. That is `i386/i386/pmap.c:883`, inside `pmap_enter`:

```c
	/*
	 * Page Directory table entry not valid, we need a new PT page
	 */
	if (!pmap_pde_v(pmap_pde(pmap, va))) {
		pg("ptdi %x", pmap->pm_pdir[PTDPTDI]);
	}
	pte = pmap_pte(pmap, va);
```

`pg()` is `isa/pccons.c:823` -- `printf` followed by `getchar()`, a
debug pager someone left in. The kernel is not hung; it is waiting
for a keystroke at a serial console nobody is typing at. QEMU takes
stdin, so pressing a key continues it.

The code beneath looks unfinished -- it detects a missing page table,
prints, and falls into `pmap_pte` with an invalid page directory entry
regardless -- but `docs/provenance/audit.md` establishes that it is
not a missing function.

No i386 pmap in the Mach lineage allocates a page table here. NetBSD
1.0's has the same comment at its `pmap.c:1070` and panics; FreeBSD
2.0.5 panics; and `pmap_pte` returns `NULL` because the page tables
live in the self-mapped window at `UPT_MIN_ADDRESS`, where touching an
absent one takes a fault that `vm_fault` resolves. **The VM system
allocates them; the pmap never does.**

So reaching this line is a symptom rather than a gap: something
upstream failed to fault the page table in. The question is why
`pmap_enter` is called for an address whose table was never faulted,
which points at `pmap_pinit`, at the object backing the `UPT` region,
or at the caller -- not at writing hp300's `pmap_enter_ptpage` for the
i386, which no BSD has and the architecture does not want.

Then **`vfs_busy` dereferences a null mount pointer**, `trap type 12`
at `fe017bf1`, because there is no root device: `rootdev` is
`makedev(0,0)` and `setconf()` is entirely inside `#ifdef notdef`.
That needs `wd.c`'s nine `b_actf` sites first.

## What had to be fixed to get here

Each was a defect in Berkeley's own code that could not surface while
the kernel did not link.

| | |
|---|---|
| `copyout` | two definitions of the symbol, one in C and one in assembly, since 4.4BSD-Lite. The C one is Pace Willisson's 386 write-protection workaround |
| `wddriver` | `genericconf` declared `extern struct driver`, `wd.c:163` defines `struct isa_driver` -- two types for one symbol |
| `lgdt`, `lidt` | called with two arguments; `locore.s` takes one pointer. `r_gdt` and `r_idt` sat correct and unreferenced |
| `region_descriptor` | `rd_base` a pointer, so `lgdt` read the base from the padding |
| `segment_descriptor` | twelve bytes where the processor reads eight, so `gdt[]` strided wrong and the entries interleaved |
| the clock | `startrtclock()` was called from nowhere, so the 8254 was never programmed and every `DELAY` was a no-op |
| three page mappings | kernel text/data/bss, the proc 0 stack's PTEs and its PDE, all mapped without the write bit |

That last row is one defect in three places, and `locore.s` states
its premise: "don't bother with making kernel text RO, as 386 ignores
R/W AND U/S bits on kernel access (only v works) !" True of the
80386, which has no `CR0_WP`. Enabling write protection made every
assumption resting on it surface in turn, and `copyout` was the same
sentence expressed in C.

## The host these numbers come from

Ubuntu 24.04, gcc 13.3.0, GNU binutils 2.42, bmake 20200710,
mtree-netbsd, byacc, flex.

**`build/make.sh`'s reasoning was written against gcc 14.2.0**, and
so were the measurements in every commit before `trap.c: copyout is
in locore.s`. The differences found so far are two, both recorded
where they matter:

- `memset` appears in the kernel's undefined symbols on gcc 14 and
  not on gcc 13. `docs/provenance/missing.md` explains it as an
  artefact of gcc 14 turning loops in `libkern/qdivrem.c` and
  `strncpy.c` into calls; `nm` on both objects built here shows no
  reference, with or without `-ffreestanding`.
- `-std=gnu89` is correct on both, but gcc 13 only warns where gcc 14
  makes implicit declarations hard errors, so this host is the more
  forgiving of the two. A clean result here does not prove a clean
  result there.

## Userland: builds

`sh build/sysroot.sh` runs to completion, rc=0. It installs the
hierarchy from `etc/mtree/4.4BSD.dist`, the headers, three host tools
(`lorder`, `yacc`, `config`), eleven libraries and the four ELF
startup files.

A program builds against that root. `bin/cat` via
`sh build/make.sh NOMAN=noman` gives an ELF 32-bit i386 statically
linked executable.

`NOMAN=noman` is needed unless the host has `nroff`: a program's
`all` target builds its manual page, and without nroff the build
stops with `Error code 127` after the binary is already linked.
`sysroot.sh` passes `NOMAN=noman` for the libraries for this reason;
nothing passes it for a program, so it is typed by hand.

Several hundred of Lite2's own compiler warnings remain throughout,
and `tsort` reports loops among the profiling objects. Neither has
been audited.

## Kernel: compiles, does not yet link

From `usr/src/sys/i386/conf`, `config LINK.i386` into
`usr/src/sys/compile/LINK.i386`, then make there:

```
136 objects, 0 compile or assembler errors, 423 warnings
```

**The kernel links.** As of `i386: genericconf carries no driver, and
the kernel links`:

```
vmunix: ELF 32-bit LSB executable, Intel 80386, statically linked
text 454844   data 14404   bss 42376
entry 0xfe000000, with `start' at that address
```

So far as the CSRG history shows this is the first time: the `copyout`
duplicate alone made it impossible, and that defect is present in
4.4BSD-Lite and in Berkeley's own tree.

The four symbols that stood in the way, and what each needed:

| symbol | answer |
|---|---|
| `blkclr` | a second name on `bzero`, as FreeBSD 2.0.5's i386 `support.s` writes it |
| `chrtoblk` | hp300's function unchanged, plus a `chrtoblktbl` read off this port's own two switch tables |
| `fuswintr`, `suswintr` | both names on one body returning -1, as FreeBSD 2.0.5 does, with its reason: "Fail all the time for now - until the trap code is able to deal with this" |
| `wddriver` | not a missing routine. `genericconf` named `extern struct driver wddriver` where `wd.c:163` defines `struct isa_driver wddriver` -- two types for one symbol. FreeBSD 2.0.5's table has no driver field at all, and its entries are guarded by the device counts `config` writes |

### Where the kernel loads, and who knows it

`readelf -l vmunix` reports `VirtAddr 0xfe000000  PhysAddr 0xfe000000`
for every LOAD segment, while the kernel must be loaded at **physical
0**: `locore.s` reaches its variables as `symbol-SYSTEM` before paging
is on, and `i386/stand/boot.c:148` masks the a.out entry with
`& 0x000fffff` to turn `0xFE000000` into 0.

**This is normal and is not a defect.** An earlier version of this
section called the image "linked for the wrong physical address" and
said any ELF-aware loader would place it 4 GB too high. That was
wrong, and the check that disproved it is NetBSD 1.6, the earliest
BSD with an i386 kernel linker script: it links at `TEXTADDR =
c0100000`, its `kern.ldscript` carries no physical address at all,
and so its program headers say the same thing ours do. Its loader
subtracts `KERNBASE` and places the image at `0x100000`.

**The offset is the loader's convention in every BSD, a.out and ELF
alike**, and this tree's own bootstrap implements it with that mask.
A linker script would not change the `PhysAddr` field and would not
have fixed anything.

What a kernel linker script is actually for, in NetBSD's own words at
the head of `kern.ldscript`: "This script is based on elf_i386.x, but
puts `_etext` after all of the read-only sections." Section placement
and symbol boundaries -- which matter for `dumpsys` and the symbol
table, neither exercised here yet.

It first appears in **NetBSD 1.6 (2001)**, `kern.ldscript` rev 1.1 by
Jason Thorpe; 1.0, 1.2 and 1.4 have none. So `a50b8e5a`'s "no BSD of
this period has one to copy" is exact -- the earliest is six years
after Lite2.

### How the kernel is built

`build/kernel.sh`, which takes a configuration name and defaults to
`LINK.i386`. It runs `config`, builds `libkern.a`, then `make depend`
and `make`, and reports the object count and whether the link
completed.

It is a separate script rather than a mode of `build/make.sh` because
`make.sh` sets `MAKEOBJDIRPREFIX`, and the kernel Makefile reaches its
sources through `$S`, a path relative to the compile directory: under
an object directory it looks for `../../i386/i386/genassym.c` from
inside `${L2_BUILD}` and does not find it. The kernel Makefile also
sets `AS`, `CC`, `CPP` and `LD` with `=` rather than `?=`, so they
must come from bmake's command line, where they beat a makefile
assignment; the environment would not.

**The kernel's compiler settings are this project's own.** No BSD of
this period cross-compiled a kernel, so there is no donor line for any
of them. Each is named in the script's header with the reason.
`-ffreestanding` in particular is not carried over from anything.

### libkern.a

`build/kernel.sh` builds it, into the compile directory. Nothing else
in this tree does: the kernel Makefile links
`${S}/libkern/obj/libkern.a`, and that `obj` is one of 468 symlinks
shipped in the 1995 tarball
pointing at `/usr/obj`, so it has always dangled. The Makefile's rule
has no prerequisites and is prefixed `-@`, so a `libkern.a` that
already exists is left alone and the dangling symlink is never
followed.

31 members. It is compiled with the kernel's own settings plus two
include paths the kernel Makefile does not need: the compile
directory, for the `machine/` symlink `config` writes there, and the
target root's headers, because `libkern/bcmp.c` includes
`<string.h>`. **Mixing kernel and userland include paths inside the
kernel's own library is not a settled decision.** It is what compiles,
it is confined to this one archive, and the script says so.

**Consequence for the record:** the undefined-symbol counts in the
commits of 28 September (13, 10, 8, 7, 6) cannot have come from a
completed link, since none was possible. They are consistent with a
count across objects, which cannot see a duplicate definition -- and
one was there, `copyout`, found only when the link was first run.

### How far it is from linking

Before the four symbols were supplied, the link was proved reachable
by stubbing them:

```
ELF 32-bit LSB executable, Intel 80386, entry 0xfe000000
text 454263  data 14372  bss 42380
```

That is a scratch measurement and says nothing about correctness. It
establishes only that nothing else stands between the objects and a
linked image.

The link passes no `-e`. `ld` warns `cannot find entry symbol _start;
defaulting to fe000000`, and that is right by coincidence: `start` is
at `0xfe000000` because `locore.o` is first in `SYSTEM_OBJ` and
`start` is its first symbol. OpenBSD 1996's line, quoted in
`Makefile.i386`'s own note, carries `-e start`.

## Behind the link

Three things stand between a linked kernel and one that runs. None is
started. They are listed because reaching the link will otherwise
look like more progress than it is.

**The settlement removed 35 function bodies, and 34 of them are in
this configuration.** `docs/provenance/missing.md` has the argument
and names NetBSD 1.0 as the BSD-licensed donor, and why XNU and
Rhapsody are the worst place to look for exactly these files. By
name:

```
kern/vfs_bio.c     bread breadn bwrite bdwrite bawrite brelse incore
                   getblk geteblk allocbuf getnewbuf biowait biodone
                   count_lock_queue
kern/tty_subr.c    clist_init getc q_to_b ndqb ndflush putc b_to_q
                   nextc unputc catq
kern/subr_rmap.c   rminit rmalloc rmfree
kern/sys_process.c ptrace trace_req
kern/kern_physio.c physio minphys
kern/kern_acct.c   acct acct_process
kern/kern_exec.c   execve
```

They link because Berkeley left stubs that return an error --
`bread()` is `return (EIO);`. `kern_exec.o` is 50 bytes.
`hp/dev/hil_subr.c:hilq_to_b` is the 35th and is not built here.

**Nothing in this tree can load the kernel.** `i386/stand/boot.c:116`
declares `struct exec` and line 122 accepts only `a_magic` 0407, 0410
and 0413. The kernel links ELF. The `OBJECT_FMT` switch in
`Makefile.i386` does not answer this on the current toolchain -- see
the note there and `Revision: the a.out arm of OBJECT_FMT was never
linked`. Unresolved, and larger than a commit.

**`DELAY()` rounds anything under a millisecond to nothing.** Its own
note in `i386/include/param.h` says so, names the answer -- NetBSD
1.0's and OpenBSD 1996's 8254-reading `delay()` in `isa/clock.c`,
about thirty lines, adapted rather than copied -- and says it was
deferred while the object was to reach a link. `pccons.c` asks for
`DELAY(4000000)` and `wt.c` for `DELAY(1000)`.

## Smaller things deferred, with the note that records each

- `CLKF_INTR` returns zero: the i386 `clockframe` carries no flag
  saying whether the clock interrupted at interrupt level
  (`i386/include/cpu.h`).
- `stathz` stays zero: no statistics clock on this port, so
  `kern_clock.c` does the statistics from `hardclock`
  (`i386/isa/clock.c`).
- `reset_cpu` is this port's name for what every other BSD calls
  `cpu_reset`; renaming touches `pccons.c`, `i386/i386/machdep.c` and
  the bootstrap (`i386/isa/pccons.c`).
- The `PANIC` and `PRINTF` macros in `locore.s` (lines 1573, 1575)
  still emit `_waittime` and `_printf` rather than going through
  `_C_LABEL`. Both are invoked only from commented-out lines, so
  nothing breaks; every other bare `_name` left in `locore.s` and
  `icu.s` is inside a comment.
- The kernel's 423 warnings have not been audited.

## What has been ruled out

Kept so nobody runs these again.

- **`copyout` was not masking another duplicate.** Every symbol
  defined more than once across all 136 kernel objects and libkern's
  31 was enumerated with `nm`; `copyout` was the only one. `icu.s`
  and `vector.s` are `#include`d by `locore.s` at lines 1798 and
  1799, so `locore.o` is the only assembly object.
- **The `OBJECT_FMT` switch does not give an a.out kernel here.**
  binutils 2.42 has no a.out emulation; `-n` means NMAGIC alignment
  inside ELF. Measured.
- **`memset` was not fixed by anything.** See the host section above.

## Not established

- OpenBSD 1996 and 4.4BSD-Lite were not reachable from the session
  that wrote the `copyout` change, so the sweep behind it covers four
  trees (NetBSD 1.0, 1.1, 1.2, FreeBSD 2.0.5) and not all of them. No
  absence is claimed from it.
- Nothing here has been booted. There is no QEMU script in `build/`
  and no kernel image to boot with one.
