# stdarg.h: the tree's own header was miscompiling the tree

A shell that could not run a command turned out to be a header. This
records the diagnosis, the measurement, and why the replacement is the
descendants' and not this project's invention.

## The symptom

Any non-empty command killed the single-user shell:

	# echo hi
	Oct 10 09:24 init: single user shell terminated, restarting

A bare return was fine. The shell exited on SIGSEGV -- `exit1 rv=0xb`,
reached through `sigexit <- postsig <- trap`.

## Narrowing it

A temporary `printf` in `i386/i386/trap.c`, on the path that turns a
failed `vm_fault` into a signal, gave the fault and, through `copyin`
of the user stack, the frames above it:

	XXFAULT pid 3 eip 807b844 va 0 code 4 rv 1
	XXEVA 2 eax 10 ecx 2 edx 2
	XXESP efbfddfc: 8072210 2 0 0 80ac040 80ac040 1 80ae000
	                8067470 2 ...

`code 4` is a user-mode read of a page that is not there. Resolved
against `/bin/sh` with `nm -S` and `objdump -d`, and by walking the
frames by hand through each function's prologue:

	807b844   strlen+4, `cmpb $0x0,(%edx)'
	8072210   strdup+16, the instruction after its call to strlen
	8067470   history_def_enter+32, the instruction after its strdup

and the faulting address is `2`. So libedit's `history_def_enter` was
called with `str = 2` and handed it to `strdup`.

`history_def_enter` is reached only from `history()`, which takes the
string as a variadic argument:

	case H_ENTER:
	    str = va_arg(va, const char *);
	    ev = HENTER(h, str);

and its one caller in the shell, `input.c:264`, passes
`parsefile->buf` -- a pointer into the static `basebuf[BUFSIZ]`, which
cannot be 2. Disassembling that call site confirmed the shell pushes
the right thing: `push 0x14(%ecx)` with `%ecx` holding `parsefile`,
and 0x14 is `buf`'s offset in `struct parsefile`.

So the argument was pushed correctly and read incorrectly. In
`history()`:

	 80678b0:	sub    $0x8,%esp
	 80678b3:	mov    %eax,0x24(%esp)
	 80678b7:	push   0x28(%esp)
	 80678bb:	push   (%ebx)
	 80678bd:	call   *0x18(%ebx)

Entry `%esp` worked out to `history+`'s frame base plus 60; 0x28(%esp)
there is **32 bytes below** the entry stack pointer -- inside
`history`'s own locals, four bytes past the slot the instruction
before it had just written `fun` into. The third argument lives 12
bytes *above* the entry stack pointer and was never read.

## The cause

`machine/stdarg.h`, this tree's own:

	typedef char *va_list;

	#define	__va_promote(type) \
		(((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

	#define	va_start(ap, last) \
		(ap = ((char *)&(last) + __va_promote(last)))

`&last` is the whole mechanism: take the address of the last named
parameter and step past it. That is correct only while the compiler
keeps a named parameter in the slot the calling convention gave it.
GCC does not promise that. When a parameter's address is taken and
nothing else observes it, GCC may copy it to a stack slot of its own
choosing, and `&last + 4` then points at an unrelated local.

Reduced, with this tree's headers and the userland's flags:

	int dispatch(void *h, int fun, ...)
	{
		va_list va;
		va_start(va, fun);
		switch (fun) {
		case 7: case 8: return sink(h, va_arg(va, const char *));
		...
	}

	$ gcc -m32 -std=gnu89 -fcommon -fno-builtin -fno-stack-protector \
	      -fno-pic -nostdinc -isystem $ROOT/usr/include -O -S vatest.c

	dispatch:
		subl	$28, %esp
		movl	36(%esp), %edx	# fun, at entry+8
		...
		subl	$8, %esp
		movl	%edx, 20(%esp)	# fun, copied to entry-16
		pushl	24(%esp)	# entry-12: an uninitialised local
		call	sink

Three things make this the worst kind of bug. It is silent -- the read
succeeds and yields whatever the local holds. It is per-function --
GCC takes the bad lowering only where it sees a reason to, so most
variadic functions in the tree are fine and a few are not. And it
moves: the same source can change sides with a flag or a compiler
version.

## The replacement, and whose it is

	typedef __builtin_va_list va_list;

	#define	va_start(ap, last)	__builtin_va_start((ap), (last))
	#define	va_arg(ap, type)	__builtin_va_arg((ap), type)
	#define	va_end(ap)		__builtin_va_end((ap))
	#define	__va_copy(dst, src)	__builtin_va_copy((dst), (src))

This is where all three descendants ended up, and the lineage is
exact: NetBSD's `sys/sys/stdarg.h` still carries this file's own SCCS
identifier, `@(#)stdarg.h 8.1 (Berkeley) 6/10/93`, and defines the
same macros over the same builtins. FreeBSD's `sys/sys/_stdarg.h` does
(`sys/i386/include/stdarg.h` is now four lines and a public-domain
notice pointing at it). OpenBSD's `sys/sys/stdarg.h` does.

Measured against each, line by line, of the five lines above:

	FreeBSD  sys/sys/_stdarg.h    2 identical
	NetBSD   sys/sys/stdarg.h     1 identical
	OpenBSD  sys/sys/stdarg.h     0 identical

The overlap is the macro signatures, which C fixes, over the builtin
names, which GCC fixes. There is no other way to write these lines,
and the differences between the three are whitespace and the spelling
of the macro parameters.

`__va_promote` is dropped: nothing in this tree used it but `va_start`.
`va_arg` no longer differs between the kernel and the userland; the
old userland form called `abort()` for a type narrower than `int`.
Nothing in this tree outside `contrib` asks for one -- every
`va_arg(ap, char ...)` in the tree is `char *` -- and the two forms
fail differently rather than one of them being safe: Berkeley's macro
called `abort()` at run time, and `__builtin_va_arg` warns at compile
time ("`char' is promoted to `int' when passed through ...", "if this
code is reached, the program will abort") and traps at run time.
Measured; an earlier version of this file and of the commit message
said the compiler rejects it outright, which it does not.

Also checked, because it is the one unusual use in the tree: the
kernel's `printf` passes a `va_list` *through* a `va_list` for its
`%r` recursion -- `subr_prf.c` does `va_arg(ap, va_list)`, fed by
`panic`. `__builtin_va_arg` takes it, because on this machine
`__builtin_va_list` is still a pointer type. It was portable by
construction while `va_list` was spelled `char *` and now is not, so
it is noted here rather than left to be found.

## Which files, and which ports

Lite2 ships **three** real `machine/stdarg.h` files; every other port
reaches one of them by symlink:

	hp300/include/stdarg.h     i386, vax, luna68k -> hp300; tahoe -> vax
	pmax/include/stdarg.h      news3400 -> pmax
	sparc/include/stdarg.h

`sys/mips` is itself a symlink to `sys/pmax`, so what looks like a
fourth file, `mips/include/stdarg.h`, is pmax's own seen through it.

and one `ansi.h`, hp300's, which every port symlinks and which defines
`_BSD_VA_LIST_` for the `vfprintf`-style prototypes in `<stdio.h>`,
`<err.h>` and `<sys/syslog.h>`. On the i386 `__builtin_va_list` *is*
`char *`, so changing `_BSD_VA_LIST_` changes no type and no ABI. On a
port where it is not, those prototypes would disagree with anything
still defining `va_list` from `machine/varargs.h`, which is left as
Berkeley wrote it -- `sys/syslog.h` carries a comment about exactly
that hazard.

All three files get the same body, because the mechanism is no longer
machine-dependent. That is why NetBSD collapsed its per-port copies
into one machine-independent `sys/sys/stdarg.h`; this tree keeps them
per port because `include/Makefile` installs `machine/stdarg.h`.

Only the i386 is built and measured here. Neither of the other two
bodies could have worked with this compiler in any case: sparc's is
written over `__builtin_saveregs` and `__builtin_next_arg`, which GCC
no longer provides, and pmax's walks the list with `&last`.

`machine/varargs.h` is left alone. Its K&R `va_dcl` form needs
`__builtin_va_alist`, which GCC removed; FreeBSD's modern `varargs.h`
is an `#error` for the same reason. Nothing this tree builds reaches
it -- every include of it sits in the `#else` arm of an `#if __STDC__`,
and `gnu89` takes the other arm.

## What it was worth, measured

Rebuilt from clean and compared object by object against the copies
kept aside:

	libc       11 of 478 objects changed
	           err fprintf fscanf printf scanf snprintf sprintf
	           sscanf syslog vfprintf vfscanf
	libedit     2 of 3     editline history
	sh          3 of 28    error output show
	kernel      2 of 138   subr_prf vm_glue
	                       (trap and vers changed for other reasons)

And a sweep, to answer the question those counts do not: every file in
`usr/src` outside `contrib` that names `va_start` or `va_arg` -- 98 of
them -- syntax-checked with this tree's userland flags against the new
header and against the old one, by `-fsyntax-only` with the old header
staged in a directory ahead of the sysroot. Identical outcome on all
98: the same 83 pass and the same 15 fail, the failures being missing
local headers and nothing to do with either version. So the change
breaks no file it reaches, including the ones no port here builds.

Of the functions in them, one was reading the wrong slot: libedit's
`history()`. Checked by hand through their prologues, `err`, `warn`,
`printf`, `fprintf`, `scanf`, `syslog`, `sprintf`, and the shell's
`out1fmt` and `fmtstr` all already computed the incoming slot -- `err`
at entry+12, `warn` at entry+8, `fmtstr` at entry+16 -- so for those
the difference is instruction scheduling and nothing else. Every one
of them was one compiler decision away from the other answer, which is
the point.

After the change, single user:

	# echo hi
	hi
	# x=7
	# echo $x
	7
	# cd /
	# for i in 1 2 3; do echo i$i; done
	i1
	i2
	i3
	# echo $((3+4))
	7
	# echo `echo sub`
	sub
	# /bin/sh -c "echo D"
	D

Command substitution and `/bin/sh -c` both fork and the second also
execs, so the exec path and the job path work too.

## The VM experiment this retires

While chasing the crash, an earlier fault at `va=0xefbfd000`
(`KERN_NO_SPACE`) led to an experiment: raise `VM_MAXUSER_ADDRESS`
from `0xEFBFD000` to `0xEFBFE000` so it meets `USRSTACK`, as NetBSD 1.0
and FreeBSD 2.0 both have it, and drop the `- NBPG` from
`kern_exec.c`'s stack allocation. It moved the fault but did not fix
the shell, and it was never committed.

Measured again with the header fixed: everything above works with
pristine `vmparam.h` and pristine `kern_exec.c`. The experiment is
unnecessary and is dropped. Lite2's one-page gap below `USRSTACK` is
left exactly as Berkeley wrote it.

## This is the rule in precedent.md, again

`docs/provenance/precedent.md`, *Era-appropriate is not the same as
correct*: the era donors are no help here. NetBSD 1.0's and
FreeBSD 2.0's `stdarg.h` carry the same `&last` body this tree does,
because that is what GCC 2.x honoured. The answer only exists in
releases well after Lite2, which is the same shape as the ELF
four-segment finding recorded there.

It adds one thing to that section: **a header is part of the
toolchain**. This tree supplies its own `<stdarg.h>`, and a header that
tells the compiler how to do something the compiler now does itself is
as much a toolchain-era fault as a missing `-fno-builtin`. The place to
look when generated code disagrees with readable source is not only
the flags.

## And a bug this uncovered

After a command whose exec fails, the shell's standard output went
silent for the rest of the session while standard error kept working:

	# pwd
	pwd: Cannot exec /bin/pwd
	# echo one two three
	#                         <- nothing, here and ever after
	# echo a | /bin/sh -c "cat"
	cat: not found            <- stderr still fine

Unrelated to this header, and chased separately:
`docs/provenance/pwd.md` has it. It is Berkeley's `getpwd` calling
`error()` in a forked child, where `error()` does not exit.
