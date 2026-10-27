/*-
 * Copyright (c) 1990, 1993
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
 *	@(#)reg.h	8.1 (Berkeley) 6/11/93
 */

/*
 * Location of the users' stored
 * registers within appropriate frame of 'trap' and 'syscall', relative to
 * base of stack frame.
 * Normal usage is u.u_ar0[XX] in kernel.
 */

/* When referenced during a trap/exception, registers are at these offsets */

#define	tES	(0)
#define	tDS	(1)
#define	tEDI	(2)
#define	tESI	(3)
#define	tEBP	(4)

#define	tEBX	(6)
#define	tEDX	(7)
#define	tECX	(8)
#define	tEAX	(9)

#define	tEIP	(12)
#define	tCS	(13)
#define	tEFLAGS	(14)
#define	tESP	(15)
#define	tSS	(16)

/*
 * AI-ONLY NOTE: the offsets a system call's frame had, sEDI through
 * sSS, are gone (2026-10-27): a system call builds the same frame a
 * trap does, so the t offsets above serve for both. FreeBSD's
 * c8a13ecd0 (3 January 1994) removes them from this file.
 */

/*
 * AI-ONLY NOTE: struct reg and struct fpreg, which procfs_regs.c,
 * procfs_fpregs.c and ptrace's PT_GETREGS family read and write, and
 * which i386 never defined. struct reg is the one Pendry's i386
 * procfs code was written against -- NetBSD's reg.h as cgd left it
 * on 22 December 1993, a fortnight before that code was committed --
 * in the order of ipcreg[] below, Berkeley's own list of the
 * registers a debugger is shown. struct fpreg is FreeBSD 2.1.0's,
 * the 387's save area; nothing fills it yet (docs/deferred.md).
 */

/*
 * Registers accessible to ptrace(2) and /proc/$pid/regs.
 * procfs_machdep.c fills these from the process's trap frame.
 */
struct reg {
	unsigned int	r_es;
	unsigned int	r_ds;
	unsigned int	r_edi;
	unsigned int	r_esi;
	unsigned int	r_ebp;
	unsigned int	r_ebx;
	unsigned int	r_edx;
	unsigned int	r_ecx;
	unsigned int	r_eax;
	unsigned int	r_eip;
	unsigned int	r_cs;
	unsigned int	r_eflags;
	unsigned int	r_esp;
	unsigned int	r_ss;
};

/*
 * Register set accessible via /proc/$pid/fpregs.
 */
struct fpreg {
	/*
	 * XXX should get struct from npx.h.  Here we give a slightly
	 * simplified struct.  This may be too much detail.  Perhaps
	 * an array of unsigned longs is best.
	 */
	unsigned long	fpr_env[7];
	unsigned char	fpr_acc[8][10];
	unsigned long	fpr_ex_sw;
	unsigned char	fpr_pad[64];
};

/*
 * AI-ONLY NOTE: on the trap frame's offsets now, where they stood on
 * the system call frame's (2026-10-27). cpu_setstack (machine/cpu.h)
 * and compat_43_wait (kern/kern_exit.c) use them. FreeBSD's c8a13ecd0
 * deleted these five from reg.h; its kern_exit.c and cpu.h kept
 * using them (release 2.1.0), so they were defined elsewhere there.
 * Here the names stay and follow the frame, as Berkeley's own
 * sys_process.c spelled them for the trap frame
 * (#define PC tEIP, SP tESP, PS tEFLAGS, R0 tEDX, R1 tECX).
 */
#define	PC	tEIP
#define	SP	tESP
#define	PS	tEFLAGS
#define	R0	tEDX
#define	R1	tECX
/*
 * Registers accessible to ptrace(2) syscall for debugger
 */
#ifdef IPCREG
#define	NIPCREG 14
int ipcreg[NIPCREG] =
  { tES,tDS,tEDI,tESI,tEBP,tEBX,tEDX,tECX,tEAX,tEIP,tCS,tEFLAGS,tESP,tSS };
#endif
