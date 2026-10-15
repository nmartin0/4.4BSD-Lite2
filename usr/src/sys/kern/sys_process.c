/*
 * Copyright (c) 1994, Sean Eric Fagan
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
 *	This product includes software developed by Sean Eric Fagan.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: sys_process.c,v 1.24 1996/06/02 06:24:27 dyson Exp $
 */

/*
 * AI-ONLY NOTE: FreeBSD 2.2's kern/sys_process.c (rev 1.24, above), in
 * place of Lite2's, whose ptrace() and trace_req() bodies were deleted
 * for the USL settlement and which answered ENOSYS. FreeBSD 2.2's was
 * chosen for keeping Lite2's own interface whole -- Berkeley's request
 * numbers and PT_READ_U and PT_WRITE_U, with the u-area write confined
 * to the registers by ptrace_write_u -- and for reaching memory and
 * registers through the procfs functions Lite2 already has. Changed
 * from FreeBSD's:
 *
 *  - ptrace() is declared as Berkeley's own last version (CSRG, 8.6,
 *    February 1995) declares it, with <sys/syscallargs.h>'s argument
 *    structure, SCARG() and register_t, in place of FreeBSD's
 *    <sys/sysproto.h>;
 *  - PT_STEP goes through Lite2's procfs_sstep, not a second hook,
 *    ptrace_single_step; the one new hook is ptrace_set_pc, which with
 *    ptrace_write_u is in i386/i386/machdep.c, as in FreeBSD;
 *  - PHOLD and PRELE, FreeBSD's guard against swapping out, which
 *    Lite2 has not got, are dropped, as NetBSD 1.0 does without them;
 *    PT_READ_U and PT_WRITE_U still require P_INMEM;
 *  - the user area's size is ctob(UPAGES), PAGE_SIZE being a variable
 *    here;
 *  - FreeBSD's pread and pwrite, which it had left under `#if 0', and
 *    its VM headers, which only they used, are left out.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/ptrace.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/user.h>

#include <sys/mount.h>
#include <sys/syscallargs.h>

#include <machine/reg.h>
#include <machine/psl.h>
#include <miscfs/procfs/procfs.h>

/*
 * Process debugging system call.
 */
int
ptrace(curp, uap, retval)
	struct proc *curp;
	struct ptrace_args /* {
		syscallarg(int) req;
		syscallarg(pid_t) pid;
		syscallarg(caddr_t) addr;
		syscallarg(int) data;
	} */ *uap;
	register_t *retval;
{
	struct proc *p;
	struct iovec iov;
	struct uio uio;
	int error = 0;
	int write;
	int s;

	if (SCARG(uap, req) == PT_TRACE_ME)
		p = curp;
	else {
		if ((p = pfind(SCARG(uap, pid))) == NULL)
			return ESRCH;
	}

	/*
	 * Permissions check
	 */
	switch (SCARG(uap, req)) {
	case PT_TRACE_ME:
		/* Always legal. */
		break;

	case PT_ATTACH:
		/* Self */
		if (p->p_pid == curp->p_pid)
			return EINVAL;

		/* Already traced */
		if (p->p_flag & P_TRACED)
			return EBUSY;

		/* not owned by you, has done setuid (unless you're root) */
		if ((p->p_cred->p_ruid != curp->p_cred->p_ruid) ||
		     (p->p_flag & P_SUGID)) {
			if (error = suser(curp->p_ucred, &curp->p_acflag))
				return error;
		}

		/* can't trace init when securelevel > 0 */
		if (securelevel > 0 && p->p_pid == 1)
			return EPERM;

		/* OK */
		break;

	case PT_READ_I:
	case PT_READ_D:
	case PT_READ_U:
	case PT_WRITE_I:
	case PT_WRITE_D:
	case PT_WRITE_U:
	case PT_CONTINUE:
	case PT_KILL:
	case PT_STEP:
	case PT_DETACH:
#ifdef PT_GETREGS
	case PT_GETREGS:
#endif
#ifdef PT_SETREGS
	case PT_SETREGS:
#endif
#ifdef PT_GETFPREGS
	case PT_GETFPREGS:
#endif
#ifdef PT_SETFPREGS
	case PT_SETFPREGS:
#endif
		/* not being traced... */
		if ((p->p_flag & P_TRACED) == 0)
			return EPERM;

		/* not being traced by YOU */
		if (p->p_pptr != curp)
			return EBUSY;

		/* not currently stopped */
		if (p->p_stat != SSTOP || (p->p_flag & P_WAITED) == 0)
			return EBUSY;

		/* OK */
		break;

	default:
		return EINVAL;
	}

#ifdef FIX_SSTEP
	/*
	 * Single step fixup ala procfs
	 */
	FIX_SSTEP(p);
#endif

	/*
	 * Actually do the requests
	 */

	write = 0;
	*retval = 0;

	switch (SCARG(uap, req)) {
	case PT_TRACE_ME:
		/* set my trace flag and "owner" so it can read/write me */
		p->p_flag |= P_TRACED;
		p->p_oppid = p->p_pptr->p_pid;
		return 0;

	case PT_ATTACH:
		/* security check done above */
		p->p_flag |= P_TRACED;
		p->p_oppid = p->p_pptr->p_pid;
		if (p->p_pptr != curp)
			proc_reparent(p, curp);
		SCARG(uap, data) = SIGSTOP;
		goto sendsig;	/* in PT_CONTINUE below */

	case PT_STEP:
	case PT_CONTINUE:
	case PT_DETACH:
		if ((unsigned)SCARG(uap, data) >= NSIG)
			return EINVAL;

		if (SCARG(uap, req) == PT_STEP) {
			if ((error = procfs_sstep(p, 1)))
				return error;
		}

		if (SCARG(uap, addr) != (caddr_t)1) {
			fill_eproc (p, &p->p_addr->u_kproc.kp_eproc);
			if ((error = ptrace_set_pc (p, (u_int)SCARG(uap, addr))))
				return error;
		}

		if (SCARG(uap, req) == PT_DETACH) {
			/* reset process parent */
			if (p->p_oppid != p->p_pptr->p_pid) {
				struct proc *pp;

				pp = pfind(p->p_oppid);
				proc_reparent(p, pp ? pp : initproc);
			}

			p->p_flag &= ~(P_TRACED | P_WAITED);
			p->p_oppid = 0;

			/* should we send SIGCHLD? */

		}

	sendsig:
		/* deliver or queue signal */
		s = splhigh();
		if (p->p_stat == SSTOP) {
			p->p_xstat = SCARG(uap, data);
			setrunnable(p);
		} else if (SCARG(uap, data)) {
			psignal(p, SCARG(uap, data));
		}
		splx(s);
		return 0;

	case PT_WRITE_I:
	case PT_WRITE_D:
		write = 1;
		/* fallthrough */
	case PT_READ_I:
	case PT_READ_D:
		/* write = 0 set above */
		iov.iov_base = write ? (caddr_t)&SCARG(uap, data) : (caddr_t)retval;
		iov.iov_len = sizeof(int);
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = (off_t)(u_long)SCARG(uap, addr);
		uio.uio_resid = sizeof(int);
		uio.uio_segflg = UIO_SYSSPACE;	/* ie: the uap */
		uio.uio_rw = write ? UIO_WRITE : UIO_READ;
		uio.uio_procp = p;
		error = procfs_domem(curp, p, NULL, &uio);
		if (uio.uio_resid != 0) {
			/*
			 * XXX procfs_domem() doesn't currently return ENOSPC,
			 * so I think write() can bogusly return 0.
			 * XXX what happens for short writes?  We don't want
			 * to write partial data.
			 * XXX procfs_domem() returns EPERM for other invalid
			 * addresses.  Convert this to EINVAL.  Does this
			 * clobber returns of EPERM for other reasons?
			 */
			if (error == 0 || error == ENOSPC || error == EPERM)
				error = EINVAL;	/* EOF */
		}
		return (error);

	case PT_READ_U:
		if ((u_int)SCARG(uap, addr) > (ctob(UPAGES) - sizeof(int))) {
			return EFAULT;
		}
		error = 0;
		if (p->p_flag & P_INMEM) {
			p->p_addr->u_kproc.kp_proc = *p;
			fill_eproc (p, &p->p_addr->u_kproc.kp_eproc);
			*retval = *(int*)((u_int)p->p_addr + (u_int)SCARG(uap, addr));
		} else {
			*retval = 0;
			error = EFAULT;
		}
		return error;

	case PT_WRITE_U:
		if (p->p_flag & P_INMEM) {
			p->p_addr->u_kproc.kp_proc = *p;
			fill_eproc (p, &p->p_addr->u_kproc.kp_eproc);
			error = ptrace_write_u(p, (vm_offset_t)SCARG(uap, addr), SCARG(uap, data));
		} else {
			error = EFAULT;
		}
		return error;

	case PT_KILL:
		SCARG(uap, data) = SIGKILL;
		goto sendsig;	/* in PT_CONTINUE above */

#ifdef PT_SETREGS
	case PT_SETREGS:
		write = 1;
		/* fallthrough */
#endif /* PT_SETREGS */
#ifdef PT_GETREGS
	case PT_GETREGS:
		/* write = 0 above */
#endif /* PT_SETREGS */
#if defined(PT_SETREGS) || defined(PT_GETREGS)
		if (!procfs_validregs(p))	/* no P_SYSTEM procs please */
			return EINVAL;
		else {
			iov.iov_base = SCARG(uap, addr);
			iov.iov_len = sizeof(struct reg);
			uio.uio_iov = &iov;
			uio.uio_iovcnt = 1;
			uio.uio_offset = 0;
			uio.uio_resid = sizeof(struct reg);
			uio.uio_segflg = UIO_USERSPACE;
			uio.uio_rw = write ? UIO_WRITE : UIO_READ;
			uio.uio_procp = curp;
			return (procfs_doregs(curp, p, NULL, &uio));
		}
#endif /* defined(PT_SETREGS) || defined(PT_GETREGS) */

#ifdef PT_SETFPREGS
	case PT_SETFPREGS:
		write = 1;
		/* fallthrough */
#endif /* PT_SETFPREGS */
#ifdef PT_GETFPREGS
	case PT_GETFPREGS:
		/* write = 0 above */
#endif /* PT_SETFPREGS */
#if defined(PT_SETFPREGS) || defined(PT_GETFPREGS)
		if (!procfs_validfpregs(p))	/* no P_SYSTEM procs please */
			return EINVAL;
		else {
			iov.iov_base = SCARG(uap, addr);
			iov.iov_len = sizeof(struct fpreg);
			uio.uio_iov = &iov;
			uio.uio_iovcnt = 1;
			uio.uio_offset = 0;
			uio.uio_resid = sizeof(struct fpreg);
			uio.uio_segflg = UIO_USERSPACE;
			uio.uio_rw = write ? UIO_WRITE : UIO_READ;
			uio.uio_procp = curp;
			return (procfs_dofpregs(curp, p, NULL, &uio));
		}
#endif /* defined(PT_SETFPREGS) || defined(PT_GETFPREGS) */

	default:
		break;
	}

	return 0;
}

int
trace_req(p)
	struct proc *p;
{
	return 1;
}
