/*-
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
 *	@(#)stdarg.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _STDARG_H_
#define	_STDARG_H_

/*
 * The mechanism is the compiler's.  Berkeley's version of this file
 * walked the argument list by hand: va_list was char *, va_start took
 * the address of the last named parameter and added its promoted
 * size, and va_arg stepped that pointer.  That is only correct while
 * the compiler keeps a named parameter in the slot the calling
 * convention gave it, which GCC stopped guaranteeing: with -O it may
 * copy a parameter whose address is taken into a stack slot of its
 * own choosing, and then &last + 4 points at an unrelated local
 * instead of at the first variable argument.  Reading one is silent
 * -- it yields whatever that local holds -- so a function miscompiled
 * this way fails far from the header, and only in the functions where
 * GCC happened to make that choice.
 *
 * __builtin_va_start is how a compiler is told what is wanted rather
 * than shown a trick that used to work, and it is where every
 * descendant of this file ended up: NetBSD's sys/sys/stdarg.h still
 * carries this file's SCCS identifier and defines the same three
 * macros over the builtins, FreeBSD's sys/sys/_stdarg.h does, and so
 * does OpenBSD's sys/sys/stdarg.h.  The bodies no longer depend on
 * the machine, which is why NetBSD collapsed the per-port copies into
 * one machine-independent header; this tree keeps them per port
 * because include/Makefile installs machine/stdarg.h.
 *
 * __va_promote is gone: nothing in this tree used it but va_start.
 * va_arg no longer differs between the kernel and the userland.  The
 * old userland form called abort() for a type narrower than int,
 * which the compiler now rejects outright at compile time.
 */

/*
 * GCC before 2.96 has no __builtin_va_list.  For it, va_list is a
 * pointer again, but started where the compiler says the variable
 * arguments begin, by __builtin_next_arg, not from the address of the
 * last named parameter: NetBSD 1.6's <machine/stdarg.h> for the i386,
 * which it built with GCC 2.95.3, written with <sys/cdefs.h>'s test of
 * the GCC version.
 */
#if defined(__GNUC__) && (__GNUC__ > 2 || \
	(__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
typedef __builtin_va_list va_list;

#define	va_start(ap, last)	__builtin_va_start((ap), (last))
#define	va_arg(ap, type)	__builtin_va_arg((ap), type)
#define	va_end(ap)		__builtin_va_end((ap))
#define	__va_copy(dst, src)	__builtin_va_copy((dst), (src))
#else
typedef char *va_list;

#define	__va_size(type) \
	(((sizeof(type) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

#define	va_start(ap, last) \
	((ap) = (va_list)__builtin_next_arg(last))

#define	va_arg(ap, type) \
	(*(type *)(void *)((ap) += __va_size(type), (ap) - __va_size(type)))

#define	va_end(ap)

#define	__va_copy(dst, src)	((dst) = (src))
#endif

#endif /* !_STDARG_H_ */
