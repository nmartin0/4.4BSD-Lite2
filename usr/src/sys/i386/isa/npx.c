/*-
 * Copyright (c) 1990 William Jolitz.
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)npx.c	8.1 (Berkeley) 6/11/93
 */
#include "npx.h"
#if NNPX > 0

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>

#include <machine/cpu.h>
#include <machine/trap.h>
#include <machine/specialreg.h>
#include <machine/segments.h>
#include <i386/isa/isa_device.h>
#include <i386/isa/icu.h>
#include <i386/isa/isa.h>
/*
 * 387 and 287 Numeric Coprocessor Extension (NPX) Driver.
 */

int	npxprobe(), npxattach(), npxintr();
struct	isa_driver npxdriver = {
	npxprobe, npxattach, "npx",
};

struct proc *npxproc;	/* process who owns device, otherwise zero */
struct pcb *npxpcb;	/* owners context structure */
static npxexists;
extern long npx0mask;

/*
 * AI-ONLY NOTE: for npxsave(), below, which Lite2's swtch and savectx
 * call and its tree had not got (see npxsave). The gate is the one
 * locore.s's npxsaveintr makes, built in npxattach.
 */
extern struct gate_descriptor idt[];
extern void npxsaveintr();
static struct gate_descriptor npx_idt_saveintr;
static int npx_intrno;
void npxsave();

/*
 * Probe routine - look device, otherwise set emulator bit
 */
npxprobe(dvp)
	struct isa_device *dvp;
{	static status, control;

#ifdef lint
	npxintr();
#endif

	/* insure EM bit off */
	load_cr0(rcr0() & ~CR0_EM);

	asm("	fninit ");	/* put device in known state */

	/* check for a proper status of zero */
	status = 0xa5a5;	
	asm ("	fnstsw	%0 " : "=m" (status) : "m" (status) );

	if (status == 0) {

		/* good, now check for a proper control word */
		control = 0xa5a5;	
		asm ("	fnstcw %0 " : "=m" (control) : "m" (control));

		if ((control&0x103f) == 0x3f) {
			/* then we have a numeric coprocessor */
		/* XXX should force an exception here to generate an intr */
			return (1);
		}
	}

	/* insure EM bit on */
	load_cr0(rcr0() | CR0_EM);
	return (0);
}

/*
 * Attach routine - announce which it is, and wire into system
 */
npxattach(dvp)
	struct isa_device *dvp;
{

	npxinit(0x262);
	/* check for ET bit to decide 387/287 */
	npxexists++;
	npx0mask = dvp->id_irq;
	{
		struct gate_descriptor save;

		npx_intrno = ICU_OFFSET + ffs(dvp->id_irq) - 1;
		save = idt[npx_intrno];
		setidt(npx_intrno, npxsaveintr, SDT_SYS386IGT, SEL_KPL);
		npx_idt_saveintr = idt[npx_intrno];
		idt[npx_intrno] = save;
	}
}

/*
 * Initialize floating point unit, usually after an error
 */
npxinit(control) {

	if (npxexists == 0) return;

	/*
	 * AI-ONLY NOTE: the owner's live state is saved and the chip freed
	 * first, as FreeBSD 1.1.5.1's and NetBSD 1.0's npxinit do (they
	 * pass npxsave a scratch area). setregs calls this at exec: the
	 * old image's registers go, and npxproc must not still name the
	 * process, for npxdna would then not reload what is saved below.
	 */
	if (npxproc)
		npxsave(&npxproc->p_addr->u_pcb.pcb_savefpu);

	load_cr0(rcr0() & ~CR0_EM);	/* stop emulating */
#ifdef INTEL_COMPAT
	asm ("	finit");
	asm("	fldcw %0" : : "m" (control));
	asm("	fnsave %0 " : "=m" (curpcb->pcb_savefpu) );
#else
	asm("fninit");
	asm("	fnsave %0 " : "=m" (curpcb->pcb_savefpu) );
#endif
	load_cr0(rcr0() | CR0_EM);	/* start emulating */

}

/*
 * Record information needed in processing an exception and clear status word
 */
npxintr(frame)
	struct intrframe frame;
{
	if (npxexists == 0) {
		printf ("stray npxintr\n");
		return;
	}

	outb(0xf0,0);		/* reset coprocessor */

	/*
	 * npxproc may be NULL, if this is a delayed interrupt from
	 * a process that just exited.
	 */
	if (npxproc) {
		/*
		 * sync state in process context structure, in advance 
		 * of debugger/process looking for it.
		 */
		asm("fnsave %0" : "=m" (npxproc->p_addr->u_pcb.pcb_savefpu));
		psignal(npxproc, SIGFPE);

		/* clear the exception so we can catch others like it */
		asm ("	fnclex");
	}
}

/*
 * Implement device not available (DNA) exception
 *
 * AI-ONLY NOTE: Lite2's npxdna saved the previous owner's state only
 * here, on the first use of the coprocessor by another process, and
 * never set EM again, so that after the first such fault nothing
 * trapped any more, nothing was saved at a context switch, and every
 * process that used the FPU saw its neighbours' registers. swtch and
 * savectx now save the owner's state (npxsave) and set EM, as FreeBSD
 * 1.1.5.1's and NetBSD 1.0's do, so the chip is normally free when a
 * process faults here, and this has only to load that process's
 * state. Where those panic on finding an owner, this saves it and
 * goes on.
 */
npxdna()
{
	if (npxexists == 0)
		return(0);

	if (npxproc)
		npxsave(&npxproc->p_addr->u_pcb.pcb_savefpu);
	load_cr0(rcr0() & ~CR0_EM);	/* stop emulating */
	/*
	 * Record the owner before frstor, which may raise an IRQ13 when the
	 * state it loads has an error pending (NetBSD 1.0's comment).
	 */
	npxproc = curproc;
	asm("	frstor %0 " : : "m" (curpcb->pcb_savefpu));
	return (1);
}

/*
 * Free the coprocessor (if p has it) when p exits: set EM again, so
 * that the next process's first FPU instruction traps and loads its
 * own state. NetBSD 1.0's npxexit; FreeBSD 1.1.5.1's saves into the
 * dying process's pcb, which nothing reads.
 */
void
npxexit(p)
	struct proc *p;
{

	if (p == npxproc) {
		load_cr0(rcr0() | CR0_EM);
		npxproc = NULL;
	}
}

/*
 * Wrapper for fnsave instruction to handle h/w bugs.  If there is an error
 * pending, then fnsave generates a bogus IRQ13 on some systems.  Force
 * any IRQ13 to be handled immediately, and then ignore it.  This routine is
 * often called at splhigh so it must not use many system services.  In
 * particular, it's much easier to install a special handler than to
 * guarantee that it's safe to use npxintr() and its supporting code.
 *
 * AI-ONLY NOTE: FreeBSD 1.1.5.1's npxsave, which NetBSD 1.0's is, with
 * three differences. The vector is npx_intrno, found in npxattach
 * where theirs is found by their probe; the coprocessor's mask is
 * npx0mask, as there; and the interrupt flag is put back as it was
 * found where theirs is set, because swtch calls this with
 * interrupts off. It saves the state, sets EM and leaves npxproc
 * NULL: the chip is then free.
 */
void
npxsave(addr)
	struct save87 *addr;
{
	u_char	icu1_mask;
	u_char	icu2_mask;
	u_char	old_icu1_mask;
	u_char	old_icu2_mask;
	struct gate_descriptor	save_idt_npxintr;
	u_long	eflags;

	asm("pushfl; popl %0" : "=r" (eflags));
	asm("cli");
	old_icu1_mask = inb(IO_ICU1 + 1);
	old_icu2_mask = inb(IO_ICU2 + 1);
	save_idt_npxintr = idt[npx_intrno];
	outb(IO_ICU1 + 1, old_icu1_mask & ~(IRQ_SLAVE | npx0mask));
	outb(IO_ICU2 + 1, old_icu2_mask & ~(npx0mask >> 8));
	idt[npx_intrno] = npx_idt_saveintr;
	asm("sti");
	load_cr0(rcr0() & ~CR0_EM);	/* stop emulating */
	asm("fnsave %0" : "=m" (*addr));
	asm("fwait");
	load_cr0(rcr0() | CR0_EM);	/* start emulating */
	npxproc = NULL;
	asm("cli");
	icu1_mask = inb(IO_ICU1 + 1);	/* masks may have changed */
	icu2_mask = inb(IO_ICU2 + 1);
	outb(IO_ICU1 + 1,
	     (icu1_mask & ~npx0mask) | (old_icu1_mask & npx0mask));
	outb(IO_ICU2 + 1,
	     (icu2_mask & ~(npx0mask >> 8))
	     | (old_icu2_mask & (npx0mask >> 8)));
	idt[npx_intrno] = save_idt_npxintr;
	asm("pushl %0; popfl" : : "r" (eflags));	/* back to how it was */
}
#endif
