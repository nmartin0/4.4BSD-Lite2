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
 *
 */

/*
 * AI-ONLY NOTE (2026-10-31; docs/provenance/exec-stack.md): NetBSD 1.1's
 * kern/exec_elf.c, rev 1.3 of 16 September 1995, the first NetBSD ELF
 * loader, now that this tree has the exec framework it plugs into
 * (sys/exec.h, kern/exec_subr.c, kern/exec_conf.c, kern/kern_exec.c).
 * Until today this file held three of its functions, adapted to work
 * without that framework, and kern_exec.c did the rest itself.
 *
 * Taken out, all for programs this tree does not have -- every ELF
 * program it builds is static, with one text and one data segment and
 * no PT_INTERP (all 515 ELF executables in the built root, 473 files,
 * counted 2026-10-31):
 *
 *	elf_probe_funcs	the SVR4 and Linux emulations' probes
 *	elf_copyargs	the auxiliary vector for a dynamic linker
 *	elf_load_file	loads that dynamic linker
 *
 * and in exec_elf_makecmds the interpreter's path, its loading and its
 * argument block go with them: a program that asks for an interpreter
 * is refused with ENOEXEC rather than started without one. Also not
 * taken, for its own reason given where it was: the line that marks
 * the vnode VTEXT.
 *
 * Kept from this tree's own earlier fixes to the same functions, each
 * with its note below: elf_check_header's machine types from
 * <machine/elf_machdep.h> (2026-10-27); ELF_ROUND and ELF_TRUNC for
 * ELF_ALIGN (NetBSD exec_elf32.c rev 1.18); elf_read_from's short
 * read returning ENOEXEC (patch 246, NetBSD 1.2). Added: the bound on
 * e_phnum, from NetBSD exec_elf32.c rev 1.61 -- see elf_check_header.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/exec.h>
#include <sys/exec_elf.h>

#include <sys/mman.h>
#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_map.h>

#include <machine/cpu.h>
#include <machine/reg.h>
/*
 * AI-ONLY NOTE: NetBSD includes <machine/exec.h> here as well. Lite2's
 * <sys/exec.h>, above, already includes it, and it has no guard.
 */

static int elf_set_segment __P((struct exec_package *, u_long, u_long,
	int));
static int elf_read_from __P((struct proc *, struct vnode *, u_long,
	caddr_t, int));
static void elf_load_psection __P((struct exec_vmcmd_set *,
	struct vnode *, Elf32_Phdr *, u_long *, u_long *, int *));

/*
 * AI-ONLY NOTE: changed (2026-10-31) -- split into ELF_ROUND and
 * ELF_TRUNC, as NetBSD's own sys/kern/exec_elf32.c, its own
 * elf_load_psection ($NetBSD: exec_elf32.c,v 1.18 1996/10/11 19:44:02
 * cgd Exp $, "replace ELF_ALIGN with ELF_TRUNC ... and ELF_ROUND
 * ... This fixes a bug found by Robert Baron ... where
 * elf_load_psection, if given a properly aligned address at which to
 * load the section, would ... actually load it at the next highest
 * alignment boundary" -- exactly this bug); NetBSD 1.1's own
 * exec_elf.c, $NetBSD: exec_elf.c,v 1.3 1995/09/16 00:28:08 thorpej
 * Exp $, still has the single ELF_ALIGN this tree's own
 * elf_load_psection was itself adapted from -- see
 * docs/provenance/smaller-fixes.md. (An earlier citation of this
 * note, rev 1.26.8.1/mycroft 1998-05-05, was a real but much later
 * pullup of the same fix, not the fix's origin; corrected after
 * independent review to rev 1.18, the first NetBSD revision to
 * carry it, found in /home/claude/donors/nb12h.)
 * The one `ELF_ALIGN' below always truncated down to the alignment
 * boundary; elf_load_psection() used it below both where that's
 * right (truncating `ph->p_vaddr' down to find the segment's own
 * aligned base) and where it's wrong (rounding `*addr', a
 * caller-specified load address, UP to the next boundary) --
 * `ELF_ALIGN(*addr + ph->p_align, ph->p_align)' adds a full
 * `p_align' and then truncates, which always lands one full
 * alignment unit past `*addr' even when `*addr' was already
 * aligned, rather than leaving an aligned address unchanged.
 * `ELF_ROUND' (round up, leaving an aligned value as-is) replaces
 * ELF_ALIGN only at the two sites that want rounding up; ELF_TRUNC
 * (identical to the old ELF_ALIGN) replaces it at the one that wants
 * truncating down. This path is presently unreachable in this tree:
 * kern_exec.c's only caller always passes `addr = ELF32_NO_ADDR'
 * (since patch 262 the caller is exec_elf_makecmds, below, and it
 * passes the same),
 * so the `*addr != ELF32_NO_ADDR' branch below, where the bug
 * lives, never runs -- fixed anyway, since latent-but-wrong code is
 * still wrong code, and no donor fix was skipped over for being
 * latent.
 */
#define ELF_ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))
#define ELF_TRUNC(a, b)		((a) & ~((b) - 1))

/*
 * elf_check_header():
 *
 * Check header for validity; return 0 of ok ENOEXEC if error
 *
 * XXX machine type needs to be moved to <machine/param.h> so
 * just one comparison can be done. Unfortunately, there is both
 * em_486 and em_386, so this would not work on the i386.
 */
int
elf_check_header(eh, type)
	Elf32_Ehdr *eh;
	int type;
{

	if (bcmp(eh->e_ident, Elf32_e_ident, Elf32_e_siz) != 0)
		return ENOEXEC;

	/*
	 * AI-ONLY NOTE: changed (2026-10-27) -- the machine types come from
	 * <machine/elf_machdep.h> as NetBSD 1.3's exec_elf32.c has them,
	 * which spells it ELFDEFNNAME(MACHDEP_ID_CASES); this tree has no
	 * ELF64 and no such pasting macro, so the Elf32 name is written out.
	 * Replaces the #ifdef i386 and #ifdef sparc cases (and the XXX).
	 */
	switch (eh->e_machine) {

	ELF32_MACHDEP_ID_CASES

	default:
		return ENOEXEC;
	}

	if (eh->e_type != type)
		return ENOEXEC;

	/*
	 * AI-ONLY NOTE (2026-10-31): the bound on e_phnum is NetBSD's
	 * exec_elf32.c rev 1.61 (Charles M. Hannum, 15 December 2000,
	 * "Tighten up the ELF signature checks"), the first NetBSD
	 * revision to have one. Without it exec_elf_makecmds mallocs
	 * e_phnum program headers, M_WAITOK, for any number a file
	 * claims. Rev 1.61 bounds e_shnum in the same line; that half is
	 * not taken -- exec reads no section headers, and NetBSD itself
	 * later raised that bound (to 1024, then 32768) where it refused
	 * real programs.
	 */
	if (eh->e_phnum > 128)
		return ENOEXEC;

	return 0;
}

/*
 * elf_load_psection():
 * 
 * Load a psection at the appropriate address
 */
static void
elf_load_psection(vcset, vp, ph, addr, size, prot)
	struct exec_vmcmd_set *vcset;
	struct vnode *vp;
	Elf32_Phdr *ph;
	u_long *addr;
	u_long *size;
	int *prot;
{
	u_long uaddr, msize, rm, rf;
	long diff, offset;

	/*
         * If the user specified an address, then we load there.
         */
	if (*addr != ELF32_NO_ADDR) {
		if (ph->p_align > 1) {
			*addr = ELF_ROUND(*addr, ph->p_align);
			uaddr = ELF_TRUNC(ph->p_vaddr, ph->p_align);
		} else
			uaddr = ph->p_vaddr;
		diff = ph->p_vaddr - uaddr;
	} else {
		*addr = uaddr = ph->p_vaddr;
		if (ph->p_align > 1)
			*addr = ELF_TRUNC(uaddr, ph->p_align);
		diff = uaddr - *addr;
	}

	*prot |= (ph->p_flags & Elf32_pf_r) ? VM_PROT_READ : 0;
	*prot |= (ph->p_flags & Elf32_pf_w) ? VM_PROT_WRITE : 0;
	*prot |= (ph->p_flags & Elf32_pf_x) ? VM_PROT_EXECUTE : 0;

	offset = ph->p_offset - diff;
	*size = ph->p_filesz + diff;
	msize = ph->p_memsz + diff;

	NEW_VMCMD(vcset, vmcmd_map_readvn, *size, *addr, vp, offset, *prot);

	/*
         * Check if we need to extend the size of the segment
         */
	rm = round_page(*addr + msize);
	rf = round_page(*addr + *size);

	if (rm != rf) {
		NEW_VMCMD(vcset, vmcmd_map_zero, rm - rf, rf, NULLVP, 0, *prot);
		*size = msize;
	}
}

/*
 * elf_set_segment():
 *
 * Decide if the segment is text or data, depending on the protection
 * and set it appropriately
 */
static int
elf_set_segment(epp, vaddr, size, prot)
	struct exec_package *epp;
	u_long vaddr;
	u_long size;
	int prot;
{
	/*
         * Kludge: Unfortunately the current implementation of
         * exec package assumes a single text and data segment.
         * In Elf we can have more, but here we limit ourselves
         * to two and hope :-(
         * We also assume that the text is r-x, and data is rwx or rw-.
         */
	switch (prot) {
	case (VM_PROT_READ | VM_PROT_EXECUTE):
		if (epp->ep_tsize != ELF32_NO_ADDR)
			return ENOEXEC;
		epp->ep_taddr = vaddr;
		epp->ep_tsize = size;
		break;

	case (VM_PROT_READ | VM_PROT_WRITE):
	case (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE):
		if (epp->ep_dsize != ELF32_NO_ADDR)
			return ENOEXEC;
		epp->ep_daddr = vaddr;
		epp->ep_dsize = size;
		break;

	default:
		return ENOEXEC;
	}
	return 0;
}

/*
 * elf_read_from():
 *
 *	Read from vnode into buffer at offset.
 */
static int
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
         * See if we got all of it.  AI-ONLY NOTE: this returned `error',
         * which is 0 here, so a short read -- program headers past the
         * end of the file -- counted as success and the headers were
         * whatever the buffer held.  NetBSD 1.2 returns ENOEXEC (patch
         * 246; provenance/elf-short-read.md).
         */
	if (resid != 0)
		return ENOEXEC;
	return 0;
}

/*
 * exec_elf_makecmds(): Prepare an Elf binary's exec package
 *
 * First, set of the various offsets/lengths in the exec package.
 *
 * Then, mark the text image busy (so it can be demand paged) or error
 * out if this is not possible.  Finally, set up vmcmds for the
 * text, data, bss, and stack segments.
 *
 * XXX no demand paging (yet?)
 */
int
exec_elf_makecmds(p, epp)
	struct proc *p;
	struct exec_package *epp;
{
	Elf32_Ehdr *eh = epp->ep_hdr;
	Elf32_Phdr *ph, *pp;
	int error, i;
	u_long phsize;

	if (epp->ep_hdrvalid < sizeof(Elf32_Ehdr))
		return ENOEXEC;

	if (elf_check_header(eh, Elf32_et_exec))
		return ENOEXEC;

	/*
         * check if vnode is in open for writing, because we want to
         * demand-page out of it.  if it is, don't do it, for various
         * reasons
         */
	if (epp->ep_vp->v_writecount != 0) {
#ifdef DIAGNOSTIC
		if (epp->ep_vp->v_flag & VTEXT)
			panic("exec: a VTEXT vnode has writecount != 0\n");
#endif
		return ETXTBSY;
	}
	/*
         * Allocate space to hold all the program headers, and read them
         * from the file
         */
	phsize = eh->e_phnum * sizeof(Elf32_Phdr);
	ph = (Elf32_Phdr *) malloc(phsize, M_TEMP, M_WAITOK);

	if ((error = elf_read_from(p, epp->ep_vp, eh->e_phoff,
				    (caddr_t) ph, phsize)) != 0)
		goto bad;

	epp->ep_tsize = ELF32_NO_ADDR;
	epp->ep_dsize = ELF32_NO_ADDR;

	/*
         * Load all the necessary sections
         */
	for (i = 0; i < eh->e_phnum; i++) {
		u_long  addr = ELF32_NO_ADDR, size = 0;
		int prot = 0;

		pp = &ph[i];

		switch (ph[i].p_type) {
		case Elf32_pt_load:
			elf_load_psection(&epp->ep_vmcmds, epp->ep_vp,
				&ph[i], &addr, &size, &prot);
			if ((error = elf_set_segment(epp, addr, size,
						      prot)) != 0)
				goto bad;
			break;

		case Elf32_pt_shlib:
			error = ENOEXEC;
			goto bad;

		case Elf32_pt_interp:
			/*
			 * AI-ONLY NOTE: refused. NetBSD 1.1 loads the
			 * interpreter named here (elf_load_file, not taken);
			 * this tree has no dynamic linker to load.
			 */
			error = ENOEXEC;
			goto bad;

		case Elf32_pt_dynamic:
		case Elf32_pt_phdr:
		case Elf32_pt_note:
			break;

		default:
			/*
			 * Not fatal, we don't need to understand everything
			 * :-)
			 */
			break;
		}
	}

	epp->ep_entry = eh->e_entry;

	free((char *) ph, M_TEMP);
	/*
	 * AI-ONLY NOTE (2026-10-31): NetBSD 1.1 sets VTEXT on the vnode
	 * here, `epp->ep_vp->v_flag |= VTEXT;'. Not taken. VTEXT marks a
	 * vnode the vnode pager is paging text from, and only
	 * vnode_pager_dealloc (vm/vnode_pager.c) clears it; this loader
	 * never maps the file -- vmcmd_map_readvn reads the segments in,
	 * as NetBSD's own comment above says ("no demand paging") -- so no
	 * pager would ever exist to clear it, and the flag would stay on
	 * every program ever run. Stuck, it is not harmless here:
	 * nfs/nfs_bio.c's nfs_doio kills any process reading a VTEXT file
	 * over NFS whose modification time has changed ("Process killed due
	 * to text file modification"), and the DIAGNOSTIC panic above
	 * would fire for an exec of a program that has run before and is
	 * open for writing at the time. (A stuck VTEXT lasts only while
	 * the vnode stays cached: getnewvnode clears v_flag when it
	 * recycles one.) Nothing in this tree set VTEXT before this patch.
	 * The open-for-writing check above, ETXTBSY, is kept: Lite2's own
	 * execve(2) documents it.
	 */
	return exec_aout_setup_stack(p, epp);

bad:
	free((char *) ph, M_TEMP);
	kill_vmcmds(&epp->ep_vmcmds);
	return ENOEXEC;
}
