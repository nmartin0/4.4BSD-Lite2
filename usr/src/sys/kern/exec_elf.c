/*	$NetBSD: exec_elf.c,v 1.3 1995/09/16 00:28:08 thorpej Exp $	*/

/*
 * Copyright (c) 1994 Christos Zoulas
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
 * 3. The name of the author may not be used to endorse or promote products
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
 * AI-ONLY NOTE: the two functions below are taken verbatim from NetBSD
 * 1.1's kern/exec_elf.c, revision 1.3 of 16 September 1995 -- the
 * earliest NetBSD release with a working i386 ELF loader. This tree's
 * binaries are ELF and GNU ld has no i386 a.out emulation, which
 * docs/provenance/csu-elf.md records for lib/csu; kern/kern_exec.c's
 * execve is one of the thirty-five bodies the AT&T settlement removed,
 * so there is nothing of Berkeley's here to correct.
 *
 * Only these two of that file's five functions are taken, because only
 * these two are free of NetBSD's exec framework, which this tree does
 * not have and is not adopting:
 *
 *	elf_check_header   27 lines, 0 references to exec_package
 *	elf_read_from      20 lines, 0 references to exec_package
 *	elf_load_psection  48 lines, 4 -- adapted separately
 *	elf_set_segment    33 lines, 7 -- not needed, it records text,
 *	                   data and bss into an exec_package where
 *	                   Berkeley's execve keeps them in locals
 *	elf_copyargs       84 lines, 1 -- not needed, execve copies its
 *	                   own arguments as 4.4BSD and 386BSD both do
 *
 * Both use only what 4.4BSD-Lite2 already has: bcmp, and vn_rdwr at
 * kern/vfs_vnops.c. Nothing was changed, not even the XXX comment
 * above the machine switch or the indentation of elf_read_from's
 * second comment, which is NetBSD's.
 *
 * Three-clause BSD, Christos Zoulas 1994. No Regents notice, so
 * nothing here is Berkeley's, and docs/provenance/imports.md records
 * it as a transplant.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/exec_elf.h>

#include <vm/vm.h>

#define ELF_ALIGN(a, b) ((a) & ~((b) - 1))

int
elf_check_header(eh, type)
	Elf32_Ehdr *eh;
	int type;
{

	if (bcmp(eh->e_ident, Elf32_e_ident, Elf32_e_siz) != 0)
		return ENOEXEC;

	switch (eh->e_machine) {
	/* XXX */
#ifdef i386
	case Elf32_em_386:
	case Elf32_em_486:
#endif
#ifdef sparc
	case Elf32_em_sparc:
#endif
		break;

	default:
		return ENOEXEC;
	}

	if (eh->e_type != type)
		return ENOEXEC;

	return 0;
}

int
elf_read_from(p, vp, off, buf, size)
	struct vnode *vp;
	u_long off;
	struct proc *p;
	caddr_t buf;
	int size;
{
	int error;
	int resid;

	if ((error = vn_rdwr(UIO_READ, vp, buf, size,
			     off, UIO_SYSSPACE, IO_NODELOCKED, p->p_ucred,
			     &resid, p)) != 0)
		return error;
	/*
         * See if we got all of it
         */
	if (resid != 0)
		return error;
	return 0;
}

/*
 * AI-ONLY NOTE: elf_load_psection, adapted rather than taken. NetBSD
 * 1.1's is at their exec_elf.c:212 and is 48 lines, of which four
 * touch their exec framework: the vcset parameter and its
 * declaration, and two NEW_VMCMD calls. The other 44 -- the alignment
 * arithmetic, the protection bits and the memsz/filesz difference --
 * are taken unchanged.
 *
 * The two NEW_VMCMD calls become the work those commands do. From
 * their exec_subr.c:155 and :183:
 *
 *	vmcmd_map_readvn   vm_allocate, then vn_rdwr, then vm_map_protect
 *	vmcmd_map_zero     vm_allocate, then vm_map_protect
 *
 * so the calls below are those sequences written out, with this
 * tree's own vm_allocate at vm/vm_user.c, vn_rdwr at
 * kern/vfs_vnops.c and vm_map_protect at vm/vm_map.c.
 *
 * What the framework buys, and why not having it is sound here. A
 * vmcmd set is a deferred-action list: an activator plans the new
 * address space, and the list runs only once the kernel has committed
 * to the exec, so a header that fails to parse cannot have already
 * destroyed the process. Acting immediately is correct only if
 * validation has finished first -- which is exactly how 4.4BSD's
 * execve is built, reading and checking the whole header before it
 * calls vm_deallocate on the old address space. The caller must
 * preserve that order, and the one written for this tree does.
 *
 * The signature therefore loses vcset and gains the proc, because the
 * vm calls need p->p_vmspace where the commands carried it.
 */
int
elf_load_psection(p, vp, ph, addr, size, prot)
	struct proc *p;
	struct vnode *vp;
	Elf32_Phdr *ph;
	u_long *addr;
	u_long *size;
	int *prot;
{
	u_long uaddr, msize, rm, rf;
	long diff, offset;
	int error;

	/*
	 * If the user specified an address, then we load there.
	 */
	if (*addr != ELF32_NO_ADDR) {
		if (ph->p_align > 1) {
			*addr = ELF_ALIGN(*addr + ph->p_align, ph->p_align);
			uaddr = ELF_ALIGN(ph->p_vaddr, ph->p_align);
		} else
			uaddr = ph->p_vaddr;
		diff = ph->p_vaddr - uaddr;
	} else {
		*addr = uaddr = ph->p_vaddr;
		if (ph->p_align > 1)
			*addr = ELF_ALIGN(uaddr, ph->p_align);
		diff = uaddr - *addr;
	}

	*prot |= (ph->p_flags & Elf32_pf_r) ? VM_PROT_READ : 0;
	*prot |= (ph->p_flags & Elf32_pf_w) ? VM_PROT_WRITE : 0;
	*prot |= (ph->p_flags & Elf32_pf_x) ? VM_PROT_EXECUTE : 0;

	offset = ph->p_offset - diff;
	*size = ph->p_filesz + diff;
	msize = ph->p_memsz + diff;

	/* NEW_VMCMD(vcset, vmcmd_map_readvn, *size, *addr, vp, offset, *prot) */
	if (error = vm_allocate(&p->p_vmspace->vm_map, addr, *size, 0))
		return (error);
	if (error = vn_rdwr(UIO_READ, vp, (caddr_t)*addr, *size, offset,
	    UIO_USERSPACE, IO_UNIT|IO_NODELOCKED, p->p_ucred, (int *)0, p))
		return (error);
	if (error = vm_map_protect(&p->p_vmspace->vm_map, trunc_page(*addr),
	    round_page(*addr + *size), *prot, FALSE))
		return (error);

	/*
	 * Check if we need to extend the size of the segment
	 */
	rm = round_page(*addr + msize);
	rf = round_page(*addr + *size);

	if (rm != rf) {
		/* NEW_VMCMD(vcset, vmcmd_map_zero, rm - rf, rf, NULLVP, 0, *prot) */
		if (error = vm_allocate(&p->p_vmspace->vm_map, &rf, rm - rf, 0))
			return (error);
		if (error = vm_map_protect(&p->p_vmspace->vm_map,
		    trunc_page(rf), round_page(rm), *prot, FALSE))
			return (error);
		*size = msize;
	}
	return (0);
}
