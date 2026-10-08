# procfs: 4.4BSD's own answer, complete and unadopted

`docs/provenance/clist.md` ends by concluding that no BSD restored
CSRG's code, so the settlement bodies must be written. That holds for
the clist and the buffer cache. For process tracing it is the wrong
question, because this tree already contains 4.4BSD's answer.

This is the `subr_autoconf.c` situation again: a complete new
mechanism sitting in the tree, adopted by some ports and not others,
which an earlier session found when the i386's device configuration
looked archaic and sparc turned out to be using something newer.

## What is here

`miscfs/procfs` is **2,444 lines across nine files with no stubs at
all**:

	procfs_ctl.c       300   attach, detach, step, run, wait
	procfs_mem.c       300   procfs_rwmem, procfs_domem
	procfs_regs.c       95   procfs_doregs
	procfs_fpregs.c     95   procfs_dofpregs
	procfs_status.c    145
	procfs_note.c       73
	procfs_subr.c      314
	procfs_vfsops.c    193
	procfs_vnops.c     929

Between them they do everything `ptrace` does -- attach to a process,
stop and step it, read and write its memory, read and write its
registers -- through the filesystem rather than a system call.

## Why it survived the settlement when `sys_process.c` did not

`procfs_ctl.c` opens

	Copyright (c) 1993 Jan-Simon Pendry
	Copyright (c) 1993
		The Regents of the University of California.

It was new work at Berkeley, taking the idea from Plan 9. `ptrace` and
`procxmt` descend from AT&T's Unix, which is why `kern/sys_process.c`
was emptied and `miscfs/procfs` was not. The settlement removed what
USL claimed; it did not remove what CSRG had written themselves.

## What is missing, and it is only the machine-dependent half

	hp300     hp300/procfs_machdep.c    164 lines
	pmax      pmax/procfs_machdep.c     150 lines
	luna68k, sparc, news3400, vax, tahoe, i386   absent

Two ports of eight have written it. hp300's provides six functions:

	procfs_read_regs    procfs_write_regs
	procfs_read_fpregs  procfs_write_fpregs
	procfs_sstep        procfs_fix_sstep

and they are short. Its `procfs_read_regs` is the whole shape:

	f = (struct frame *) p->p_md.md_regs;
	bcopy((void *) f->f_regs, (void *) regs->r_regs,
	    sizeof(regs->r_regs));
	regs->r_pc = f->f_pc;
	regs->r_sr = f->f_sr;

The i386 has `p_md.md_regs` -- `i386/include/proc.h:41` -- so the body
is a copy out of the trap frame this port already maintains and whose
offsets `i386/include/reg.h` already names, `sEIP`, `sESP` and the
rest.

What the i386 lacks is `struct reg` and `struct fpreg`. hp300 declares
both in its `include/reg.h`; the i386's declares no structures at all,
only the index macros. So adopting procfs here means two things, both
modelled on ports in this tree:

1. `struct reg` and `struct fpreg` in `i386/include/reg.h`, laid out
   to match the trap frame the index macros already describe.
2. `i386/i386/procfs_machdep.c`, after hp300's and pmax's.

## And this explains what the contemporaries wanted

NetBSD's `sys_process.c` will not link here because it calls
`process_sstep` and `process_set_pc`; FreeBSD 2.2's wants
`ptrace_set_pc` and `ptrace_single_step`. Those are the same six
functions under each project's own names: both took 4.4BSD's procfs
machine-dependent interface, renamed it, and routed their `ptrace`
through it.

So the hooks the contemporaries demand are **4.4BSD's own**, already
specified by this tree and already implemented by two of its ports.
Writing `i386/i386/procfs_machdep.c` is rule 1 work -- this tree's own
ports first -- and it serves `procfs` directly and `ptrace` through
it, which is the order 4.4BSD intended.
