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
 *	@(#)DEFS.h	8.1 (Berkeley) 6/4/93
 */

/*
 * AI-ONLY NOTE: C symbols go through _C_LABEL: plain under ELF, where
 * C names carry no leading underscore, and _name otherwise. The block
 * is NetBSD 1.5's <machine/asm.h> (2000); OpenBSD's i386 asm.h says
 * the same, `#define _C_LABEL(name) name'. Its __STDC__ arm goes
 * unused here, since assembly is preprocessed with -traditional-cpp,
 * as NetBSD 1.5 does.
 *
 * Both of them keep this in the kernel's <machine/asm.h>, and their
 * libc DEFS.h is one line including it. This tree has no i386 asm.h,
 * so the block lives here and in SYS.h, as ENTRY already does. When
 * the kernel brings an asm.h, these can include it instead.
 */
#ifdef __ELF__
#define	_C_LABEL(x)	x
#else
#ifdef __STDC__
#define	_C_LABEL(x)	_ ## x
#else
#define	_C_LABEL(x)	_/**/x
#endif
#endif

#ifdef PROF
#define	ENTRY(x)	.globl _C_LABEL(x); _C_LABEL(x):  \
			.data; 1:; .long 0; .text; lea 1b,%eax ; call mcount
#define	ASENTRY(x)	.globl x; x: \
			.data; 1:; .long 0; .text; lea 1b,%eax ; call mcount
#else
#define	ENTRY(x)	.globl _C_LABEL(x); _C_LABEL(x): 
#define	ASENTRY(x)	.globl x; x: 
#endif
