/*
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1993 Jan-Simon Pendry
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * From:
 *	Id: procfs_i386.c,v 4.1 1993/12/17 10:47:45 jsp Rel
 *
 *	$Id: procfs_machdep.c,v 1.1 1994/01/05 07:41:44 cgd Exp $
 */

/*
 * AI-ONLY NOTE: Jan-Simon Pendry's i386 procfs code, procfs_i386.c
 * 4.1 of 17 December 1993, as NetBSD committed it on 5 January 1994
 * (procfs_machdep.c 1.1, above): the original from which Berkeley's
 * own hp300 and pmax procfs_machdep.c say they come. NetBSD 1.0's
 * process_machdep.c is the same code renamed. Lite2 shipped no i386
 * one, so neither /proc's register files nor ptrace(2) could be built
 * for it. Changed from Pendry's, each for this tree:
 *
 *  - SLOAD and p_regs are Lite2's P_INMEM and p_md.md_regs;
 *  - this port saves two kinds of frame (<machine/frame.h>): trap()
 *    a struct trapframe, syscall() a struct syscframe, and the pcb's
 *    FM_TRAP says which md_regs points at, as sendsig() and Lite2's
 *    own gdb (contrib/gdb-4.7.LBL, i386b-nat.c) read it. So the hooks
 *    use reg.h's tXXX or sXXX offsets accordingly. A system-call frame
 *    has no %es or %ds; the return path in locore.s loads both with
 *    _udatasel, which is what is read, and a write to them is ignored;
 *  - the flags are checked with OpenBSD's PSL_USERSTATIC, as sigreturn
 *    checks them, in place of Pendry's USERCLR and USERSET test, and
 *    a new %cs must be a user selector, as OpenBSD's check has it;
 *  - procfs_sstep takes Lite2's second argument, and clears the trace
 *    bit when asked to, as hp300's and pmax's are called to;
 *  - procfs_read_fpregs and procfs_write_fpregs answer EOPNOTSUPP, as
 *    hp300's and pmax's do, until the FPU's state can be saved on
 *    demand (docs/deferred.md).
 */

/*
 * Functions to be implemented here are:
 *
 * procfs_read_regs(proc, regs)
 *	Get the current user-visible register set from the process
 *	and copy it into the regs structure (<machine/reg.h>).
 *	The process is stopped at the time read_regs is called.
 *
 * procfs_write_regs(proc, regs)
 *	Update the current register set from the passed in regs
 *	structure.  Take care to avoid clobbering special CPU
 *	registers or privileged bits in the PSL.
 *	The process is stopped at the time write_regs is called.
 *
 * procfs_read_fpregs, procfs_write_fpregs
 *	deal with the floating point register set, otherwise as above.
 *
 * procfs_sstep(proc, sstep)
 *	Arrange for the process to trap after executing a single instruction.
 *
 * procfs_fix_sstep(proc)
 *	Cleanup process state after executing a single-step instruction.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <miscfs/procfs/procfs.h>
#include <machine/segments.h>	/* after procfs.h: it defines d_type */

extern int kstack[];		/* XXX */
extern int _udatasel;

procfs_read_regs(p, regs)
	struct proc *p;
	struct reg *regs;
{
	int *tp;

	if ((p->p_flag & P_INMEM) == 0)
		return (EIO);

	tp = (int *) ((char *) p->p_addr +
		((char *) p->p_md.md_regs - (char *) kstack));

	if (p->p_addr->u_pcb.pcb_flags & FM_TRAP) {
		regs->r_es = tp[tES];
		regs->r_ds = tp[tDS];
		regs->r_edi = tp[tEDI];
		regs->r_esi = tp[tESI];
		regs->r_ebp = tp[tEBP];
		regs->r_ebx = tp[tEBX];
		regs->r_edx = tp[tEDX];
		regs->r_ecx = tp[tECX];
		regs->r_eax = tp[tEAX];
		regs->r_eip = tp[tEIP];
		regs->r_cs = tp[tCS];
		regs->r_eflags = tp[tEFLAGS];
		regs->r_esp = tp[tESP];
		regs->r_ss = tp[tSS];
	} else {
		regs->r_es = _udatasel;
		regs->r_ds = _udatasel;
		regs->r_edi = tp[sEDI];
		regs->r_esi = tp[sESI];
		regs->r_ebp = tp[sEBP];
		regs->r_ebx = tp[sEBX];
		regs->r_edx = tp[sEDX];
		regs->r_ecx = tp[sECX];
		regs->r_eax = tp[sEAX];
		regs->r_eip = tp[sEIP];
		regs->r_cs = tp[sCS];
		regs->r_eflags = tp[sEFLAGS];
		regs->r_esp = tp[sESP];
		regs->r_ss = tp[sSS];
	}

	return (0);
}

/*
 * Update the process's current register
 * set.  Depending on the architecture this
 * may have fix-up work to do, especially
 * if the IAR or PCW are modified.
 */
procfs_write_regs(p, regs)
	struct proc *p;
	struct reg *regs;
{
	int *tp, trap;

	if ((p->p_flag & P_INMEM) == 0)
		return (EIO);

	tp = (int *) ((char *) p->p_addr +
		((char *) p->p_md.md_regs - (char *) kstack));
	trap = p->p_addr->u_pcb.pcb_flags & FM_TRAP;

	if (((regs->r_eflags ^ tp[trap ? tEFLAGS : sEFLAGS]) &
	    PSL_USERSTATIC) != 0 || ISPL(regs->r_cs) != SEL_UPL)
		return (EPERM);

	if (trap) {
		tp[tES] = regs->r_es;
		tp[tDS] = regs->r_ds;
		tp[tEDI] = regs->r_edi;
		tp[tESI] = regs->r_esi;
		tp[tEBP] = regs->r_ebp;
		tp[tEBX] = regs->r_ebx;
		tp[tEDX] = regs->r_edx;
		tp[tECX] = regs->r_ecx;
		tp[tEAX] = regs->r_eax;
		tp[tEIP] = regs->r_eip;
		tp[tCS] = regs->r_cs;
		tp[tEFLAGS] = regs->r_eflags;
		tp[tESP] = regs->r_esp;
		tp[tSS] = regs->r_ss;
	} else {
		tp[sEDI] = regs->r_edi;
		tp[sESI] = regs->r_esi;
		tp[sEBP] = regs->r_ebp;
		tp[sEBX] = regs->r_ebx;
		tp[sEDX] = regs->r_edx;
		tp[sECX] = regs->r_ecx;
		tp[sEAX] = regs->r_eax;
		tp[sEIP] = regs->r_eip;
		tp[sCS] = regs->r_cs;
		tp[sEFLAGS] = regs->r_eflags;
		tp[sESP] = regs->r_esp;
		tp[sSS] = regs->r_ss;
	}

	return (0);
}

procfs_read_fpregs(p, fpregs)
	struct proc *p;
	struct fpreg *fpregs;
{

	return (EOPNOTSUPP);
}

procfs_write_fpregs(p, fpregs)
	struct proc *p;
	struct fpreg *fpregs;
{

	return (EOPNOTSUPP);
}

procfs_sstep(p, sstep)
	struct proc *p;
	int sstep;
{
	int error;
	struct reg r;

	error = procfs_read_regs(p, &r);
	if (error == 0) {
		if (sstep)
			r.r_eflags |= PSL_T;
		else
			r.r_eflags &= ~PSL_T;
		error = procfs_write_regs(p, &r);
	}

	return (error);
}

void
procfs_fix_sstep(p)
	struct proc *p;
{
}
