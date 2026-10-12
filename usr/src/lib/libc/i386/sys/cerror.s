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
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)cerror.s	8.1 (Berkeley) 6/4/93"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"
/* AI-ONLY NOTE: _C_LABEL, for ELF naming; see SYS.h. */

	.globl	_C_LABEL(errno)
/*
 * AI-ONLY NOTE: %edx as well as %eax, so a failing call that returns
 * a quad -- lseek's off_t, through __syscall -- returns (off_t)-1 and
 * not (whatever %edx held : 0xffffffff). The kernel's error path in
 * i386/i386/trap.c writes only %eax and the carry bit, so %edx is the
 * caller's own value coming back.
 *
 * Every port in this tree whose libc was brought to 4.4BSD's 64-bit
 * off_t does this: hp300's and luna68k's cerror.s set d0 and d1,
 * sparc's %o0 and %o1, mips's v0 and v1. vax's and tahoe's set one
 * register, as this file did; those two ports were not carried
 * forward. Measured: `dd if=/dev/rwd0c ... | pax' listed one member
 * and stopped, because pax's ar_io.c:175 tells a pipe from a file by
 * `lseek(arfd, 0, SEEK_CUR) == -1 && errno == ESPIPE', the high word
 * was not -1, and pax took the pipe for a seekable file.
 */
cerror:
	movl	%eax,_C_LABEL(errno)
	movl	$-1,%eax
	movl	$-1,%edx
	ret
