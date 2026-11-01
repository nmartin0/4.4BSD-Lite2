/*-
 * Copyright (c) 1994 Christopher G. Demetriou
 * Copyright (c) 1993 Theo de Raadt
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)exec.h	8.4 (Berkeley) 2/19/95
 */

#include <machine/exec.h>

/*
 * The following structure is found at the top of the user stack of each
 * user process. The ps program uses it to locate argv and environment
 * strings. Programs that wish ps to display other information may modify
 * it; normally ps_argvstr points to argv[0], and ps_nargvstr is the same
 * as the program's argc. The fields ps_envstr and ps_nenvstr are the
 * equivalent for the environment.
 *
 * AI-ONLY NOTE: ps_argvstr and ps_envstr point at the argv and envp
 * vectors, char **, as NetBSD's revision 1.56 has them (Charles M.
 * Hannum, 16 May 1995, NetBSD 1.1, "Change ps_{argv,env}str to
 * char**"), and FreeBSD 2.2's; Berkeley's pointed at the text of the
 * first string, and libkvm found the rest by counting NULs. This
 * comment's wording is that revision's too.
 */
struct ps_strings {
	char	**ps_argvstr;	/* first of 0 or more argument strings */
	int	ps_nargvstr;	/* the number of argument strings */
	char	**ps_envstr;	/* first of 0 or more environment strings */
	int	ps_nenvstr;	/* the number of environment strings */
};

/*
 * Address of ps_strings structure (in user space).
 */
#define	PS_STRINGS \
	((struct ps_strings *)(USRSTACK - sizeof(struct ps_strings)))

/*
 * AI-ONLY NOTE (2026-10-31; docs/provenance/exec-stack.md): everything
 * below is NetBSD 1.0's sys/exec.h (rev 1.43.2.2, 15 August 1994) --
 * the exec framework that kern/kern_exec.c, kern/exec_subr.c,
 * kern/exec_conf.c, kern/exec_script.c and kern/exec_elf.c are built
 * on -- with the two Copyright lines above that its file carries
 * besides the Regents'. Six changes to fit this tree:
 *
 *  - all of it inside `#ifdef KERNEL'. NetBSD 1.0 leaves the
 *    structures outside it, where they reach every program that
 *    includes this header for struct ps_strings (libkvm, ps, w);
 *    nothing outside the kernel uses them, and Lite2's header gave
 *    those programs ps_strings alone.
 *  - struct execve_args is not taken: Lite2's own sys/syscallargs.h
 *    already declares it, with the same three members.
 *  - ep_setup and ep_emul are not taken. ep_setup is a hook for the
 *    compatibility formats (NetBSD 1.1 removed it); ep_emul names an
 *    emulation, and this tree has no emulation switch.
 *  - vmcmd_map_pagedvn is not declared: it maps a.out text demand-
 *    paged, and NetBSD 1.1's ELF loader reads its segments in
 *    (vmcmd_map_readvn), so kern/exec_subr.c does not carry it.
 *  - NetBSD's last two lines, `#include <sys/exec_aout.h>' and
 *    `#include <machine/exec.h>', are not taken: exec_aout.h is not,
 *    and this file already includes <machine/exec.h> at its top, as
 *    Lite2's does.
 *  - exec_aout_setup_stack is declared here, since it came over from
 *    NetBSD's exec_aout.c, alone, into kern/exec_subr.c; NetBSD
 *    declares it in sys/exec_aout.h, which is not taken.
 */
#ifdef KERNEL
/*
 * Below the PS_STRINGS and sigtramp, we may require a gap on the stack
 * (used to copyin/copyout various emulation data structures). The base
 * address of this gap may need alignment, so use this to reference it:
 *	(caddr_t)ALIGN(PS_STRINGS - szsigcode - STACKGAPLEN);
 */
#if defined(COMPAT_SUNOS) || defined(COMPAT_ULTRIX) || defined(COMPAT_IBCS2)
#define	STACKGAPLEN	400	/* plenty enough for now */
#else
#define	STACKGAPLEN	0
#endif

/*
 * the following structures allow execve() to put together processes
 * in a more extensible and cleaner way.
 *
 * the exec_package struct defines an executable being execve()'d.
 * it contains the header, the vmspace-building commands, the vnode
 * information, and the arguments associated with the newly-execve'd
 * process.
 *
 * the exec_vmcmd struct defines a command description to be used
 * in creating the new process's vmspace.
 */

struct proc;
struct exec_package;

typedef int (*exec_makecmds_fcn) __P((struct proc *, struct exec_package *));

struct execsw {
	u_int	es_hdrsz;		/* size of header for this format */
	exec_makecmds_fcn es_check;	/* function to check exec format */
};

/* exec vmspace-creation command set; see below */
struct exec_vmcmd_set {
	u_int	evs_cnt;
	u_int	evs_used;
	struct	exec_vmcmd *evs_cmds;
};

#define	EXEC_DEFAULT_VMCMD_SETSIZE	5	/* # of cmds in set to start */

struct exec_package {
	char	*ep_name;		/* file's name */
	void	*ep_hdr;		/* file's exec header */
	u_int	ep_hdrlen;		/* length of ep_hdr */
	u_int	ep_hdrvalid;		/* bytes of ep_hdr that are valid */
	struct nameidata *ep_ndp;	/* namei data pointer for lookups */
	struct	exec_vmcmd_set ep_vmcmds;  /* vmcmds used to build vmspace */
	struct	vnode *ep_vp;		/* executable's vnode */
	struct	vattr *ep_vap;		/* executable's attributes */
	u_long	ep_taddr;		/* process's text address */
	u_long	ep_tsize;		/* size of process's text */
	u_long	ep_daddr;		/* process's data(+bss) address */
	u_long	ep_dsize;		/* size of process's data(+bss) */
	u_long	ep_maxsaddr;		/* proc's max stack addr ("top") */
	u_long	ep_minsaddr;		/* proc's min stack addr ("bottom") */
	u_long	ep_ssize;		/* size of process's stack */
	u_long	ep_entry;		/* process's entry point */
	u_int	ep_flags;		/* flags; see below. */
	char	**ep_fa;		/* a fake args vector for scripts */
	int	ep_fd;			/* a file descriptor we're holding */
};
#define	EXEC_INDIR	0x0001		/* script handling already done */
#define	EXEC_HASFD	0x0002		/* holding a shell script */
#define	EXEC_HASARGL	0x0004		/* has fake args vector */
#define	EXEC_SKIPARG	0x0008		/* don't copy user-supplied argv[0] */
#define	EXEC_DESTR	0x0010		/* destructive ops performed */

struct exec_vmcmd {
	int	(*ev_proc) __P((struct proc *p, struct exec_vmcmd *cmd));
				/* procedure to run for region of vmspace */
	u_long	ev_len;		/* length of the segment to map */
	u_long	ev_addr;	/* address in the vmspace to place it at */
	struct	vnode *ev_vp;	/* vnode pointer for the file w/the data */
	u_long	ev_offset;	/* offset in the file for the data */
	u_int	ev_prot;	/* protections for segment */
};

/*
 * funtions used either by execve() or the various cpu-dependent execve()
 * hooks.
 */
void	kill_vmcmd		__P((struct exec_vmcmd **));
int	exec_makecmds		__P((struct proc *, struct exec_package *));
int	exec_runcmds		__P((struct proc *, struct exec_package *));
void	vmcmdset_extend		__P((struct exec_vmcmd_set *));
void	kill_vmcmds		__P((struct exec_vmcmd_set *evsp));
int	vmcmd_map_readvn	__P((struct proc *, struct exec_vmcmd *));
int	vmcmd_map_zero		__P((struct proc *, struct exec_vmcmd *));
int	exec_aout_setup_stack	__P((struct proc *, struct exec_package *));

#ifdef DEBUG
void	new_vmcmd __P((struct exec_vmcmd_set *evsp,
		    int (*proc) __P((struct proc *p, struct exec_vmcmd *)),
		    u_long len, u_long addr, struct vnode *vp, u_long offset,
		    u_int prot));
#define	NEW_VMCMD(evsp,proc,len,addr,vp,offset,prot) \
	new_vmcmd(evsp,proc,len,addr,vp,offset,prot);
#else	/* DEBUG */
#define	NEW_VMCMD(evsp,proc,len,addr,vp,offset,prot) { \
	struct exec_vmcmd *vcp; \
	if ((evsp)->evs_used >= (evsp)->evs_cnt) \
		vmcmdset_extend(evsp); \
	vcp = &(evsp)->evs_cmds[(evsp)->evs_used++]; \
	vcp->ev_proc = (proc); \
	vcp->ev_len = (len); \
	vcp->ev_addr = (addr); \
	if ((vcp->ev_vp = (vp)) != NULLVP) \
                VREF(vp); \
        vcp->ev_offset = (offset); \
        vcp->ev_prot = (prot); \
}
#endif /* EXEC_DEBUG */

/*
 * Exec function switch:
 *
 * Note that each makecmds function is responsible for loading the
 * exec package with the necessary functions for any exec-type-specific
 * handling.
 *
 * Functions for specific exec types should be defined in their own
 * header file.
 */
extern struct	execsw execsw[];
extern int	nexecs;
extern int	exec_maxhdrsz;

#endif /* KERNEL */
