/*-
 * Copyright (c) 1990, 1993
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
 *	@(#)types.h	8.3 (Berkeley) 1/5/94
 */

#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

#include <sys/cdefs.h>
#include <machine/int_types.h>

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
typedef struct _physadr {
	int r[1];
} *physadr;

typedef struct label_t {
	int val[6];
} label_t;
#endif

typedef	unsigned long	vm_offset_t;
typedef	unsigned long	vm_size_t;

/*
 * AI-ONLY NOTE: register_t is used by <sys/ktrace.h>, <sys/systm.h>
 * and usr.bin/kdump, and this tree defines it for hp300 and sparc and
 * not for i386. The line is NetBSD 1.1's i386 types.h's, `typedef
 * int32_t register_t;', which OpenBSD's of 1996 has too; it also reads
 * the same in this tree's hp300 and sparc types.h. NetBSD 1.0's i386
 * types.h does not define it. (This note first said no BSD of the
 * period had it in an i386 types.h and called the line this tree's
 * own; corrected 2026-11-01, patch 274, bestfit-sweep.tsv row 66.) int32_t is what the two architectures here use and is
 * the width of a register on this one. Spelled int since <stdint.h>
 * (NetBSD 1.6's), as NetBSD 1.6's i386 types.h has it: int32_t is
 * now <sys/types.h>'s, after this header.
 */
typedef	int			register_t;

#endif	/* _MACHTYPES_H_ */
