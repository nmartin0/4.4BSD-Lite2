/*	$Id: ccconfig.h,v 1.3 2016/01/06 16:14:54 ragge Exp $	*/

/*
 * Copyright (c) 2004 Anders Magnusson (ragge@ludd.luth.se).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Various settings that controls how the C compiler works.
 */

/*
 * AI-ONLY NOTE: this is pcc's target for LiteBSD, Serge Vakulenko's
 * 4.4BSD-Lite on MIPS, made a target for this tree's i386: the closest
 * of pcc's operating systems to this one -- 4.4BSD-Lite, static,
 * crt0.o and libc -- where NetBSD's names a dynamic linker and crt
 * files this csu does not have. Changed: __LiteBSD__ goes, since this
 * is not LiteBSD; unix and i386 are predefined as 4.4BSD-Lite2's own
 * compiler predefines them for the i386 (contrib/gcc-2.3.3/config/
 * i386bsd.h, CPP_PREDEFINES), since Lite2's sources test them --
 * <setjmp.h>, <rpc/auth.h>, strtod.c; __i386__ and the type names
 * are NetBSD's os/netbsd/ccconfig.h's for mach_i386; MIPS's go. The
 * startup files are this tree's lib/csu, NetBSD 1.6's: crt0.o with
 * crtbegin.o and crtend.o around a program, static or not, and no
 * crti.o or crtn.o. The libraries are libc alone: there is no
 * libgcc, and libc's quad routines (__divdi3 and the rest) are what
 * pcc's i386 code calls for 64-bit division.
 */
/* common cpp predefines */
#define CPPADD  { "-Dunix", "-D__ELF__", "-D__unix__", \
	NULL, }
#define	DYNLINKER { NULL }

#ifdef LANG_F77
#define F77LIBLIST { "-L/usr/local/lib", "-lF77", "-lI77", "-lm", "-lc", NULL };
#endif

#define	CPPMDADD { "-D__i386__", "-Di386", NULL, }
#define	PCC_SIZE_TYPE		"unsigned int"
#define	PCC_PTRDIFF_TYPE	"int"

#define DEFLIBS { "-lc", NULL }

#define CRTBEGIN_T	"crtbegin.o"
#define CRTEND_T	"crtend.o"
#define CRTI		0
#define CRTN		0
