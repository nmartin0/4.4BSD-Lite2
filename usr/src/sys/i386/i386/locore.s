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
 *	from: @(#)locore.s	7.3 (Berkeley) 5/13/91
 *	from NetBSD: Id: locore.s,v 1.12 1993/05/27 16:44:13 cgd Exp
 *
 *      @(#)locore.s	8.3 (Berkeley) 9/23/93
 */


/*
 * locore.s:	4BSD machine support for the Intel 386
 *		Preliminary version
 *		Written by William F. Jolitz, 386BSD Project
 */

#include "assym.s"
#include "machine/psl.h"
#include "machine/pte.h"

#include "errno.h"

#include "machine/trap.h"

#include "machine/specialreg.h"

#ifdef cgd_notdef
#include "machine/cputypes.h"
#endif

/*
 * AI-ONLY NOTE: _C_LABEL, and ENTRY and ALTENTRY through it.
 *
 * The a.out compilers put a leading underscore on every C symbol, so
 * this file named them _main, _trap, _cnt. An ELF compiler adds
 * nothing, and the two no longer meet: the kernel link reported
 * hundreds of undefined symbols, every one of them this.
 *
 * NetBSD has carried this macro in <machine/asm.h> since 1.0 and
 * converted its i386 assembly to use it at 1.4, where locore.s went
 * from 198 bare _name references to 222 _C_LABEL uses. The same
 * macro is already in this tree's lib/libc/i386/DEFS.h and SYS.h,
 * put there for the same reason. It is defined here rather than in a
 * machine/asm.h because this tree has no such header, and because
 * locore.s includes vector.s and icu.s, so one definition serves all
 * three.
 */
#ifdef __ELF__
#define	_C_LABEL(x)	x
#else
#define	_C_LABEL(x)	_/**/x
#endif

#define	KDSEL		0x10

/*
 * Note: This version greatly munged to avoid various assembler errors
 * that may be fixed in newer versions of gas. Perhaps newer versions
 * will have more pleasant appearance.
 */

	.set	IDXSHIFT,10
	.set	SYSTEM,0xFE000000	# virtual address of system start
	/*note: gas copys sign bit (e.g. arithmetic >>), can't do SYSTEM>>22! */
	.set	SYSPDROFF,0x3F8		# Page dir index of System Base

#define	NOP	inb $0x84, %al ; inb $0x84, %al 
#define	ALIGN32	.align 2	/* 2^2  = 4 */

/*
 * PTmap is recursive pagemap at top of virtual address space.
 * Within PTmap, the page directory can be found (third indirection).
 */
	.set	PDRPDROFF,0x3F7		# Page dir index of Page dir
	.globl	_C_LABEL(PTmap), _C_LABEL(PTD), _C_LABEL(PTDpde), _C_LABEL(Sysmap)
	.set	_C_LABEL(PTmap),0xFDC00000
	.set	_C_LABEL(PTD),0xFDFF7000
	.set	_C_LABEL(Sysmap),0xFDFF8000
	.set	_C_LABEL(PTDpde),0xFDFF7000+4*PDRPDROFF

/*
 * APTmap, APTD is the alternate recursive pagemap.
 * It's used when modifying another process's page tables.
 */
	.set	APDRPDROFF,0x3FE		# Page dir index of Page dir
	.globl	_C_LABEL(APTmap), _C_LABEL(APTD), _C_LABEL(APTDpde)
	.set	_C_LABEL(APTmap),0xFF800000
	.set	_C_LABEL(APTD),0xFFBFE000
	.set	_C_LABEL(APTDpde),0xFDFF7000+4*APDRPDROFF

/*
 * Access to each processes kernel stack is via a region of
 * per-process address space (at the beginning), immediatly above
 * the user process stack.
 */
	.set	_C_LABEL(kstack), USRSTACK
	.globl	_C_LABEL(kstack)
	.set	PPDROFF,0x3F6
	.set	PPTEOFF,0x400-UPAGES	# 0x3FE

#define	ENTRY(name) \
	.globl _C_LABEL(name); _C_LABEL(name):
#define	ALTENTRY(name) \
	.globl _C_LABEL(name); _C_LABEL(name):

/*
 * Initialization
 */
	.data
	.globl	_C_LABEL(cpu),_C_LABEL(cold),_C_LABEL(boothowto),_C_LABEL(bootdev),_C_LABEL(cyloffset),_C_LABEL(atdevbase),_C_LABEL(atdevphys)
_C_LABEL(cpu):	.long	0		# are we 386, 386sx, or 486
_C_LABEL(cold):	.long	1		# cold till we are not
_C_LABEL(atdevbase):	.long	0	# location of start of iomem in virtual
_C_LABEL(atdevphys):	.long	0	# location of device mapping ptes (phys)

	.globl	_C_LABEL(IdlePTD), _C_LABEL(KPTphys)
_C_LABEL(IdlePTD):	.long	0
_C_LABEL(KPTphys):	.long	0

	.space 512
tmpstk:
	.text
	.globl	start
start:	movw	$0x1234,%ax
	movw	%ax,0x472	# warm boot
	jmp	1f
	.space	0x500		# skip over warm boot shit

	/*
	 * pass parameters on stack (howto, bootdev, unit, cyloffset)
	 * note: 0(%esp) is return address of boot
	 * ( if we want to hold onto /boot, it's physical %esp up to _C_LABEL(end))
	 */

 1:	movl	4(%esp),%eax
	movl	%eax,_C_LABEL(boothowto)-SYSTEM
	movl	8(%esp),%eax
	movl	%eax,_C_LABEL(bootdev)-SYSTEM
	movl	12(%esp),%eax
	movl	%eax, _C_LABEL(cyloffset)-SYSTEM

#ifdef cgd_notdef
	/* find out our CPU type. */
        pushfl
        popl    %eax
        movl    %eax, %ecx
        xorl    $0x40000, %eax
        pushl   %eax
        popfl
        pushfl
        popl    %eax
        xorl    %ecx, %eax
        shrl    $18, %eax
        andl    $1, %eax
        push    %ecx
        popfl
      
        cmpl    $0, %eax
        jne     1f
        movl    $CPU_386, _C_LABEL(cpu)-SYSTEM
	jmp	2f
1:      movl    $CPU_486, _C_LABEL(cpu)-SYSTEM
2:
#endif

#ifdef garbage
	/* count up memory */

	xorl	%eax,%eax		# start with base memory at 0x0
	#movl	$ 0xA0000/NBPG,%ecx	# look every 4K up to 640K
	movl	$ 0xA0,%ecx		# look every 4K up to 640K
1:	movl	0(%eax),%ebx		# save location to check
	movl	$0xa55a5aa5,0(%eax)	# write test pattern
	/* flush stupid cache here! (with bcopy (0,0,512*1024) ) */
	cmpl	$0xa55a5aa5,0(%eax)	# does not check yet for rollover
	jne	2f
	movl	%ebx,0(%eax)		# restore memory
	addl	$ NBPG,%eax
	loop	1b
2:	shrl	$12,%eax
	movl	%eax,_C_LABEL(Maxmem)-SYSTEM

	movl	$0x100000,%eax		# next, talley remaining memory
	#movl	$((0xFFF000-0x100000)/NBPG),%ecx
	movl	$(0xFFF-0x100),%ecx
1:	movl	0(%eax),%ebx		# save location to check
	movl	$0xa55a5aa5,0(%eax)	# write test pattern
	cmpl	$0xa55a5aa5,0(%eax)	# does not check yet for rollover
	jne	2f
	movl	%ebx,0(%eax)		# restore memory
	addl	$ NBPG,%eax
	loop	1b
2:	shrl	$12,%eax
	movl	%eax,_C_LABEL(Maxmem)-SYSTEM
#endif

/* find end of kernel image */
	movl	$_end-SYSTEM,%ecx
	addl	$ NBPG-1,%ecx
	andl	$~(NBPG-1),%ecx
	movl	%ecx,%esi

/* clear bss and memory for bootstrap pagetables. */
	movl	$_edata-SYSTEM,%edi
	subl	%edi,%ecx
	addl	$(UPAGES+5)*NBPG,%ecx
/*
 * Virtual address space of kernel:
 *
 *	text | data | bss | page dir | proc0 kernel stack | usr stk map | Sysmap
 *			     0               1       2       3             4
 */
	xorl	%eax,%eax	# pattern
	cld
	rep
	stosb

	movl	%esi,_C_LABEL(IdlePTD)-SYSTEM /*physical address of Idle Address space */
	movl	$ tmpstk-SYSTEM,%esp	# bootstrap stack end location

#define	fillkpt		\
1:	movl	%eax,0(%ebx)	; \
	addl	$ NBPG,%eax	; /* increment physical address */ \
	addl	$4,%ebx		; /* next pte */ \
	loop	1b		;

/*
 * Map Kernel
 * N.B. don't bother with making kernel text RO, as 386
 * ignores R/W AND U/S bits on kernel access (only v works) !
 *
 * First step - build page tables
 */
	movl	%esi,%ecx		# this much memory,
	shrl	$ PGSHIFT,%ecx		# for this many pte s
	addl	$ UPAGES+4,%ecx		# including our early context
	movl	$ PG_V,%eax		#  having these bits set,
	lea	(4*NBPG)(%esi),%ebx	#   physical address of KPT in proc 0,
	movl	%ebx,_C_LABEL(KPTphys)-SYSTEM	#    in the kernel page table,
	fillkpt

/* map I/O memory map */

	movl	$0x100-0xa0,%ecx	# for this many pte s,
	movl	$(0xa0000|PG_V|PG_UW),%eax # having these bits set,(perhaps URW?) XXX 06 Aug 92
	movl	%ebx,_C_LABEL(atdevphys)-SYSTEM	#   remember phys addr of ptes
	fillkpt

 /* map proc 0's kernel stack into user page table page */

	movl	$ UPAGES,%ecx		# for this many pte s,
	lea	(1*NBPG)(%esi),%eax	# physical address in proc 0
	lea	(SYSTEM)(%eax),%edx
	movl	%edx,_C_LABEL(proc0paddr)-SYSTEM  # remember VA for 0th process init
	orl	$ PG_V|PG_URKW,%eax	#  having these bits set,
	lea	(3*NBPG)(%esi),%ebx	# physical address of stack pt in proc 0
	addl	$(PPTEOFF*4),%ebx
	fillkpt

/*
 * Construct a page table directory
 * (of page directory elements - pde's)
 */
	/* install a pde for temporary double map of bottom of VA */
	lea	(4*NBPG)(%esi),%eax	# physical address of kernel page table
	orl     $ PG_V|PG_UW,%eax	# pde entry is valid XXX 06 Aug 92
	movl	%eax,(%esi)		# which is where temp maps!

	/* kernel pde's */
	movl	$ 3,%ecx		# for this many pde s,
	lea	(SYSPDROFF*4)(%esi), %ebx	# offset of pde for kernel
	fillkpt

	/* install a pde recursively mapping page directory as a page table! */
	movl	%esi,%eax		# phys address of ptd in proc 0
	orl	$ PG_V|PG_UW,%eax	# pde entry is valid XXX 06 Aug 92
	movl	%eax, PDRPDROFF*4(%esi)	# which is where PTmap maps!

	/* install a pde to map kernel stack for proc 0 */
	lea	(3*NBPG)(%esi),%eax	# physical address of pt in proc 0
	orl	$ PG_V,%eax		# pde entry is valid
	movl	%eax,PPDROFF*4(%esi)	# which is where kernel stack maps!

	/* load base of page directory, and enable mapping */
	movl	%esi,%eax		# phys address of ptd in proc 0
 	orl	$ I386_CR3PAT,%eax
	movl	%eax,%cr3		# load ptd addr into mmu
	movl	%cr0,%eax		# get control word
	orl	$0x80000001,%eax	# and let s page!
	movl	%eax,%cr0		# NOW!

	pushl	$begin				# jump to high mem!
	ret

begin: /* now running relocated at SYSTEM where the system is linked to run */

	.globl _C_LABEL(Crtat)
	movl	_C_LABEL(Crtat),%eax
	subl	$0xfe0a0000,%eax
	movl	_C_LABEL(atdevphys),%edx	# get pte PA
	subl	_C_LABEL(KPTphys),%edx	# remove base of ptes, now have phys offset
	shll	$ PGSHIFT-2,%edx  # corresponding to virt offset
	addl	$ SYSTEM,%edx	# add virtual base
	movl	%edx, _C_LABEL(atdevbase)
	addl	%eax,%edx
	movl	%edx,_C_LABEL(Crtat)

	/* set up bootstrap stack */
	movl	$ _C_LABEL(kstack)+UPAGES*NBPG-4*12,%esp	# bootstrap stack end location
	xorl	%eax,%eax		# mark end of frames
	movl	%eax,%ebp
	movl	_C_LABEL(proc0paddr), %eax
	movl	%esi, PCB_CR3(%eax)

	lea	7*NBPG(%esi),%esi	# skip past stack.
	pushl	%esi
	
	call	_C_LABEL(init386)		# wire 386 chip for unix operation
	
	movl	$0,_C_LABEL(PTD)
	call 	_C_LABEL(main)
	popl	%esi

	.globl	_C_LABEL(_ucodesel),_C_LABEL(_udatasel)
	movzwl	_C_LABEL(_ucodesel),%eax
	movzwl	_C_LABEL(_udatasel),%ecx
	# build outer stack frame
	pushl	%ecx		# user ss
	pushl	$ USRSTACK	# user esp
	pushl	%eax		# user cs
	pushl	$0		# user ip
	movw	%cx,%ds
	movw	%cx,%es
	movw	%ax,%fs		# double map cs to fs
	movw	%cx,%gs		# and ds to gs
	lret	# goto user!

	pushl	$lretmsg1	/* "should never get here!" */
	call	_C_LABEL(panic)
lretmsg1:
	.asciz	"lret: toinit\n"


	.set	exec,59
	.set	exit,1
	.globl	_C_LABEL(icode)
	.globl	_C_LABEL(szicode)

#define	LCALL(x,y)	.byte 0x9a ; .long y; .word x
/*
 * Icode is copied out to process 1 to exec /etc/init.
 * If the exec fails, process 1 exits.
 */
_C_LABEL(icode):
	# pushl	$argv-_icode	# gas fucks up again
	movl	$argv,%eax
	subl	$_icode,%eax
	pushl	%eax

	# pushl	$init-_icode
	movl	$init,%eax
	subl	$_icode,%eax
	pushl	%eax
	pushl	%eax	# dummy out rta

	movl	%esp,%ebp
	movl	$exec,%eax
	LCALL(0x7,0x0)
	pushl	%eax
	movl	$exit,%eax
	pushl	%eax	# dummy out rta
	LCALL(0x7,0x0)

init:
	.asciz	"/sbin/init"
	.align	2
argv:
	.long	init+6-_C_LABEL(icode)		# argv[0] = "init" ("/sbin/init" + 6)
	.long	eicode-_C_LABEL(icode)		# argv[1] follows icode after copyout
	.long	0
eicode:

_C_LABEL(szicode):
	.long	_C_LABEL(szicode)-_C_LABEL(icode)

	.globl	_C_LABEL(sigcode),_C_LABEL(szsigcode)
_C_LABEL(sigcode):
	movl	12(%esp),%eax	# unsure if call will dec stack 1st
	call	%eax
	xorl	%eax,%eax	# smaller movl $103,%eax
	movb	$103,%al	# sigreturn()
	LCALL(0x7,0)		# enter kernel with args on stack
	hlt			# never gets here

_C_LABEL(szsigcode):
	.long	_C_LABEL(szsigcode)-_C_LABEL(sigcode)

	/*
	 * Support routines for GCC
	 */
	.globl _C_LABEL(__udivsi3)
	ALIGN32
_C_LABEL(__udivsi3):
	movl 4(%esp),%eax
	xorl %edx,%edx
	divl 8(%esp)
	ret

	.globl _C_LABEL(__divsi3)
	ALIGN32
_C_LABEL(__divsi3):
	movl 4(%esp),%eax
	#xorl %edx,%edx		/* not needed - cltd sign extends into %edx */
	cltd
	idivl 8(%esp)
	ret

	/*
	 * I/O bus instructions via C
	 */
	.globl	_C_LABEL(inb)
	ALIGN32
_C_LABEL(inb):	movl	4(%esp),%edx
	subl	%eax,%eax	# clr eax
	NOP
	inb	%dx,%al
	ret


	.globl	_C_LABEL(inw)
	ALIGN32
_C_LABEL(inw):	movl	4(%esp),%edx
	subl	%eax,%eax	# clr eax
	NOP
	inw	%dx,%ax
	ret


	.globl	_C_LABEL(rtcin)
	ALIGN32
_C_LABEL(rtcin):	movl	4(%esp),%eax
	outb	%al,$0x70
	subl	%eax,%eax	# clr eax
	inb	$0x71,%al	# Compaq SystemPro 
	ret

	.globl	_C_LABEL(outb)
	ALIGN32
_C_LABEL(outb):	movl	4(%esp),%edx
	NOP
	movl	8(%esp),%eax
	outb	%al,%dx
	NOP
	ret

	.globl	_C_LABEL(outw)
	ALIGN32
_C_LABEL(outw):	movl	4(%esp),%edx
	NOP
	movl	8(%esp),%eax
	outw	%ax,%dx
	NOP
	ret

	/*
	 * void bzero(void *base, u_int cnt)
	 */

	.globl _C_LABEL(bzero)
	ALIGN32
_C_LABEL(bzero):
	pushl	%edi
	movl	8(%esp),%edi
	movl	12(%esp),%ecx
	xorl	%eax,%eax
	shrl	$2,%ecx	
	cld
	rep
	stosl
	movl	12(%esp),%ecx
	andl	$3,%ecx
	rep
	stosb
	popl	%edi
	ret

	/*
	 * fillw (pat,base,cnt)
	 */

	.globl _C_LABEL(fillw)
	ALIGN32
_C_LABEL(fillw):
	pushl	%edi
	movl	8(%esp),%eax
	movl	12(%esp),%edi
	movw	%ax, %cx
	rorl	$16, %eax
	movw	%cx, %ax
	cld
	movl	16(%esp),%ecx
	shrl	%ecx
	rep
	stosl
	movl	16(%esp),%ecx
	andl	$1, %ecx
	rep
	stosw
	popl	%edi
	ret

	.globl _C_LABEL(bcopyb)
	ALIGN32
_C_LABEL(bcopyb):
	pushl	%esi
	pushl	%edi
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	cld
	rep
	movsb
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	ret

	/*
	 * (ov)bcopy (src,dst,cnt)
	 *  ws@tools.de     (Wolfgang Solfrank, TooLs GmbH) +49-228-985800
	 */

	.globl	_C_LABEL(bcopy),_C_LABEL(ovbcopy)
	ALIGN32
_C_LABEL(ovbcopy):
_C_LABEL(bcopy):
	pushl	%esi
	pushl	%edi
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	cmpl	%esi,%edi	/* potentially overlapping? */
	jnb	1f
	cld			/* nope, copy forwards. */
	shrl	$2,%ecx		/* copy by words */
	rep
	movsl
	movl	20(%esp),%ecx
	andl	$3,%ecx		/* any bytes left? */
	rep
	movsb
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	ret
	ALIGN32
1:
	addl	%ecx,%edi	/* copy backwards. */
	addl	%ecx,%esi
	std
	andl	$3,%ecx		/* any fractional bytes? */
	decl	%edi
	decl	%esi
	rep
	movsb
	movl	20(%esp),%ecx	/* copy remainder by words */
	shrl	$2,%ecx
	subl	$3,%esi
	subl	$3,%edi
	rep
	movsl
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	cld
	ret

#ifdef notdef
	.globl	_C_LABEL(copyout)
	ALIGN32
_C_LABEL(copyout):
	movl	_C_LABEL(curpcb), %eax
	movl	$cpyflt, PCB_ONFAULT(%eax) # in case we page/protection violate
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	movl	16(%esp), %esi
	movl	20(%esp), %edi
	movl	24(%esp), %ebx

 				/* first, check to see if "write fault" */
1:	movl	%edi, %eax
#ifdef notyet
	shrl	$IDXSHIFT, %eax	/* fetch pte associated with address */
	andb	$0xfc, %al
	movl	_C_LABEL(PTmap)(%eax), %eax

	andb	$7, %al		/* if we are the one case that won't trap... */
	cmpb	$5, %al
	jne	2f
				/* ... then simulate the trap! */
	pushl	%edi
	call	_C_LABEL(trapwrite)	/* trapwrite(addr) */
	popl	%edx

	cmpl	$0, %eax	/* if not ok, return */
	jne	cpyflt
				/* otherwise, continue with reference */
2:
	movl	%edi, %eax	/* calculate remainder this pass */
	andl	$0xfffff000, %eax
	movl	$NBPG, %ecx
	subl	%eax, %ecx
	cmpl	%ecx, %ebx
	jle	3f
	movl	%ebx, %ecx
3:	subl	%ecx, %ebx
	movl	%ecx, %edx
#else
	movl	%ebx, %ecx
	movl	%ebx, %edx
#endif

	shrl	$2,%ecx			/* movem */
	cld
	rep
	movsl
	movl	%edx, %ecx		/* don't depend on ecx here! */
	andl	$3, %ecx
	rep
	movsb

#ifdef notyet
	cmpl	$0, %ebx
	jl	1b
#endif

	popl	%ebx
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	movl	_C_LABEL(curpcb),%edx
	movl	%eax,PCB_ONFAULT(%edx)
	ret

	.globl	_C_LABEL(copyin)
	ALIGN32
_C_LABEL(copyin):
	movl	_C_LABEL(curpcb),%eax
	movl	$cpyflt,PCB_ONFAULT(%eax) # in case we page/protection violate
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	shrl	$2,%ecx
	cld
	rep
	movsl
	movl	20(%esp),%ecx
	andl	$3,%ecx
	rep
	movsb
	popl	%ebx
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	movl	_C_LABEL(curpcb),%edx
	movl	%eax,PCB_ONFAULT(%edx)
	ret

	ALIGN32
cpyflt:
	popl	%ebx
	popl	%edi
	popl	%esi
	movl	_C_LABEL(curpcb),%edx
	movl	$0,PCB_ONFAULT(%edx)
	movl	$ EFAULT,%eax
	ret
#else
	.globl	_C_LABEL(copyout)
	ALIGN32
_C_LABEL(copyout):
	movl	_C_LABEL(curpcb),%eax
	movl	$cpyflt,PCB_ONFAULT(%eax) # in case we page/protection violate
	pushl	%esi
	pushl	%edi
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	shrl	$2,%ecx
	cld
	rep
	movsl
	movl	20(%esp),%ecx
	andl	$3,%ecx
	rep
	movsb
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	movl	_C_LABEL(curpcb),%edx
	movl	%eax,PCB_ONFAULT(%edx)
	ret

	.globl	_C_LABEL(copyin)
	ALIGN32
_C_LABEL(copyin):
	movl	_C_LABEL(curpcb),%eax
	movl	$cpyflt,PCB_ONFAULT(%eax) # in case we page/protection violate
	pushl	%esi
	pushl	%edi
	movl	12(%esp),%esi
	movl	16(%esp),%edi
	movl	20(%esp),%ecx
	shrl	$2,%ecx
	cld
	rep
	movsl
	movl	20(%esp),%ecx
	andl	$3,%ecx
	rep
	movsb
	popl	%edi
	popl	%esi
	xorl	%eax,%eax
	movl	_C_LABEL(curpcb),%edx
	movl	%eax,PCB_ONFAULT(%edx)
	ret

	ALIGN32
cpyflt: popl	%edi
	popl	%esi
	movl	_C_LABEL(curpcb),%edx
	movl	$0,PCB_ONFAULT(%edx)
	movl	$ EFAULT,%eax
	ret

#endif

	# insb(port,addr,cnt)
	.globl	_C_LABEL(insb)
	ALIGN32
_C_LABEL(insb):
	pushl	%edi
	movw	8(%esp),%dx
	movl	12(%esp),%edi
	movl	16(%esp),%ecx
	cld
	NOP
	rep
	insb
	NOP
	movl	%edi,%eax
	popl	%edi
	ret

	# insw(port,addr,cnt)
	.globl	_C_LABEL(insw)
	ALIGN32
_C_LABEL(insw):
	pushl	%edi
	movw	8(%esp),%dx
	movl	12(%esp),%edi
	movl	16(%esp),%ecx
	cld
	NOP
	.byte 0x66,0xf2,0x6d	# rep insw
	NOP
	movl	%edi,%eax
	popl	%edi
	ret

	# outsw(port,addr,cnt)
	.globl	_C_LABEL(outsw)
	ALIGN32
_C_LABEL(outsw):
	pushl	%esi
	movw	8(%esp),%dx
	movl	12(%esp),%esi
	movl	16(%esp),%ecx
	cld
	NOP
	.byte 0x66,0xf2,0x6f	# rep outsw
	NOP
	movl	%esi,%eax
	popl	%esi
	ret

	# outsb(port,addr,cnt)
	.globl	_C_LABEL(outsb)
	ALIGN32
_C_LABEL(outsb):
	pushl	%esi
	movw	8(%esp),%dx
	movl	12(%esp),%esi
	movl	16(%esp),%ecx
	cld
	NOP
	rep
	outsb
	NOP
	movl	%esi,%eax
	popl	%esi
	ret

	/*
	 * void lgdt(struct region_descriptor *rdp);
	 */
	.globl	_C_LABEL(lgdt)
	ALIGN32
_C_LABEL(lgdt):
	/* reload the descriptor table */
	movl	4(%esp),%eax
	lgdt	(%eax)
	/* flush the prefetch q */
	jmp	1f
	nop
1:
	/* reload "stale" selectors */
	# movw	$KDSEL,%ax
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss

	/* reload code selector by turning return into intersegmental return */
	movl	0(%esp),%eax
	pushl	%eax
	# movl	$KCSEL,4(%esp)
	movl	$8,4(%esp)
	lret

	/*
	 * void lidt(struct region_descriptor *rdp);
	 */
	.globl	_C_LABEL(lidt)
	ALIGN32
_C_LABEL(lidt):
	movl	4(%esp),%eax
	lidt	(%eax)
	ret

	/*
	 * void lldt(u_short sel)
	 */
	.globl	_C_LABEL(lldt)
	ALIGN32
_C_LABEL(lldt):
	lldt	4(%esp)
	ret

	/*
	 * void ltr(u_short sel)
	 */
	.globl	_C_LABEL(ltr)
	ALIGN32
_C_LABEL(ltr):
	ltr	4(%esp)
	ret

	/*
	 * void lcr3(caddr_t cr3)
	 */
	.globl	_C_LABEL(lcr3)
	.globl	_C_LABEL(load_cr3)
	ALIGN32
_C_LABEL(load_cr3):
_C_LABEL(lcr3):
	inb	$0x84,%al	# check wristwatch
	movl	4(%esp),%eax
 	orl	$ I386_CR3PAT,%eax
	movl	%eax,%cr3
	inb	$0x84,%al	# check wristwatch
	ret

	# tlbflush()
	.globl	_C_LABEL(tlbflush)
	ALIGN32
_C_LABEL(tlbflush):
	inb	$0x84,%al	# check wristwatch
	movl	%cr3,%eax
 	orl	$ I386_CR3PAT,%eax
	movl	%eax,%cr3
	inb	$0x84,%al	# check wristwatch
	ret

	# lcr0(cr0)
	.globl	_C_LABEL(lcr0),_C_LABEL(load_cr0)
	ALIGN32
_C_LABEL(lcr0):
_C_LABEL(load_cr0):
	movl	4(%esp),%eax
	movl	%eax,%cr0
	ret

	# rcr0()
	.globl	_C_LABEL(rcr0)
	ALIGN32
_C_LABEL(rcr0):
	movl	%cr0,%eax
	ret

	# rcr2()
	.globl	_C_LABEL(rcr2)
	ALIGN32
_C_LABEL(rcr2):
	movl	%cr2,%eax
	ret

	# rcr3()
	.globl	_C_LABEL(rcr3)
	.globl	_C_LABEL(_cr3)
	ALIGN32
_C_LABEL(_cr3):
_C_LABEL(rcr3):
	movl	%cr3,%eax
	ret

	# ssdtosd(*ssdp,*sdp)
	.globl	_C_LABEL(ssdtosd)
	ALIGN32
_C_LABEL(ssdtosd):
	pushl	%ebx
	movl	8(%esp),%ecx
	movl	8(%ecx),%ebx
	shll	$16,%ebx
	movl	(%ecx),%edx
	roll	$16,%edx
	movb	%dh,%bl
	movb	%dl,%bh
	rorl	$8,%ebx
	movl	4(%ecx),%eax
	movw	%ax,%dx
	andl	$0xf0000,%eax
	orl	%eax,%ebx
	movl	12(%esp),%ecx
	movl	%edx,(%ecx)
	movl	%ebx,4(%ecx)
	popl	%ebx
	ret

/*
 * {fu,su},{byte,word}
 */
	ALIGN32
ALTENTRY(fuiword)
ENTRY(fuword)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx)
	movl	4(%esp),%edx
	.byte	0x65		# use gs
	movl	0(%edx),%eax
	movl	$0,PCB_ONFAULT(%ecx)
	ret
	
	ALIGN32
ENTRY(fusword)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx) #in case we page/protection violate
	movl	4(%esp),%edx
	.byte	0x65		# use gs
	movzwl	0(%edx),%eax
	movl	$0,PCB_ONFAULT(%ecx)
	ret
	
	ALIGN32
ALTENTRY(fuibyte)
ENTRY(fubyte)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx) #in case we page/protection violate
	movl	4(%esp),%edx
	.byte	0x65		# use gs
	movzbl	0(%edx),%eax
	movl	$0,PCB_ONFAULT(%ecx)
	ret
	
	ALIGN32
fusufault:
	movl	_C_LABEL(curpcb),%ecx
	xorl	%eax,%eax
	movl	%eax,PCB_ONFAULT(%ecx) #in case we page/protection violate
	decl	%eax
	ret

	ALIGN32
ALTENTRY(suiword)
ENTRY(suword)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx) #in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax

#ifdef notdef
	shrl	$IDXSHIFT, %edx	/* fetch pte associated with address */
	andb	$0xfc, %dl
	movl	_C_LABEL(PTmap)(%edx), %edx

	andb	$7, %dl		/* if we are the one case that won't trap... */
	cmpb	$5 , %edx
	jne	1f
				/* ... then simulate the trap! */
	pushl	%edi
	call	_C_LABEL(trapwrite)	/* trapwrite(addr) */
	popl	%edx
	cmpl	$0, %eax	/* if not ok, return */
	jne	fusufault
	movl	8(%esp),%eax	/* otherwise, continue with reference */
1:
	movl	4(%esp),%edx
#endif
	.byte	0x65		# use gs
	movl	%eax,0(%edx)
	xorl	%eax,%eax
	movl	%eax,PCB_ONFAULT(%ecx) #in case we page/protection violate
	ret
	
	ALIGN32
ENTRY(susword)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx) #in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax
#ifdef notdef
shrl	$IDXSHIFT, %edx	/* calculate pte address */
andb	$0xfc, %dl
movl	_C_LABEL(PTmap)(%edx), %edx
andb	$7, %edx	/* if we are the one case that won't trap... */
cmpb	$5 , %edx
jne	1f
/* ..., then simulate the trap! */
	pushl	%edi
	call	_C_LABEL(trapwrite)	/* trapwrite(addr) */
	popl	%edx
movl	_C_LABEL(curpcb), %ecx	# restore trashed registers
cmpl	$0, %eax	/* if not ok, return */
jne	fusufault
movl	8(%esp),%eax
1: movl	4(%esp),%edx
#endif
	.byte	0x65		# use gs
	movw	%ax,0(%edx)
	xorl	%eax,%eax
	movl	%eax,PCB_ONFAULT(%ecx) #in case we page/protection violate
	ret

	ALIGN32
ALTENTRY(suibyte)
ENTRY(subyte)
	movl	_C_LABEL(curpcb),%ecx
	movl	$fusufault,PCB_ONFAULT(%ecx) #in case we page/protection violate
	movl	4(%esp),%edx
	movl	8(%esp),%eax
#ifdef notdef
shrl	$IDXSHIFT, %edx	/* calculate pte address */
andb	$0xfc, %dl
movl	_C_LABEL(PTmap)(%edx), %edx
andb	$7, %edx	/* if we are the one case that won't trap... */
cmpb	$5 , %edx
jne	1f
/* ..., then simulate the trap! */
	pushl	%edi
	call	_C_LABEL(trapwrite)	/* trapwrite(addr) */
	popl	%edx
movl	_C_LABEL(curpcb), %ecx	# restore trashed registers
cmpl	$0, %eax	/* if not ok, return */
jne	fusufault
movl	8(%esp),%eax
1: movl	4(%esp),%edx
#endif
	.byte	0x65		# use gs
/*
 * AI-ONLY NOTE: %al, not %eax. A byte move names a byte register;
 * the assembler of the day took the 32-bit name and emitted the same
 * instruction, and gas now refuses: "`%eax' not allowed with `movb'".
 * NetBSD 1.0 and OpenBSD 1996 both write `movb %al,(%edx)' in this
 * routine. The encoding is unchanged.
 */
	movb	%al,0(%edx)
	xorl	%eax,%eax
	movl	%eax,PCB_ONFAULT(%ecx) #in case we page/protection violate
	ret

	ALIGN32
	ENTRY(setjmp)
	movl	4(%esp),%eax
	movl	%ebx, 0(%eax)		# save ebx
	movl	%esp, 4(%eax)		# save esp
	movl	%ebp, 8(%eax)		# save ebp
	movl	%esi,12(%eax)		# save esi
	movl	%edi,16(%eax)		# save edi
	movl	(%esp),%edx		# get rta
	movl	%edx,20(%eax)		# save eip
	xorl	%eax,%eax		# return (0);
	ret

	ALIGN32
	ENTRY(longjmp)
	movl	4(%esp),%eax
	movl	 0(%eax),%ebx		# restore ebx
	movl	 4(%eax),%esp		# restore esp
	movl	 8(%eax),%ebp		# restore ebp
	movl	12(%eax),%esi		# restore esi
	movl	16(%eax),%edi		# restore edi
	movl	20(%eax),%edx		# get rta
	movl	%edx,(%esp)		# put in return frame
	xorl	%eax,%eax		# return (1);
	incl	%eax
	ret

/*
 * The following primitives manipulate the run queues.  _C_LABEL(whichqs) tells which
 * of the 32 queues _C_LABEL(qs) have processes in them.  Setrunqueue puts processes
 * into queues, Remrq removes them from queues.  The running process is on
 * no queue, other processes are on a queue related to p->p_priority, divided
 * by 4 actually to shrink the 0-127 range of priorities into the 32 available
 * queues.
 */
	.globl	_C_LABEL(whichqs),_C_LABEL(qs),_C_LABEL(cnt),_C_LABEL(panic)
	.comm	_C_LABEL(noproc),4
	.comm	_C_LABEL(runrun),4

/*
 * Setrq(p)
 *
 * Call should be made at spl6(), and p->p_stat should be SRUN
 */
	ALIGN32
ENTRY(setrunqueue)
	movl	4(%esp),%eax
	cmpl	$0,P_BACK(%eax)		# should not be on q already
	je	set1
	pushl	$set2
	call	_C_LABEL(panic)
set1:
	movzbl	P_PRIORITY(%eax),%edx
	shrl	$2,%edx
	btsl	%edx,_C_LABEL(whichqs)		# set q full bit
	shll	$3,%edx
	addl	$_qs,%edx		# locate q hdr
	movl	%edx,P_FORW(%eax)	# link process on tail of q
	movl	P_BACK(%edx),%ecx
	movl	%ecx,P_BACK(%eax)
	movl	%eax,P_BACK(%edx)
	movl	%eax,P_FORW(%ecx)
	ret

set2:	.asciz	"setrunqueue"

/*
 * Remrq(p)
 *
 * Call should be made at spl6().
 */
	ALIGN32
ENTRY(remrq)
	movl	4(%esp),%eax
	movzbl	P_PRIORITY(%eax),%edx
	shrl	$2,%edx
	btrl	%edx,_C_LABEL(whichqs)		# clear full bit, panic if clear already
	jb	rem1
	pushl	$rem3
	call	_C_LABEL(panic)
rem1:
	pushl	%edx
	movl	P_FORW(%eax),%ecx	# unlink process
	movl	P_BACK(%eax),%edx
	movl	%edx,P_BACK(%ecx)
	movl	P_BACK(%eax),%ecx
	movl	P_FORW(%eax),%edx
	movl	%edx,P_FORW(%ecx)
	popl	%edx
	movl	$_qs,%ecx
	shll	$3,%edx
	addl	%edx,%ecx
	cmpl	P_FORW(%ecx),%ecx	# q still has something?
	je	rem2
	shrl	$3,%edx			# yes, set bit as still full
	btsl	%edx,_C_LABEL(whichqs)
rem2:
	movl	$0,P_BACK(%eax)		# zap reverse link to indicate off list
	ret

rem3:	.asciz	"remrq"
sw0:	.asciz	"Xswitch"

/*
 * When no processes are on the runq, Swtch branches to idle
 * to wait for something to come ready.
 */
	.globl	Idle
	ALIGN32
Idle:
idle:
	call	_C_LABEL(spl0)
	cmpl	$0,_C_LABEL(whichqs)
	jne	sw1
	hlt		# wait for interrupt
	jmp	idle

	.align 4 /* ..so that profiling doesn't lump Idle with Xswitch().. */
badsw:
	pushl	$sw0
	call	_C_LABEL(panic)
	/*NOTREACHED*/

/*
 * Swtch()
 */
	ALIGN32
ENTRY(Xswitch)

	incl	_C_LABEL(cnt)+V_SWTCH

	/* switch to new process. first, save context as needed */

	movl	_C_LABEL(curproc),%ecx

	/* if no process to save, don't bother */
	cmpl	$0,%ecx
	je	sw1

	movl	P_ADDR(%ecx),%ecx


	movl	(%esp),%eax		# Hardware registers
	movl	%eax, PCB_EIP(%ecx)
	movl	%ebx, PCB_EBX(%ecx)
	movl	%esp, PCB_ESP(%ecx)
	movl	%ebp, PCB_EBP(%ecx)
	movl	%esi, PCB_ESI(%ecx)
	movl	%edi, PCB_EDI(%ecx)

#ifdef NPX
	/* have we used fp, and need a save? */
	mov	_C_LABEL(curproc),%eax
	cmp	%eax,_C_LABEL(npxproc)
	jne	1f
	pushl	%ecx			/* h/w bugs make saving complicated */
	leal	PCB_SAVEFPU(%ecx),%eax
	pushl	%eax
	call	_C_LABEL(npxsave)		/* do it in a big C function */
	popl	%eax
	popl	%ecx
1:
#endif

	movl	_C_LABEL(CMAP2),%eax		# save temporary map PTE
	movl	%eax,PCB_CMAP2(%ecx)	# in our context
	movl	$0,_C_LABEL(curproc)		#  out of process

	# movw	_cpl, %ax
	# movw	%ax, PCB_IML(%ecx)	# save ipl

	/* save is done, now choose a new process or idle */
sw1:
	movl	_C_LABEL(whichqs),%edi
2:
	cli
	bsfl	%edi,%eax		# find a full q
	jz	idle			# if none, idle
	# XX update whichqs?
swfnd:
	btrl	%eax,%edi		# clear q full status
	jnb	2b		# if it was clear, look for another
	movl	%eax,%ebx		# save which one we are using

	shll	$3,%eax
	addl	$_qs,%eax		# select q
	movl	%eax,%esi

#ifdef	DIAGNOSTIC
	cmpl	P_FORW(%eax),%eax # linked to self? (e.g. not on list)
	je	badsw			# not possible
#endif

	movl	P_FORW(%eax),%ecx	# unlink from front of process q
	movl	P_FORW(%ecx),%edx
	movl	%edx,P_FORW(%eax)
	movl	P_BACK(%ecx),%eax
	movl	%eax,P_BACK(%edx)

	cmpl	P_FORW(%ecx),%esi	# q empty
	je	3f
	btsl	%ebx,%edi		# nope, set to indicate full
3:
	movl	%edi,_C_LABEL(whichqs)		# update q status

	movl	$0,%eax
	movl	%eax,_C_LABEL(want_resched)

#ifdef	DIAGNOSTIC
	cmpl	%eax,P_WCHAN(%ecx)
	jne	badsw
	cmpb	$ SRUN,P_STAT(%ecx)
	jne	badsw
#endif

	movl	%eax,P_BACK(%ecx)	/* isolate process to run */
	movl	P_ADDR(%ecx),%edx
	movl	PCB_CR3(%edx),%ebx

	/* switch address space */
	movl	%ebx,%cr3

	/* restore context */
	movl	PCB_EBX(%edx), %ebx
	movl	PCB_ESP(%edx), %esp
	movl	PCB_EBP(%edx), %ebp
	movl	PCB_ESI(%edx), %esi
	movl	PCB_EDI(%edx), %edi
	movl	PCB_EIP(%edx), %eax
	movl	%eax, (%esp)

	movl	PCB_CMAP2(%edx),%eax	# get temporary map
	movl	%eax,_C_LABEL(CMAP2)		# reload temporary map PTE

	movl	%ecx,_C_LABEL(curproc)		# into next process
	movl	%edx,_C_LABEL(curpcb)

	/* pushl	PCB_IML(%edx)
	call	_C_LABEL(splx)
	popl	%eax*/

	movl	%edx,%eax		# return (1);
	ret

	.globl	_C_LABEL(mvesp)
	ALIGN32
_C_LABEL(mvesp):	movl	%esp,%eax
	ret
/*
 * struct proc *switch_to_inactive(p) ; struct proc *p;
 *
 * At exit of a process, move off the address space of the
 * process and onto a "safe" one. Then, on a temporary stack
 * return and run code that disposes of the old state.
 * Since this code requires a parameter from the "old" stack,
 * pass it back as a return value.
 */
	ALIGN32
ENTRY(switch_to_inactive)
	popl	%edx			# old pc
	popl	%eax			# arg, our return value
	movl	_C_LABEL(IdlePTD),%ecx
	movl	%ecx,%cr3		# good bye address space
 #write buffer?
	movl	$tmpstk-4,%esp		# temporary stack, compensated for call
	jmp	%edx			# return, execute remainder of cleanup

/*
 * savectx(pcb, altreturn)
 * Update pcb, saving current processor state and arranging
 * for alternate return ala longjmp in Xswitch if altreturn is true.
 */
	ALIGN32
ENTRY(savectx)
	movl	4(%esp), %ecx
	movw	_C_LABEL(cpl), %ax
	movw	%ax,  PCB_IML(%ecx)
	movl	(%esp), %eax	
	movl	%eax, PCB_EIP(%ecx)
	movl	%ebx, PCB_EBX(%ecx)
	movl	%esp, PCB_ESP(%ecx)
	movl	%ebp, PCB_EBP(%ecx)
	movl	%esi, PCB_ESI(%ecx)
	movl	%edi, PCB_EDI(%ecx)

#ifdef NPX
	/*
	 * If npxproc == NULL, then the npx h/w state is irrelevant and the
	 * state had better already be in the pcb.  This is true for forks
	 * but not for dumps (the old book-keeping with FP flags in the pcb
	 * always lost for dumps because the dump pcb has 0 flags).
	 *
	 * If npxproc != NULL, then we have to save the npx h/w state to
	 * npxproc's pcb and copy it to the requested pcb, or save to the
	 * requested pcb and reload.  Copying is easier because we would
	 * have to handle h/w bugs for reloading.  We used to lose the
	 * parent's npx state for forks by forgetting to reload.
	 */
	mov	_C_LABEL(npxproc),%eax
	testl	%eax,%eax
  	je	1f

	pushl	%ecx
	movl	P_ADDR(%eax),%eax
	leal	PCB_SAVEFPU(%eax),%eax
	pushl	%eax
	pushl	%eax
	call	_C_LABEL(npxsave)
	popl	%eax
	popl	%eax
	popl	%ecx

	pushl	%ecx
	pushl	$108+8*2	/* XXX h/w state size + padding */
	leal	PCB_SAVEFPU(%ecx),%ecx
	pushl	%ecx
	pushl	%eax
	call	_C_LABEL(bcopy)
	addl	$12,%esp
	popl	%ecx
1:
#endif

	movl	_C_LABEL(CMAP2), %edx		# save temporary map PTE
	movl	%edx, PCB_CMAP2(%ecx)	# in our context

	cmpl	$0, 8(%esp)
	je	1f
	movl	%esp, %edx		# relocate current sp relative to pcb
	subl	$_kstack, %edx		#   (sp is relative to kstack):
	addl	%edx, %ecx		#   pcb += sp - kstack;
	movl	%eax, (%ecx)		# write return pc at (relocated) sp@
	# this mess deals with replicating register state gcc hides
	movl	12(%esp),%eax
	movl	%eax,12(%ecx)
	movl	16(%esp),%eax
	movl	%eax,16(%ecx)
	movl	20(%esp),%eax
	movl	%eax,20(%ecx)
	movl	24(%esp),%eax
	movl	%eax,24(%ecx)
1:
	xorl	%eax, %eax		# return 0
	ret

/*
 * addupc(int pc, struct uprof *up, int ticks):
 * update profiling information for the user process.
 */

	ALIGN32
ENTRY(addupc)
	pushl %ebp
	movl %esp,%ebp
	movl 12(%ebp),%edx		/* up */
	movl 8(%ebp),%eax		/* pc */

	subl PR_OFF(%edx),%eax		/* pc -= up->pr_off */
	jl L1				/* if (pc < 0) return */

	shrl $1,%eax			/* praddr = pc >> 1 */
	imull PR_SCALE(%edx),%eax	/* praddr *= up->pr_scale */
	shrl $15,%eax			/* praddr = praddr << 15 */
	andl $-2,%eax			/* praddr &= ~1 */

	cmpl PR_SIZE(%edx),%eax		/* if (praddr > up->pr_size) return */
	ja L1

/*	addl %eax,%eax			/* praddr -> word offset */
	addl PR_BASE(%edx),%eax		/* praddr += up-> pr_base */
	movl 16(%ebp),%ecx		/* ticks */

	movl _C_LABEL(curpcb),%edx
	movl $proffault,PCB_ONFAULT(%edx)
	addl %ecx,(%eax)		/* storage location += ticks */
	movl $0,PCB_ONFAULT(%edx)
L1:
	leave
	ret

	ALIGN32
proffault:
	/* if we get a fault, then kill profiling all together */
	movl $0,PCB_ONFAULT(%edx)	/* squish the fault handler */
 	movl 12(%ebp),%ecx
	movl $0,PR_SCALE(%ecx)		/* up->pr_scale = 0 */
	leave
	ret

.data
	ALIGN32
	.globl	_C_LABEL(cyloffset), _C_LABEL(curpcb)
_C_LABEL(cyloffset):	.long	0
	.globl	_C_LABEL(proc0paddr)
_C_LABEL(proc0paddr):	.long	0
LF:	.asciz "Xswitch %x"

.text
 # To be done:
	.globl _C_LABEL(astoff)
_C_LABEL(astoff):
	ret

#define	IDTVEC(name)	.align 4; .globl _X/**/name; _X/**/name:
#define	PANIC(msg)	xorl %eax,%eax; movl %eax,_waittime; pushl 1f; \
			call _C_LABEL(panic); 1: .asciz msg
#define	PRINTF(n,msg)	pushal ; nop ; pushl 1f; call _printf; MSG(msg) ; \
			 popl %eax ; popal
#define	MSG(msg)	.data; 1: .asciz msg; .text

	.text

/*
 * Trap and fault vector routines
 */ 
#define	TRAP(a)		pushl $(a) ; jmp alltraps
#ifdef KGDB
#define	BPTTRAP(a)	pushl $(a) ; jmp bpttraps
#else
#define	BPTTRAP(a)	TRAP(a)
#endif

IDTVEC(div)
	pushl $0; TRAP(T_DIVIDE)
IDTVEC(dbg)
	pushl $0; BPTTRAP(T_TRCTRAP)
IDTVEC(nmi)
	pushl $0; TRAP(T_NMI)
IDTVEC(bpt)
	pushl $0; BPTTRAP(T_BPTFLT)
IDTVEC(ofl)
	pushl $0; TRAP(T_OFLOW)
IDTVEC(bnd)
	pushl $0; TRAP(T_BOUND)
IDTVEC(ill)
	pushl $0; TRAP(T_PRIVINFLT)
IDTVEC(dna)
	pushl $0; TRAP(T_DNA)
IDTVEC(dble)
	TRAP(T_DOUBLEFLT)
	/*PANIC("Double Fault");*/
IDTVEC(fpusegm)
	pushl $0; TRAP(T_FPOPFLT)
IDTVEC(tss)
	TRAP(T_TSSFLT)
	/*PANIC("TSS not valid");*/
IDTVEC(missing)
	TRAP(T_SEGNPFLT)
IDTVEC(stk)
	TRAP(T_STKFLT)
IDTVEC(prot)
	TRAP(T_PROTFLT)
IDTVEC(page)
	TRAP(T_PAGEFLT)
IDTVEC(rsvd)
	pushl $0; TRAP(T_RESERVED)
IDTVEC(fpu)
#ifdef NPX
	/*
	 * Handle like an interrupt so that we can call npxintr to clear the
	 * error.  It would be better to handle npx interrupts as traps but
	 * this is difficult for nested interrupts.
	 */
	pushl	$0		/* dummy error code */
	pushl	$T_ASTFLT
	pushal
	nop			/* silly, the bug is for popal and it only
				 * bites when the next instruction has a
				 * complicated address mode */
	pushl	%ds
	pushl	%es		/* now the stack frame is a trap frame */
	movl	$KDSEL,%eax
/*
 * AI-ONLY NOTE: movw, not movl. %ax and a segment register are both
 * sixteen bits, and gas rejects the l suffix on them. This file
 * already writes `movw %ax,%ds' correctly in six other places --
 * the four here are the ones that slipped -- so this is the file's
 * own spelling rather than another tree's. 4.4BSD-Lite, NetBSD 1.0
 * through 1.4 and OpenBSD 1996 all write movl at these sites and
 * their assemblers took it; NetBSD 1.6 is the first here to write
 * movw. The encoding is unchanged.
 *
 * Two of the four, in the NPX block below, are not assembled in any
 * configuration built so far: the guard tests NPX and the generated
 * npx.h defines NNPX. They are corrected all the same.
 */
	movw	%ax,%ds
	movw	%ax,%es
	pushl	_C_LABEL(cpl)
	pushl	$0		/* dummy unit to finish building intr frame */
	incl	_C_LABEL(cnt)+V_TRAP
	call	_C_LABEL(npxintr)
	jmp	doreti
#else
	pushl $0; TRAP(T_ARITHTRAP)
#endif
	/* 17 - 31 reserved for future exp */
IDTVEC(rsvd0)
	pushl $0; TRAP(17)
IDTVEC(rsvd1)
	pushl $0; TRAP(18)
IDTVEC(rsvd2)
	pushl $0; TRAP(19)
IDTVEC(rsvd3)
	pushl $0; TRAP(20)
IDTVEC(rsvd4)
	pushl $0; TRAP(21)
IDTVEC(rsvd5)
	pushl $0; TRAP(22)
IDTVEC(rsvd6)
	pushl $0; TRAP(23)
IDTVEC(rsvd7)
	pushl $0; TRAP(24)
IDTVEC(rsvd8)
	pushl $0; TRAP(25)
IDTVEC(rsvd9)
	pushl $0; TRAP(26)
IDTVEC(rsvd10)
	pushl $0; TRAP(27)
IDTVEC(rsvd11)
	pushl $0; TRAP(28)
IDTVEC(rsvd12)
	pushl $0; TRAP(29)
IDTVEC(rsvd13)
	pushl $0; TRAP(30)
IDTVEC(rsvd14)
	pushl $0; TRAP(31)

	ALIGN32
alltraps:
	pushal
	nop
	push %ds
	push %es
	# movw	$KDSEL,%ax
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
calltrap:
	incl	_C_LABEL(cnt)+V_TRAP
	call	_C_LABEL(trap)
	/*
	 * Return through doreti to handle ASTs.  Have to change trap frame
	 * to interrupt frame.
	 */
	movl	$T_ASTFLT,4+4+32(%esp)	/* new trap type (err code not used) */
	pushl	_C_LABEL(cpl)
	pushl	$0			/* dummy unit */
	jmp	doreti

#ifdef KGDB
/*
 * This code checks for a kgdb trap, then falls through
 * to the regular trap code.
 */
	ALIGN32
bpttraps:
	pushal
	nop
	push	%es
	push	%ds
	# movw	$KDSEL,%ax
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movzwl	52(%esp),%eax
	test	$3,%eax	
	jne	calltrap
	call	_C_LABEL(kgdb_trap_glue)		
	jmp	calltrap
#endif

/*
 * Call gate entry for syscall
 */

	ALIGN32
IDTVEC(syscall)
	pushfl	# only for stupid carry bit and more stupid wait3 cc kludge
	pushal	# only need eax,ecx,edx - trap resaves others
	nop
	movl	$KDSEL,%eax		# switch to kernel segments
	movw	%ax,%ds
	movw	%ax,%es
	incl	_C_LABEL(cnt)+V_SYSCALL  # kml 3/25/93
	call	_C_LABEL(syscall)
	/*
	 * Return through doreti to handle ASTs.  Have to change syscall frame
	 * to interrupt frame.
	 *
	 * XXX - we should have set up the frame earlier to avoid the
	 * following popal/pushal (not much can be done to avoid shuffling
	 * the flags).  Consistent frames would simplify things all over.
	 */
	movl	32+0(%esp),%eax	/* old flags, shuffle to above cs:eip */
	movl	32+4(%esp),%ebx	/* `int' frame should have been ef, eip, cs */
	movl	32+8(%esp),%ecx
	movl	%ebx,32+0(%esp)
	movl	%ecx,32+4(%esp)
	movl	%eax,32+8(%esp)
	popal
	nop
	pushl	$0		/* dummy error code */
	pushl	$T_ASTFLT
	pushal
	nop
	movl	_C_LABEL(_udatasel),%eax	/* switch back to user segments */
	push	%eax		/* XXX - better to preserve originals? */
	push	%eax
	pushl	_C_LABEL(cpl)
	pushl	$0
	jmp	doreti

	ALIGN32
ENTRY(htonl)
ENTRY(ntohl)
	movl	4(%esp),%eax
	xchgb	%al,%ah
	roll	$16,%eax
	xchgb	%al,%ah
	ret

	ALIGN32
ENTRY(htons)
ENTRY(ntohs)
	movzwl	4(%esp),%eax
	xchgb	%al,%ah
	ret

#include "vector.s"
#include "i386/isa/icu.s"
