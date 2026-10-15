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
 *	@(#)psl.h	8.1 (Berkeley) 6/11/93
 */

/*
 * 386 processor status longword.
 */
#define	PSL_C		0x00000001	/* carry bit */
#define	PSL_PF		0x00000004	/* parity bit */
#define	PSL_AF		0x00000010	/* bcd carry bit */
#define	PSL_Z		0x00000040	/* zero bit */
#define	PSL_N		0x00000080	/* negative bit */
#define	PSL_ALLCC	0x000000d5	/* all cc bits - unlikely */
#define	PSL_T		0x00000100	/* trace enable bit */
#define	PSL_I		0x00000200	/* interrupt enable bit */
#define	PSL_D		0x00000400	/* string instruction direction bit */
#define	PSL_V		0x00000800	/* overflow bit */
#define	PSL_IOPL	0x00003000	/* i/o priviledge level enable */
#define	PSL_NT		0x00004000	/* nested task bit */
#define	PSL_RF		0x00010000	/* restart flag bit */
#define	PSL_VM		0x00020000	/* virtual 8086 mode bit */
/*
 * AI-ONLY NOTE: PSL_AC and PSL_ID, which this file stopped short of.
 * They are architectural: bit 18 is alignment checking, which no 386
 * has, and bit 21 is the identification flag, which no early 486 has.
 * Toggling each is how locore.s now tells the three apart, and bit 21
 * is what makes cpuid safe to execute -- on a 386 it faults. Same
 * values and same position as NetBSD 1.0's machine/psl.h, lines 59
 * and 62.
 */
#define	PSL_AC		0x00040000	/* alignment checking */
#define	PSL_ID		0x00200000	/* identification bit */
/*
 * AI-ONLY NOTE: the virtual-8086 interrupt flags, bits 19 and 20, for
 * PSL_USERSTATIC below; OpenBSD's psl.h as first imported (1995).
 */
#define	PSL_VIF		0x00080000	/* virtual interrupt enable flag */
#define	PSL_VIP		0x00100000	/* virtual interrupt pending flag */

/*
 * AI-ONLY NOTE: PSL_MBZ, PSL_USERSET and PSL_USERCLR are NetBSD 1.0's
 * (machine/psl.h, lines 65 to 68), less PSL_VIP and PSL_VIF in
 * PSL_USERCLR, as FreeBSD 2.0's. Lite2's marked the carry, interrupt and
 * other live flags as must-be-zero, required IOPL 3 of a user process
 * and forbade it interrupts -- the reverse of the flags a user
 * process runs with here, PSL_I set and IOPL 0 (0x286 is typical).
 * Nothing read them until procfs_write_regs, which then refused
 * every process's own flags.
 */
#define	PSL_MBZ		0xffc08028	/* must be zero bits */
#define	PSL_MBO		0x00000002	/* must be one bits */

#define	PSL_USERSET	(PSL_MBO | PSL_I)
#define	PSL_USERCLR	(PSL_MBZ | PSL_NT | PSL_VM)

/*
 * AI-ONLY NOTE: the bits a process may never change, whatever it
 * asks: OpenBSD's PSL_USERSTATIC, as in its first tree (October 1995,
 * NetBSD-current of that month) and still in its 2.4, without VM86.
 * sigreturn, the /proc register hooks and ptrace's PT_WRITE_U refuse
 * new flags that differ from the process's own in any of these.
 */
#define	PSL_USERSTATIC	(PSL_MBO | PSL_MBZ | PSL_I | PSL_IOPL | PSL_NT | \
			 PSL_VM | PSL_VIF | PSL_VIP)
