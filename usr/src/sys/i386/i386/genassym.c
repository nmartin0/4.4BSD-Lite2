/*-
 * Copyright (c) 1982, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *	@(#)genassym.c	8.2 (Berkeley) 9/23/93
 */

#ifndef lint
static char sccsid[] = "@(#)genassym.c	8.2 (Berkeley) 9/23/93";
#endif /* not lint */

#define KERNEL

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>
#include <machine/cpu.h>
#include <machine/trap.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <sys/syscall.h>
#include <vm/vm.h>
/*
 * AI-ONLY NOTE: machine/pmap.h, for the page directory slots.
 * locore.s wrote its own copies of them -- SYSPDROFF, PDRPDROFF,
 * PPDROFF -- and its own copies of the five addresses derived from
 * them, because the assembler cannot read a C header. Exporting the
 * slots through assym.s, which config already generates and which
 * already carries UPAGES, NBPG and CLSIZE this way, means there is
 * one definition rather than two.
 *
 * NetBSD 1.0's genassym.c:50 includes this header and :75-85 export
 * UPTDI, PTDPTDI, KPTDI, APTDPTDI and PDSHIFT for the same reason;
 * its locore.s:124-136 then computes every address from them.
 */
#include <machine/pmap.h>
#include <sys/user.h>

/*
 * AI-ONLY NOTE: ASSYM rather than printf, as FreeBSD 5.0's genassym.c
 * has it, for kern/genassym.sh, which reads the values back out of
 * this file's object with nm: this file is compiled and not run, so
 * a compiler for the target need not make programs the build machine
 * can run. Each `&p->p_forw' of Lite2's became an offsetof() of the
 * same member, as FreeBSD's are written; offsetof is defined as this
 * tree's <stddef.h> defines it, which the kernel cannot include, and
 * as FreeBSD 2.1.0's diskslice_machdep.c, in this tree, defines it
 * for itself.
 */
#include <sys/assym.h>

#define	offsetof(type, member)	((size_t)(&((type *)0)->member))

ASSYM(I386_CR3PAT, I386_CR3PAT);

/*
 * AI-ONLY NOTE: MD_EIP and MD_ESP, the byte offsets of
 * md_regs[tEIP] and md_regs[tESP] (2026-10-27: they were sEIP and sESP
 * while a system call had a frame of its own).
 *
 * i386/locore.s builds the frame it lrets to user with and
 * needs both out of it. It named them 11*4 and 9*4, which are
 * the only raw structure offsets in that file: everything else
 * goes through this file, PCB_ONFAULT and PCB_CR3 among
 * them. <machine/reg.h> is already included above, so the
 * indices are in scope and the arithmetic belongs here rather
 * than in the assembly, where a change to reg.h would not reach
 * it.
 */
ASSYM(MD_EIP, tEIP * sizeof(int));
ASSYM(MD_ESP, tESP * sizeof(int));
/*
 * AI-ONLY NOTE: TF_TRAPNO, TF_ERR and TF_EFLAGS, for IDTVEC(syscall)
 * in locore.s, which fills in the frame's trap number, error code and
 * flags itself. FreeBSD's genassym.c and NetBSD 1.0's define them
 * the same way, and their syscall entries use them.
 */
ASSYM(TF_TRAPNO, offsetof(struct trapframe, tf_trapno));
ASSYM(TF_ERR, offsetof(struct trapframe, tf_err));
ASSYM(TF_EFLAGS, offsetof(struct trapframe, tf_eflags));
ASSYM(UDOT_SZ, sizeof(struct user));
ASSYM(P_FORW, offsetof(struct proc, p_forw));
ASSYM(P_BACK, offsetof(struct proc, p_back));
ASSYM(P_VMSPACE, offsetof(struct proc, p_vmspace));
ASSYM(VM_PMAP, offsetof(struct vmspace, vm_pmap));
ASSYM(P_ADDR, offsetof(struct proc, p_addr));
ASSYM(P_PRIORITY, offsetof(struct proc, p_priority));
ASSYM(P_STAT, offsetof(struct proc, p_stat));
ASSYM(P_WCHAN, offsetof(struct proc, p_wchan));
ASSYM(P_FLAG, offsetof(struct proc, p_flag));
ASSYM(P_PID, offsetof(struct proc, p_pid));
ASSYM(SSLEEP, SSLEEP);
ASSYM(SRUN, SRUN);
ASSYM(V_SWTCH, offsetof(struct vmmeter, v_swtch));
ASSYM(V_TRAP, offsetof(struct vmmeter, v_trap));
ASSYM(V_SYSCALL, offsetof(struct vmmeter, v_syscall));
ASSYM(V_INTR, offsetof(struct vmmeter, v_intr));
ASSYM(V_SOFT, offsetof(struct vmmeter, v_soft));
ASSYM(V_FAULTS, offsetof(struct vmmeter, v_faults));
ASSYM(UPAGES, UPAGES);
ASSYM(HIGHPAGES, HIGHPAGES);
ASSYM(CLSIZE, CLSIZE);
ASSYM(NBPG, NBPG);
ASSYM(UPTDI, UPTDI);
ASSYM(PTDPTDI, PTDPTDI);
ASSYM(KPTDI_FIRST, KPTDI_FIRST);
ASSYM(KPTDI_LAST, KPTDI_LAST);
ASSYM(NKPDE, NKPDE);
ASSYM(PD_SHIFT, PD_SHIFT);
ASSYM(NPTEPG, NPTEPG);
ASSYM(PGSHIFT, PGSHIFT);
ASSYM(PDRSHIFT, PDRSHIFT);
ASSYM(SYSPTSIZE, SYSPTSIZE);
ASSYM(USRPTSIZE, USRPTSIZE);
ASSYM(USRIOSIZE, USRIOSIZE);
#ifdef SYSVSHM
ASSYM(SHMMAXPGS, SHMMAXPGS);
#endif
ASSYM(USRSTACK, USRSTACK);
ASSYM(MSGBUFPTECNT, btoc(sizeof (struct msgbuf)));
ASSYM(NMBCLUSTERS, NMBCLUSTERS);
ASSYM(MCLBYTES, MCLBYTES);
ASSYM(PCB_LINK, offsetof(struct pcb, pcb_tss.tss_link));
ASSYM(PCB_ESP0, offsetof(struct pcb, pcb_tss.tss_esp0));
ASSYM(PCB_SS0, offsetof(struct pcb, pcb_tss.tss_ss0));
ASSYM(PCB_ESP1, offsetof(struct pcb, pcb_tss.tss_esp1));
ASSYM(PCB_SS1, offsetof(struct pcb, pcb_tss.tss_ss1));
ASSYM(PCB_ESP2, offsetof(struct pcb, pcb_tss.tss_esp2));
ASSYM(PCB_SS2, offsetof(struct pcb, pcb_tss.tss_ss2));
ASSYM(PCB_CR3, offsetof(struct pcb, pcb_tss.tss_cr3));
ASSYM(PCB_EIP, offsetof(struct pcb, pcb_tss.tss_eip));
ASSYM(PCB_EFLAGS, offsetof(struct pcb, pcb_tss.tss_eflags));
ASSYM(PCB_EAX, offsetof(struct pcb, pcb_tss.tss_eax));
ASSYM(PCB_ECX, offsetof(struct pcb, pcb_tss.tss_ecx));
ASSYM(PCB_EDX, offsetof(struct pcb, pcb_tss.tss_edx));
ASSYM(PCB_EBX, offsetof(struct pcb, pcb_tss.tss_ebx));
ASSYM(PCB_ESP, offsetof(struct pcb, pcb_tss.tss_esp));
ASSYM(PCB_EBP, offsetof(struct pcb, pcb_tss.tss_ebp));
ASSYM(PCB_ESI, offsetof(struct pcb, pcb_tss.tss_esi));
ASSYM(PCB_EDI, offsetof(struct pcb, pcb_tss.tss_edi));
ASSYM(PCB_ES, offsetof(struct pcb, pcb_tss.tss_es));
ASSYM(PCB_CS, offsetof(struct pcb, pcb_tss.tss_cs));
ASSYM(PCB_SS, offsetof(struct pcb, pcb_tss.tss_ss));
ASSYM(PCB_DS, offsetof(struct pcb, pcb_tss.tss_ds));
ASSYM(PCB_FS, offsetof(struct pcb, pcb_tss.tss_fs));
ASSYM(PCB_GS, offsetof(struct pcb, pcb_tss.tss_gs));
ASSYM(PCB_LDT, offsetof(struct pcb, pcb_tss.tss_ldt));
ASSYM(PCB_IOOPT, offsetof(struct pcb, pcb_tss.tss_ioopt));
ASSYM(NKMEMCLUSTERS, NKMEMCLUSTERS);
ASSYM(U_PROF, offsetof(struct user, u_stats.p_prof));
ASSYM(U_PROFSCALE, offsetof(struct user, u_stats.p_prof.pr_scale));
ASSYM(PR_BASE, offsetof(struct uprof, pr_base));
ASSYM(PR_SIZE, offsetof(struct uprof, pr_size));
ASSYM(PR_OFF, offsetof(struct uprof, pr_off));
ASSYM(PR_SCALE, offsetof(struct uprof, pr_scale));
ASSYM(RU_MINFLT, offsetof(struct rusage, ru_minflt));
ASSYM(PCB_FLAGS, offsetof(struct pcb, pcb_flags));
ASSYM(PCB_SAVEFPU, offsetof(struct pcb, pcb_savefpu));
ASSYM(FP_WASUSED, FP_WASUSED);
ASSYM(FP_NEEDSSAVE, FP_NEEDSSAVE);
ASSYM(FP_NEEDSRESTORE, FP_NEEDSRESTORE);
ASSYM(FP_USESEMC, FP_USESEMC);
ASSYM(PCB_SAVEEMC, offsetof(struct pcb, pcb_saveemc));
ASSYM(PCB_CMAP2, offsetof(struct pcb, pcb_cmap2));
ASSYM(PCB_SIGC, offsetof(struct pcb, pcb_sigc));
ASSYM(PCB_IML, offsetof(struct pcb, pcb_iml));
ASSYM(PCB_ONFAULT, offsetof(struct pcb, pcb_onfault));

ASSYM(B_READ, B_READ);
ASSYM(ENOENT, ENOENT);
ASSYM(EFAULT, EFAULT);
ASSYM(ENAMETOOLONG, ENAMETOOLONG);
