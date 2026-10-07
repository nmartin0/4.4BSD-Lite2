/*-
 * Copyright (c) 1982, 1986, 1991, 1993
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
 *	from: @(#)kern_exec.c	8.1 (Berkeley) 6/10/93
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/exec.h>
#include <sys/exec_elf.h>
#include <sys/ktrace.h>
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>

#include <machine/cpu.h>
#include <machine/reg.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>

int elf_check_header __P((Elf32_Ehdr *, int));
int elf_read_from __P((struct proc *, struct vnode *, u_long, caddr_t, int));
int elf_load_psection __P((struct proc *, struct vnode *, Elf32_Phdr *,
	u_long *, u_long *, int *));

/*
 * exec system call
 */
struct execve_args {
	char	*fname;
	char	**argp;
	char	**envp;
};
/*
 * AI-ONLY NOTE: written, not restored. This is one of the thirty-five
 * bodies the AT&T settlement removed -- docs/provenance/missing.md --
 * so the signature, the call sites and struct execve_args above are
 * Berkeley's and the body is not.
 *
 * The shape is 4.4BSD's, read from the encumbered kern_exec.c as
 * docs/provenance/precedent.md allows reading XNU: namei the path,
 * VOP_GETATTR, the MNT_NOEXEC and setuid checks, VOP_ACCESS, the VREG
 * test, then read the header; handle `#!' by re-naming the
 * interpreter and looping; copy the arguments into exec_map; and only
 * then commit, destroying the old address space and mapping the new.
 * That order matters and is kept: nothing is torn down until the
 * image is known to be loadable, which is also why
 * kern/exec_elf.c:elf_load_psection can act immediately where
 * NetBSD's defers through a vmcmd list.
 *
 * Where 4.4BSD parses a.out -- ZMAGIC, NMAGIC, OMAGIC and the
 * a_text/a_data/a_bss arithmetic -- this reads an ELF header instead,
 * because this tree's binaries are ELF and GNU ld has no i386 a.out
 * emulation. The three ELF functions are NetBSD 1.1's, imported and
 * adapted in kern/exec_elf.c.
 *
 * What is this project's own: the program-header loop, which walks
 * e_phnum entries and calls elf_load_psection for each PT_LOAD. That
 * is the part no donor could supply, because every post-settlement
 * BSD wraps it in an exec framework this tree does not have.
 */
/* ARGSUSED */
execve(p, uap, retval)
	register struct proc *p;
	register struct execve_args *uap;
	int *retval;
{
	register struct nameidata *ndp;
	struct nameidata nd;
	struct ucred *cred = p->p_ucred;
	struct vnode *vp;
	struct vattr vattr;
	struct vmspace *vm;
	Elf32_Ehdr eh;
	Elf32_Phdr *ph = NULL;
	char cfarg[MAXINTERP];
	char *cp, *sharg;
	vm_offset_t execargs = 0;
	u_long entry, addr, size;
	int indir, uid, gid, prot;
	int na, ne, nc, cc, len, resid, ssize, phsize;
	int error, i;
	int ap, ucp;

	indir = 0;
	uid = cred->cr_uid;
	gid = cred->cr_gid;
	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF | SAVENAME, UIO_USERSPACE,
	    uap->fname, p);
	ndp = &nd;
again:
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (error = VOP_GETATTR(vp, &vattr, cred, p))
		goto bad;
	if (vp->v_mount->mnt_flag & MNT_NOEXEC) {
		error = EACCES;
		goto bad;
	}
	if ((vp->v_mount->mnt_flag & MNT_NOSUID) == 0) {
		if (vattr.va_mode & VSUID)
			uid = vattr.va_uid;
		if (vattr.va_mode & VSGID)
			gid = vattr.va_gid;
	}
	if (error = VOP_ACCESS(vp, VEXEC, cred, p))
		goto bad;
	if ((p->p_flag & P_TRACED) && (error = VOP_ACCESS(vp, VREAD, cred, p)))
		goto bad;
	if (vp->v_type != VREG ||
	    (vattr.va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0) {
		error = EACCES;
		goto bad;
	}

	/*
	 * Read the header. An ELF file begins with Elf32_e_ident; a
	 * script with `#!'. 4.4BSD reads a union of struct exec and a
	 * MAXINTERP buffer for the same purpose.
	 */
	bzero((caddr_t)&eh, sizeof (eh));
	if (error = vn_rdwr(UIO_READ, vp, (caddr_t)&eh, sizeof (eh),
	    (off_t)0, UIO_SYSSPACE, (IO_UNIT|IO_NODELOCKED), cred, &resid,
	    (struct proc *)0))
		goto bad;

	if (elf_check_header(&eh, Elf32_et_exec) != 0) {
		/*
		 * Not ELF. A script, then, unless we are already one
		 * level deep -- 4.4BSD refuses to follow a `#!' line
		 * that names another script, and so does this.
		 */
		cp = (char *)&eh;
		if (cp[0] != '#' || cp[1] != '!' || indir) {
			error = ENOEXEC;
			goto bad;
		}
		for (cp = (char *)&eh + 2;; ++cp) {
			if (cp >= (char *)&eh + sizeof (eh)) {
				error = ENOEXEC;
				goto bad;
			}
			if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			if (*cp == '\t')
				*cp = ' ';
		}
		cp = (char *)&eh + 2;
		while (*cp == ' ')
			cp++;
		ndp->ni_dirp = cp;
		while (*cp && *cp != ' ')
			cp++;
		cfarg[0] = '\0';
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp)
				bcopy((caddr_t)cp, (caddr_t)cfarg, MAXINTERP);
		}
		indir = 1;
		vput(vp);
		ndp->ni_segflg = UIO_SYSSPACE;
		uid = cred->cr_uid;	/* shell scripts can't be setuid */
		gid = cred->cr_gid;
		goto again;
	}

	/*
	 * Read the program headers, before anything is destroyed.
	 */
	if (eh.e_phnum == 0 || eh.e_phentsize != sizeof (Elf32_Phdr)) {
		error = ENOEXEC;
		goto bad;
	}
	phsize = eh.e_phnum * sizeof (Elf32_Phdr);
	ph = (Elf32_Phdr *)malloc(phsize, M_TEMP, M_WAITOK);
	if (error = elf_read_from(p, vp, eh.e_phoff, (caddr_t)ph, phsize))
		goto bad;

	/*
	 * Copy the arguments into exec_map, which is where 4.4BSD puts
	 * them: out of the old address space before it is destroyed and
	 * out of the new one before it exists.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	cc = NCARGS;
	execargs = kmem_alloc_wait(exec_map, NCARGS);
	if (execargs == (vm_offset_t)0)
		panic("execve: kmem_alloc_wait");
	cp = (char *)execargs;
	for (;;) {
		ap = NULL;
		sharg = NULL;
		if (indir && na == 0) {
			sharg = ndp->ni_cnd.cn_nameptr;
			ap = (int)sharg;
			uap->argp++;			/* ignore argv[0] */
		} else if (indir && (na == 1 && cfarg[0])) {
			sharg = cfarg;
			ap = (int)sharg;
		} else if (indir && (na == 1 || na == 2 && cfarg[0]))
			ap = (int)uap->fname;
		else if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap == NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) != NULL)
				uap->envp++, ne++;
		}
		if (ap == NULL)
			break;
		na++;
		if (ap == -1) {
			error = EFAULT;
			goto bad;
		}
		do {
			if (nc >= NCARGS-1) {
				error = E2BIG;
				goto bad;
			}
			if (sharg) {
				len = strlen(sharg) + 1;
				bcopy(sharg, cp, (unsigned)len);
				sharg += len;
				error = 0;
			} else
				error = copyinstr((caddr_t)ap, cp,
				    (unsigned)cc, (u_int *)&len);
			ap += len;
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENAMETOOLONG);
		if (error) {
			if (error == EFAULT)
				error = EFAULT;
			goto bad;
		}
	}
	if (na == 0) {
		error = EFAULT;
		goto bad;
	}
	nc = (nc + NBPW-1) & ~(NBPW - 1);

	/*
	 * Everything is validated and copied. Commit: the old address
	 * space goes now, and nothing below may fail recoverably.
	 */
	vm = p->p_vmspace;
	if (vm->vm_refcnt > 1) {
		p->p_vmspace = vmspace_alloc(VM_MIN_ADDRESS, VM_MAXUSER_ADDRESS,
		    1);
		vmspace_free(vm);
		vm = p->p_vmspace;
	} else {
#ifdef SYSVSHM
		if (vm->vm_shm)
			shmexit(p);
#endif
		(void) vm_map_remove(&vm->vm_map, VM_MIN_ADDRESS,
		    VM_MAXUSER_ADDRESS);
	}
	if (p->p_flag & P_PPWAIT) {
		p->p_flag &= ~P_PPWAIT;
		wakeup((caddr_t)p->p_pptr);
	}

	/*
	 * AI-ONLY NOTE: the program-header loop is this project's own.
	 * Every post-settlement BSD wraps it in an exec framework --
	 * NetBSD's exec_package, FreeBSD's imgact -- so none could
	 * supply it in this shape. It is the minimum the ELF
	 * specification asks: for each PT_LOAD segment, map p_filesz
	 * bytes from p_offset at p_vaddr and zero to p_memsz, which is
	 * what elf_load_psection does.
	 */
	vm->vm_taddr = (caddr_t)eh.e_entry;
	vm->vm_tsize = 0;
	vm->vm_daddr = 0;
	vm->vm_dsize = 0;
	for (i = 0; i < eh.e_phnum; i++) {
		if (ph[i].p_type != Elf32_pt_load)
			continue;
		addr = ELF32_NO_ADDR;
		size = 0;
		prot = 0;
		if (error = elf_load_psection(p, vp, &ph[i], &addr, &size,
		    &prot))
			goto exec_abort;
		if (prot & VM_PROT_EXECUTE) {
			vm->vm_taddr = (caddr_t)addr;
			vm->vm_tsize = btoc(size);
		} else {
			vm->vm_daddr = (caddr_t)addr;
			vm->vm_dsize = btoc(size);
		}
	}
	entry = eh.e_entry;

	/*
	 * The stack, as 4.4BSD's getxfile allocates it.
	 *
	 * AI-ONLY NOTE: the `- NBPG' is Berkeley's and is not slack.
	 * His getxfile has
	 *
	 *	size = round_page(MAXSSIZ);
	 *	#ifdef	i386
	 *	addr = trunc_page(USRSTACK - size) - NBPG;	/ * XXX * /
	 *	#else
	 *	addr = trunc_page(USRSTACK - size);
	 *	#endif
	 *
	 * because on this port USRSTACK is 0xEFBFE000 and
	 * VM_MAXUSER_ADDRESS, which is where the vmspace's map ends, is
	 * 0xEFBFD000 -- one page lower. A region running up to USRSTACK
	 * ends one page past the map and vm_allocate refuses it. An
	 * earlier version of this body dropped the i386 arm, and every
	 * exec failed there: all four of init's PT_LOAD segments
	 * mapped, then the stack allocation failed, and exec_abort
	 * killed the process with SIGABRT -- which is the `init died
	 * (signal 6, exit 0)' this kernel was printing.
	 *
	 * The vm_map_protect below is his too. Everything above the
	 * current stack limit is VM_PROT_NONE, so the stack grows by
	 * faulting into it rather than being wired at MAXSSIZ.
	 */
	size = round_page(MAXSSIZ);
	addr = (u_long)trunc_page(USRSTACK - size) - NBPG;
	if (error = vm_allocate(&vm->vm_map, &addr, size, FALSE))
		goto exec_abort;
	size -= round_page(p->p_rlimit[RLIMIT_STACK].rlim_cur);
	if (error = vm_map_protect(&vm->vm_map, addr, addr + size,
	    VM_PROT_NONE, FALSE))
		goto exec_abort;
	vm->vm_maxsaddr = (caddr_t)addr;
	vm->vm_ssize = 0;

	free((caddr_t)ph, M_TEMP);
	ph = NULL;
	vput(vp);
	vp = NULL;

	/*
	 * Copy the arguments onto the new stack.
	 */
	ssize = (na + 3) * NBPW;
	ucp = USRSTACK - nc;
	ap = ucp - ssize;
	ssize += nc;
	cpu_setstack(p, ap);
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	cp = (char *)execargs;
	cc = NCARGS;
	for (;;) {
		ap += NBPW;
		if (na == ne) {
			(void) suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);
		do {
			error = copyoutstr(cp, (caddr_t)ucp, (unsigned)cc,
			    (u_int *)&len);
			ucp += len;
			cp += len;
			nc += len;
			cc -= len;
		} while (error == ENAMETOOLONG);
		if (error == EFAULT)
			panic("exec: EFAULT");
	}
	(void) suword((caddr_t)ap, 0);

	execsigs(p);
	fdcloseexec(p);
	if (p->p_flag & P_TRACED)
		psignal(p, SIGTRAP);
	p->p_acflag &= ~AFORK;
	if (uid != cred->cr_uid || gid != cred->cr_gid) {
		p->p_ucred = cred = crcopy(cred);
		cred->cr_uid = uid;
		cred->cr_gid = gid;
		p->p_flag |= P_SUGID;
	}
	bcopy(ndp->ni_cnd.cn_nameptr, p->p_comm, MAXCOMLEN);
	p->p_comm[MAXCOMLEN] = '\0';
	setregs(p, entry, retval);
	kmem_free_wakeup(exec_map, execargs, NCARGS);
	FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
	return (0);

exec_abort:
	/*
	 * The address space is already gone; the process cannot
	 * continue. 4.4BSD has no recovery here either.
	 */
	if (ph)
		free((caddr_t)ph, M_TEMP);
	if (execargs)
		kmem_free_wakeup(exec_map, execargs, NCARGS);
	if (vp)
		vput(vp);
	FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
	exit1(p, W_EXITCODE(0, SIGABRT));
	return (0);

bad:
	if (ph)
		free((caddr_t)ph, M_TEMP);
	if (execargs)
		kmem_free_wakeup(exec_map, execargs, NCARGS);
	if (vp)
		vput(vp);
	FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
	return (error);
}
