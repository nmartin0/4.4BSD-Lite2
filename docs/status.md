# Where the work stands

What has been built, measured, and left open. Provenance questions --
what may be copied from where, and what this tree does and does not
contain -- live in `docs/provenance/`; this file is only state.

Every number here is from a run recorded in the commit that changed
it, on the host named below. A figure with no run behind it does not
belong in this file.

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

The link does not complete. Four symbols are undefined:

| symbol | wanted by | where the model is |
|---|---|---|
| `chrtoblk` | `miscfs/specfs/spec_vnops.c:170`, `miscfs/kernfs/kernfs_vfsops.c:81` | hp300, luna68k, news3400 and pmax all carry a `chrtoblktbl` and the function in their own `conf.c`; `i386/i386/conf.c` has neither, so this one needs a table built for i386's device numbering |
| `fuswintr`, `suswintr` | `kern/subr_prof.c:224` | `hp300/hp300/locore.s:1857`, `luna68k/luna68k/locore.s` |
| `wddriver` | `i386/i386/swapgeneric.c` | see below -- a dead reference, not a missing routine |

`wddriver` is named only by `genericconf[]`, and on i386 nothing
reads that table: the whole body of `setconf()` is inside
`#ifdef notdef`, lines 81 to 135, so `setconf()` is an empty
function. hp300, luna68k, vax, news3400 and pmax all have a live
`setconf()` that walks it. The same shape as the `rawintr` dispatch
dropped in `icu.s: drop the dispatch to rawintr`, but with a
difference worth weighing before doing the same thing: the table is
the only statement anywhere in this port of which driver is meant to
root it, and `rootdev` is hard-wired to `makedev(0,0)` behind the
dead `setconf`. It is tangled with the `wd.c` question in
`docs/provenance/missing.md`.

### How the kernel is built, which nothing in this tree says

`build/make.sh` is written for the userland and drives the build into
`MAKEOBJDIRPREFIX`; the kernel Makefile reaches the sources through
`$S`, a path relative to the compile directory, so under `make.sh` it
looks for `../../i386/i386/genassym.c` from inside the object tree
and does not find it.

What was used instead, recorded because it is not obvious and was
reconstructed once already:

```sh
cd usr/src/sys/compile/LINK.i386
MAKEOBJDIRPREFIX= bmake -m <tree>/usr/src/share/mk \
    CC="gcc -m32 -std=gnu89 -fcommon -fno-stack-protector \
        -fno-pic -ffreestanding -nostdinc" \
    CPP="cpp -m32 -traditional-cpp -nostdinc" \
    AS="as --32" LD="ld -m elf_i386" depend all
```

**These flags are not a considered choice of this project's.** They
were picked to reproduce a build and they do; `-ffreestanding` in
particular is not carried over from anything. The kernel Makefile
sets `AS`, `CC`, `CPP` and `LD` with `=` rather than `?=`, so they
must come from the command line and not the environment. Whether
`build/make.sh` should grow a kernel mode, and what the kernel's
flags should actually be, is open.

### libkern.a is not built by anything

The kernel Makefile links `libkern.a` and makes it with
`ln -s $S/libkern/obj/libkern.a libkern.a`. `usr/src/sys/libkern/obj`
is one of 468 `obj` symlinks shipped in the 1995 tarball, pointing at
`/usr/obj/sys/libkern`, so the link is dangling and the kernel link
fails with `cannot find libkern.a` before it reaches symbol
resolution.

Its 31 objects do compile, by hand, with the kernel flags above plus
`-I<compile dir>` for `machine/` and `-isystem <root>/usr/include`,
because `bcmp.c` includes `<string.h>`. That mixing of kernel and
userland include paths inside the kernel library is a design question
and has not been decided; the library used for the measurements below
was staged by hand and is not reproducible from this tree.

**Consequence for the record:** the undefined-symbol counts in the
commits of 28 September (13, 10, 8, 7, 6) cannot have come from a
completed link, since none was possible. They are consistent with a
count across objects, which cannot see a duplicate definition -- and
one was there, `copyout`, found only when the link was first run.

### How far it is from linking

With `libkern.a` staged and the four symbols above stubbed, the link
completes:

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
