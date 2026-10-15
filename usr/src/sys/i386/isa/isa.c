/*-
 * Copyright (c) 1991, 1993
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
 *	@(#)isa.c	8.1 (Berkeley) 6/11/93
 */

/*
 * code to manage AT bus
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/syslog.h>

#include <machine/segments.h>
#include <i386/isa/isa_device.h>
#include <i386/isa/icu.h>
#include <vm/vm.h>

/*
 * Configure all ISA devices
 */
isa_configure() {
	struct isa_device *dvp;
	struct isa_driver *dp;

	splhigh();
	INTREN(IRQ_SLAVE);
	for (dvp = isa_devtab_bio; config_isadev(dvp,&biomask); dvp++);
	for (dvp = isa_devtab_tty; config_isadev(dvp,&ttymask); dvp++);
	for (dvp = isa_devtab_net; config_isadev(dvp,&netmask); dvp++);
	for (dvp = isa_devtab_null; config_isadev(dvp,0); dvp++);
#include "sl.h"
#if NSL > 0
	netmask |= ttymask;
	ttymask |= netmask;
#endif

	/*
	 * The problem is... if netmask == 0, then the loopback
	 * code can do some really ugly things.
	 * workaround for this: if netmask == 0, set it to 0x8000, which
	 * is the value used by splsoftclock.  this is nasty, but it
	 * should work until this interrupt system goes away. -- cgd
	 */
	if (netmask == 0)
		netmask = 0x8000;       /* same as for softclock.  XXX */

	/* biomask |= ttymask ;  can some tty devices use buffers? */
	printf("biomask %x ttymask %x netmask %x\n", biomask, ttymask, netmask);
	splnone();
}

/*
 * Configure an ISA device.
 */
config_isadev(isdp, mp)
	struct isa_device *isdp;
	int *mp;
{
	struct isa_driver *dp;
 
	if (dp = isdp->id_driver) {
		if (isdp->id_maddr) {
			extern int atdevbase;

			isdp->id_maddr -= 0xa0000;
			isdp->id_maddr += atdevbase;
		}
		isdp->id_alive = (*dp->probe)(isdp);
		if (isdp->id_alive) {
			printf("%s%d", dp->name, isdp->id_unit);
			(*dp->attach)(isdp);
			printf(" at 0x%x ", isdp->id_iobase);
			if(isdp->id_irq) {
				int intrno;

				intrno = ffs(isdp->id_irq)-1;
				printf("irq %d ", intrno);
				INTREN(isdp->id_irq);
				if(mp)INTRMASK(*mp,isdp->id_irq);
				setidt(ICU_OFFSET+intrno, isdp->id_intr,
					 SDT_SYS386IGT, SEL_KPL);
			}
			if (isdp->id_drq != -1) printf("drq %d ", isdp->id_drq);
			printf("on isa\n");
		}
		return (1);
	} else	return(0);
}

#define	IDTVEC(name)	__CONCAT(X,name)
/* default interrupt vector table */
extern	IDTVEC(intr0), IDTVEC(intr1), IDTVEC(intr2), IDTVEC(intr3),
	IDTVEC(intr4), IDTVEC(intr5), IDTVEC(intr6), IDTVEC(intr7),
	IDTVEC(intr8), IDTVEC(intr9), IDTVEC(intr10), IDTVEC(intr11),
	IDTVEC(intr12), IDTVEC(intr13), IDTVEC(intr14), IDTVEC(intr15);
	
/*
 * Fill in default interrupt table (in case of spuruious interrupt
 * during configuration of kernel, setup interrupt control unit
 */
isa_defaultirq() {

/* first icu */
	setidt(32, &IDTVEC(intr0),  SDT_SYS386IGT, SEL_KPL);
	setidt(33, &IDTVEC(intr1),  SDT_SYS386IGT, SEL_KPL);
	setidt(34, &IDTVEC(intr2),  SDT_SYS386IGT, SEL_KPL);
	setidt(35, &IDTVEC(intr3),  SDT_SYS386IGT, SEL_KPL);
	setidt(36, &IDTVEC(intr4),  SDT_SYS386IGT, SEL_KPL);
	setidt(37, &IDTVEC(intr5),  SDT_SYS386IGT, SEL_KPL);
	setidt(38, &IDTVEC(intr6),  SDT_SYS386IGT, SEL_KPL);
	setidt(39, &IDTVEC(intr7),  SDT_SYS386IGT, SEL_KPL);

/* second icu */
	setidt(40, &IDTVEC(intr8),  SDT_SYS386IGT, SEL_KPL);
	setidt(41, &IDTVEC(intr9),  SDT_SYS386IGT, SEL_KPL);
	setidt(42, &IDTVEC(intr10),  SDT_SYS386IGT, SEL_KPL);
	setidt(43, &IDTVEC(intr11),  SDT_SYS386IGT, SEL_KPL);
	setidt(44, &IDTVEC(intr12),  SDT_SYS386IGT, SEL_KPL);
	setidt(45, &IDTVEC(intr13),  SDT_SYS386IGT, SEL_KPL);
	setidt(46, &IDTVEC(intr14),  SDT_SYS386IGT, SEL_KPL);
	setidt(47, &IDTVEC(intr15),  SDT_SYS386IGT, SEL_KPL);

	/* initialize 8259's */
	outb(0xf1,0);
	outb(0x20,0x11);
	outb(0x21,32);
	outb(0x21,4);
	outb(0x21,1);
	outb(0x21,0xff);

	outb(0xa0,0x11);
	outb(0xa1,40);
	outb(0xa1,2);
	outb(0xa1,1);
	outb(0xa1,0xff);
}

/*
 * AI-ONLY NOTE: the ISA DMA code below, from isaphysmem to
 * isa_dmarangecheck, is FreeBSD 2.0's i386/isa/isa.c (rev 1.35),
 * unchanged but for pmap_kernel(), written kernel_pmap here, and
 * stands in place of 386BSD's at_dma. NetBSD 1.0's isa/dma.c and
 * OpenBSD's first isadma.c (1995) carry the same functions, line for
 * line; FreeBSD's is the one in this file's own lineage. at_dma
 * programmed channel 2's registers whatever channel it was given,
 * bounced only user addresses, through a buffer from geteblk that
 * could lie anywhere, and handed the controller kernel addresses
 * past its 16 MB as they were: with 32 MB a floppy read came back as
 * zeros. isa_dmastart bounces whatever the controller cannot reach,
 * through isaphysmem, pages pmap_bootstrap reserves for it low in
 * memory as FreeBSD 2.0's does, and isa_dmadone copies a read back.
 * The register names come with them, from FreeBSD 2.0's isa.c and
 * <i386/isa/ic/i8237.h>.
 */
#include <i386/isa/isa.h>
#include <i386/isa/ic/i8237.h>

/*
**  Register definitions for DMA controller 1 (channels 0..3):
*/
#define	DMA1_CHN(c)	(IO_DMA1 + 1*(2*(c)))	/* addr reg for channel c */
#define	DMA1_SMSK	(IO_DMA1 + 1*10)	/* single mask register */
#define	DMA1_MODE	(IO_DMA1 + 1*11)	/* mode register */
#define	DMA1_FFC	(IO_DMA1 + 1*12)	/* clear first/last FF */

/*
**  Register definitions for DMA controller 2 (channels 4..7):
*/
#define	DMA2_CHN(c)	(IO_DMA2 + 2*(2*(c)))	/* addr reg for channel c */
#define	DMA2_SMSK	(IO_DMA2 + 2*10)	/* single mask register */
#define	DMA2_MODE	(IO_DMA2 + 2*11)	/* mode register */
#define	DMA2_FFC	(IO_DMA2 + 2*12)	/* clear first/last FF */

/* region of physical memory known to be contiguous */
vm_offset_t isaphysmem;
static caddr_t dma_bounce[8];		/* XXX */
static char bounced[8];		/* XXX */
#define MAXDMASZ 512		/* XXX */

/* high byte of address is stored in this port for i-th dma channel */
static short dmapageport[8] =
	{ 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };

static int
isa_dmarangecheck(caddr_t va, unsigned length, unsigned chan);

/*
 * isa_dmastart(): program 8237 DMA controller channel, avoid page alignment
 * problems by using a bounce buffer.
 */
void isa_dmastart(int flags, caddr_t addr, unsigned nbytes, unsigned chan)
{	vm_offset_t phys;
	int waport;
	caddr_t newaddr;

	if (    chan > 7
	    || (chan < 4 && nbytes > (1<<16))
	    || (chan >= 4 && (nbytes > (1<<17) || (u_int)addr & 1)))
		panic("isa_dmastart: impossible request"); 

	if (isa_dmarangecheck(addr, nbytes, chan)) {
		if (dma_bounce[chan] == 0)
			dma_bounce[chan] =
				/*(caddr_t)malloc(MAXDMASZ, M_TEMP, M_WAITOK);*/
				(caddr_t) isaphysmem + NBPG*chan;
		bounced[chan] = 1;
		newaddr = dma_bounce[chan];
		*(int *) newaddr = 0;	/* XXX */

		/* copy bounce buffer on write */
		if (!(flags & B_READ))
			bcopy(addr, newaddr, nbytes);
		addr = newaddr;
	}

	/* translate to physical */
	phys = pmap_extract(kernel_pmap, (vm_offset_t)addr);

	if ((chan & 4) == 0) {
		/*
		 * Program one of DMA channels 0..3.  These are
		 * byte mode channels.
		 */
		/* set dma channel mode, and reset address ff */

		/* If B_RAW flag is set, then use autoinitialise mode */
		if (flags & B_RAW) {
		  if (flags & B_READ)
			outb(DMA1_MODE, DMA37MD_AUTO|DMA37MD_WRITE|chan);
		  else
			outb(DMA1_MODE, DMA37MD_AUTO|DMA37MD_READ|chan);
		}
		else
		if (flags & B_READ)
			outb(DMA1_MODE, DMA37MD_SINGLE|DMA37MD_WRITE|chan);
		else
			outb(DMA1_MODE, DMA37MD_SINGLE|DMA37MD_READ|chan);
		outb(DMA1_FFC, 0);

		/* send start address */
		waport =  DMA1_CHN(chan);
		outb(waport, phys);
		outb(waport, phys>>8);
		outb(dmapageport[chan], phys>>16);

		/* send count */
		outb(waport + 1, --nbytes);
		outb(waport + 1, nbytes>>8);

		/* unmask channel */
		outb(DMA1_SMSK, chan);
	} else {
		/*
		 * Program one of DMA channels 4..7.  These are
		 * word mode channels.
		 */
		/* set dma channel mode, and reset address ff */

		/* If B_RAW flag is set, then use autoinitialise mode */
		if (flags & B_RAW) {
		  if (flags & B_READ)
			outb(DMA2_MODE, DMA37MD_AUTO|DMA37MD_WRITE|(chan&3));
		  else
			outb(DMA2_MODE, DMA37MD_AUTO|DMA37MD_READ|(chan&3));
		}
		else
		if (flags & B_READ)
			outb(DMA2_MODE, DMA37MD_SINGLE|DMA37MD_WRITE|(chan&3));
		else
			outb(DMA2_MODE, DMA37MD_SINGLE|DMA37MD_READ|(chan&3));
		outb(DMA2_FFC, 0);

		/* send start address */
		waport = DMA2_CHN(chan - 4);
		outb(waport, phys>>1);
		outb(waport, phys>>9);
		outb(dmapageport[chan], phys>>16);

		/* send count */
		nbytes >>= 1;
		outb(waport + 2, --nbytes);
		outb(waport + 2, nbytes>>8);

		/* unmask channel */
		outb(DMA2_SMSK, chan & 3);
	}
}

void isa_dmadone(int flags, caddr_t addr, int nbytes, int chan)
{

	/* copy bounce buffer on read */
	/*if ((flags & (B_PHYS|B_READ)) == (B_PHYS|B_READ))*/
	if (bounced[chan]) {
		bcopy(dma_bounce[chan], addr, nbytes);
		bounced[chan] = 0;
	}
}

/*
 * Check for problems with the address range of a DMA transfer
 * (non-contiguous physical pages, outside of bus address space,
 * crossing DMA page boundaries).
 * Return true if special handling needed.
 */

static int
isa_dmarangecheck(caddr_t va, unsigned length, unsigned chan) {
	vm_offset_t phys, priorpage = 0, endva;
	u_int dma_pgmsk = (chan & 4) ?  ~(128*1024-1) : ~(64*1024-1);

	endva = (vm_offset_t)round_page(va + length);
	for (; va < (caddr_t) endva ; va += NBPG) {
		phys = trunc_page(pmap_extract(kernel_pmap, (vm_offset_t)va));
#define ISARAM_END	RAM_END
		if (phys == 0)
			panic("isa_dmacheck: no physical page present");
		if (phys >= ISARAM_END)
			return (1);
		if (priorpage) {
			if (priorpage + NBPG != phys)
				return (1);
			/* check if crossing a DMA page boundary */
			if (((u_int)priorpage ^ (u_int)phys) & dma_pgmsk)
				return (1);
		}
		priorpage = phys;
	}
	return (0);
}

/*
 * Handle a NMI, possibly a machine check.
 * return true to panic system, false to ignore.
 */
isa_nmi(cd) {

	log(LOG_CRIT, "\nNMI port 61 %x, port 70 %x\n", inb(0x61), inb(0x70));
	return(0);
}

/*
 * Caught a stray interrupt, notify
 */
isa_strayintr(d) {
	static u_long strays;

	/* for some reason, we get bursts of intr #7, even if not enabled! */
	/*
	 * AI-ONLY NOTE: NetBSD 1.0's intr.c, without its irq -1 case,
	 * which nothing here passes. d is the irq number -- INTR_TAIL in
	 * icu.h pushes `unit', 0 to 15 -- not a mask, so ffs(d)-1 printed
	 * 0 for every odd irq. The likely reason for the bursts is the
	 * 8259 itself: a request masked in the IMR after it was raised,
	 * as an spl does, is acknowledged as that controller's lowest
	 * priority line, 7 on the master or 15 on the slave -- not
	 * confirmed here beyond the number. Measured under QEMU: irq 15,
	 * the slave's, which wd0's 14 is on, seventeen thousand times
	 * while pax extracted from wd0, each one a line on the console.
	 * FreeBSD 2.0 stops after five too, counting each irq apart.
	 */
	if (++strays <= 5)
		log(LOG_ERR, "stray interrupt %d%s\n", d,
		    strays >= 5 ? "; stopped logging" : "");
}
