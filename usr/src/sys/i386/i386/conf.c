/*-
 * Copyright (c) 1991, 1993
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
 *	@(#)conf.c	8.3 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>

int	rawread		__P((dev_t, struct uio *, int));
int	rawwrite	__P((dev_t, struct uio *, int));
int	swstrategy	__P((struct buf *));
int	ttselect	__P((dev_t, int, struct proc *));

#define	dev_type_open(n)	int n __P((dev_t, int, int, struct proc *))
#define	dev_type_close(n)	int n __P((dev_t, int, int, struct proc *))
#define	dev_type_strategy(n)	int n __P((struct buf *))
#define	dev_type_ioctl(n) \
	int n __P((dev_t, int, caddr_t, int, struct proc *))

/* bdevsw-specific types */
#define	dev_type_dump(n)	int n __P((dev_t))
#define	dev_type_size(n)	int n __P((dev_t))

#define	dev_decl(n,t)	__CONCAT(dev_type_,t)(__CONCAT(n,t))
#define	dev_init(c,n,t) \
	(c > 0 ? __CONCAT(n,t) : (__CONCAT(dev_type_,t)((*))) enxio)

/* bdevsw-specific initializations */
#define	dev_size_init(c,n)	(c > 0 ? __CONCAT(n,size) : 0)

#define	bdev_decl(n) \
	dev_decl(n,open); dev_decl(n,close); dev_decl(n,strategy); \
	dev_decl(n,ioctl); dev_decl(n,dump); dev_decl(n,size)

#define	bdev_disk_init(c,n) { \
	dev_init(c,n,open), (dev_type_close((*))) nullop, \
	dev_init(c,n,strategy), dev_init(c,n,ioctl), \
	dev_init(c,n,dump), dev_size_init(c,n), 0 }

#define	bdev_tape_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), \
	dev_init(c,n,strategy), dev_init(c,n,ioctl), \
	dev_init(c,n,dump), 0, B_TAPE }

#define	bdev_swap_init() { \
	(dev_type_open((*))) enodev, (dev_type_close((*))) enodev, \
	swstrategy, (dev_type_ioctl((*))) enodev, \
	(dev_type_dump((*))) enodev, 0, 0 }

#define	bdev_notdef()	bdev_tape_init(0,no)
bdev_decl(no);	/* dummy declarations */

#include "wd.h"
#include "fd.h"
#include "wt.h"
#include "xd.h"

bdev_decl(wd);
bdev_decl(Fd);
bdev_decl(wt);
bdev_decl(xd);

struct bdevsw	bdevsw[] =
{
	bdev_disk_init(NWD,wd),	/* 0: st506/rll/esdi/ide disk */
	bdev_swap_init(),	/* 1: swap pseudo-device */
	bdev_disk_init(NFD,Fd),	/* 2: floppy disk */
	bdev_tape_init(NWT,wt),	/* 3: QIC-24/60/120/150 cartridge tape */
	bdev_disk_init(NXD,xd),	/* 4: temp alt st506/rll/esdi/ide disk */
};

int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

/* cdevsw-specific types */
#define	dev_type_read(n)	int n __P((dev_t, struct uio *, int))
#define	dev_type_write(n)	int n __P((dev_t, struct uio *, int))
#define	dev_type_stop(n)	int n __P((struct tty *, int))
#define	dev_type_reset(n)	int n __P((int))
#define	dev_type_select(n)	int n __P((dev_t, int, struct proc *))
#define	dev_type_map(n)	int n __P(())

#define	cdev_decl(n) \
	dev_decl(n,open); dev_decl(n,close); dev_decl(n,read); \
	dev_decl(n,write); dev_decl(n,ioctl); dev_decl(n,stop); \
	dev_decl(n,reset); dev_decl(n,select); dev_decl(n,map); \
	dev_decl(n,strategy); extern struct tty __CONCAT(n,_tty)[]

#define	dev_tty_init(c,n)	(c > 0 ? __CONCAT(n,_tty) : 0)

/* open, read, write, ioctl, strategy */
#define	cdev_disk_init(c,n) { \
	dev_init(c,n,open), (dev_type_close((*))) nullop, dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) enodev, \
	(dev_type_reset((*))) nullop, 0, seltrue, (dev_type_map((*))) enodev, \
	dev_init(c,n,strategy) }

/* open, close, read, write, ioctl, strategy */
#define	cdev_tape_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) enodev, \
	(dev_type_reset((*))) nullop, 0, seltrue, (dev_type_map((*))) enodev, \
	dev_init(c,n,strategy) }

/* open, close, read, write, ioctl, stop, tty */
#define	cdev_tty_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), dev_init(c,n,stop), \
	(dev_type_reset((*))) nullop, dev_tty_init(c,n), ttselect, \
	(dev_type_map((*))) enodev, 0 }

#define	cdev_notdef() { \
	(dev_type_open((*))) enodev, (dev_type_close((*))) enodev, \
	(dev_type_read((*))) enodev, (dev_type_write((*))) enodev, \
	(dev_type_ioctl((*))) enodev, (dev_type_stop((*))) enodev, \
	(dev_type_reset((*))) nullop, 0, seltrue, \
	(dev_type_map((*))) enodev, 0 }

cdev_decl(no);			/* dummy declarations */

cdev_decl(cn);
/* open, close, read, write, ioctl, select -- XXX should be a tty */
#define	cdev_cn_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) nullop, \
	(dev_type_reset((*))) nullop, 0, dev_init(c,n,select), \
	(dev_type_map((*))) enodev, 0 }

cdev_decl(ctty);
/* open, read, write, ioctl, select -- XXX should be a tty */
#define	cdev_ctty_init(c,n) { \
	dev_init(c,n,open), (dev_type_close((*))) nullop, dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) nullop, \
	(dev_type_reset((*))) nullop, 0, dev_init(c,n,select), \
	(dev_type_map((*))) enodev, 0 }

dev_type_read(mmrw);
/* read/write */
#define	cdev_mm_init(c,n) { \
	(dev_type_open((*))) nullop, (dev_type_close((*))) nullop, mmrw, \
	mmrw, (dev_type_ioctl((*))) enodev, (dev_type_stop((*))) nullop, \
	(dev_type_reset((*))) nullop, 0, seltrue, (dev_type_map((*))) enodev, 0 }

/* read, write, strategy */
#define	cdev_swap_init(c,n) { \
	(dev_type_open((*))) nullop, (dev_type_close((*))) nullop, rawread, \
	rawwrite, (dev_type_ioctl((*))) enodev, (dev_type_stop((*))) enodev, \
	(dev_type_reset((*))) nullop, 0, (dev_type_select((*))) enodev, \
	(dev_type_map((*))) enodev, dev_init(c,n,strategy) }

#include "pty.h"
#define	pts_tty		pt_tty
#define	ptsioctl	ptyioctl
cdev_decl(pts);
#define	ptc_tty		pt_tty
#define	ptcioctl	ptyioctl
cdev_decl(ptc);

/* open, close, read, write, ioctl, tty, select */
#define	cdev_ptc_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) nullop, \
	(dev_type_reset((*))) nullop, dev_tty_init(c,n), dev_init(c,n,select), \
	(dev_type_map((*))) enodev, 0 }

cdev_decl(log);
/* open, close, read, ioctl, select -- XXX should be a generic device */
#define	cdev_log_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	(dev_type_write((*))) enodev, dev_init(c,n,ioctl), \
	(dev_type_stop((*))) enodev, (dev_type_reset((*))) nullop, 0, \
	dev_init(c,n,select), (dev_type_map((*))) enodev, 0 }

cdev_decl(wd);
cdev_decl(Fd);
cdev_decl(xd);
cdev_decl(wt);

#include "com.h"

cdev_decl(com);

#include "pc.h"

cdev_decl(pc);

/* open, close, read, write, ioctl, tty -- XXX should be a tty */
extern struct tty pccons;
#define	cdev_pc_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) nullop, \
	(dev_type_reset((*))) nullop, &pccons, ttselect, \
	(dev_type_map((*))) enodev, 0 }

#include "bpfilter.h"
cdev_decl(bpf);

/* open, close, read, write, ioctl, select -- XXX should be generic device */
#define	cdev_bpf_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), dev_init(c,n,read), \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) enodev, \
	(dev_type_reset((*))) enodev, 0, dev_init(c,n,select), \
	(dev_type_map((*))) enodev, 0 }

struct cdevsw	cdevsw[] =
{
	cdev_cn_init(1,cn),		/* 0: virtual console */
	cdev_ctty_init(1,ctty),		/* 1: controlling terminal */
	cdev_mm_init(1,mm),		/* 2: /dev/{null,mem,kmem,...} */
	cdev_disk_init(NWD,wd),		/* 3: st506/rll/esdi/ide disk */
	cdev_swap_init(1,sw),		/* 4: /dev/drum (swap pseudo-device) */
	cdev_tty_init(NPTY,pts),	/* 5: pseudo-tty slave */
	cdev_ptc_init(NPTY,ptc),	/* 6: pseudo-tty master */
	cdev_log_init(1,log),		/* 7: /dev/klog */
	cdev_tty_init(NCOM,com),	/* 8: serial communications ports */
	cdev_disk_init(NFD,Fd),		/* 9: floppy disk */
	cdev_tape_init(NWT,wt),		/* 10: QIC-24/60/120/150 cartridge tape */
	cdev_disk_init(NXD,xd),		/* 11: temp alt st506/rll/esdi/ide disk */
	cdev_pc_init(1,pc),		/* 12: real console */
	cdev_notdef(),			/* 13 */
	cdev_bpf_init(NBPFILTER,bpf),	/* 14: berkeley packet filter */
};

int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

int	mem_no = 2; 	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(1, 0);

/*
 * Routine that identifies /dev/mem and /dev/kmem.
 *
 * A minimal stub routine can always return 0.
 */
iskmemdev(dev)
	dev_t dev;
{

	return (major(dev) == 2 && (minor(dev) == 0 || minor(dev) == 1));
}

iszerodev(dev)
	dev_t dev;
{
	return (major(dev) == 2 && minor(dev) == 12);
}

/*
 * AI-ONLY NOTE: chrtoblktbl and chrtoblk, which this port did not
 * have while miscfs/specfs/spec_vnops.c:170 and
 * miscfs/kernfs/kernfs_vfsops.c:81 both call chrtoblk. Four of this
 * release's ports define both in their own conf.c -- hp300 at 367
 * and 397, and luna68k, news3400 and pmax likewise -- and i386 is
 * the one that does not, which is why the kernel link wanted the
 * symbol.
 *
 * The function below is hp300/hp300/conf.c's, unchanged. The table
 * is not copied from anywhere: it is this port's own device numbers,
 * read off the two switch tables above. Each entry is the block
 * major that names the same hardware as that character major, and
 * NODEV where there is none:
 *
 *	char  3 wd   -> block 0 wd    st506/rll/esdi/ide disk
 *	char  4 sw   -> block 1 swap  swap pseudo-device
 *	char  9 Fd   -> block 2 Fd    floppy disk
 *	char 10 wt   -> block 3 wt    QIC cartridge tape
 *	char 11 xd   -> block 4 xd    temp alt st506 disk
 *
 * Every other character major -- cn, ctty, mm, pts, ptc, log, com,
 * pc, bpf and the hole at 13 -- has no block device at all.
 *
 * MAXDEV is 15, the length of cdevsw, which nchrdev below computes
 * the same way. The bound matters because chrtoblk indexes
 * chrtoblktbl[major(dev)] with a character major, so the table must
 * cover every major nchrdev admits and the bound must not let an
 * index past the end through.
 *
 * The number is this port's own, not taken from anywhere, and the
 * four ports that have one disagree: news3400 writes 43 against a
 * cdevsw of 43 and pmax 19 against 19, both equal to their nchrdev,
 * while hp300 writes 21 against 23 and luna68k 21 against 23.
 * Those last two are wrong -- character majors 21 and 22 return
 * NODEV there whether or not they have a block device -- and the two
 * that agree are what fixes the invariant as MAXDEV == nchrdev.
 * Recorded rather than fixed: neither port has ever been compiled
 * here, and correcting another port's table is not this change.
 */
#define MAXDEV	15
static int chrtoblktbl[MAXDEV] = {
      /* VCHR */      /* VBLK */
	/* 0 */		NODEV,
	/* 1 */		NODEV,
	/* 2 */		NODEV,
	/* 3 */		0,
	/* 4 */		1,
	/* 5 */		NODEV,
	/* 6 */		NODEV,
	/* 7 */		NODEV,
	/* 8 */		NODEV,
	/* 9 */		2,
	/* 10 */	3,
	/* 11 */	4,
	/* 12 */	NODEV,
	/* 13 */	NODEV,
	/* 14 */	NODEV,
};

/*
 * Convert a character device number to a block device number.
 */
chrtoblk(dev)
	dev_t dev;
{
	int blkmaj;

	if (major(dev) >= MAXDEV || (blkmaj = chrtoblktbl[major(dev)]) == NODEV)
		return (NODEV);
	return (makedev(blkmaj, minor(dev)));
}
