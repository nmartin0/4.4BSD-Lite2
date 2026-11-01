/*	$NetBSD: exec_conf.c,v 1.14 1995/10/10 01:26:50 mycroft Exp $	*/

/*
 * Copyright (c) 1993, 1994 Christopher G. Demetriou
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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
 * AI-ONLY NOTE (2026-10-31; docs/provenance/exec-stack.md): NetBSD 1.1's
 * kern/exec_conf.c, rev 1.14, cut to the two formats this tree runs:
 * `#!' scripts and ELF. NetBSD 1.1 defines EXEC_ELF only for its SVR4
 * and Linux emulations, its own programs being a.out; this tree's are
 * ELF (GNU ld has no i386 a.out emulation here, csu-elf.md), so it is
 * defined outright. Taken out: EXEC_AOUT and its entry (no a.out
 * loader is taken), and the ECOFF, LKM and COMPAT_* entries and their
 * includes, for code this tree does not have. Nothing is reordered.
 */
#define	EXEC_SCRIPT			/* XXX */
#define EXEC_ELF

#include <sys/param.h>
#include <sys/exec.h>

#ifdef EXEC_SCRIPT
#include <sys/exec_script.h>
#endif

#ifdef EXEC_ELF
#include <sys/exec_elf.h>
#endif

struct execsw execsw[] = {
#ifdef EXEC_SCRIPT
	{ MAXINTERP, exec_script_makecmds, },		/* shell scripts */
#endif
#ifdef EXEC_ELF
	{ ELF_HDR_SIZE, exec_elf_makecmds, },	/* elf binaries */
#endif
};
int nexecs = (sizeof execsw / sizeof(*execsw));
int exec_maxhdrsz;
