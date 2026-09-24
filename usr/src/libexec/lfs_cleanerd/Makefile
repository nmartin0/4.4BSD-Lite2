#	@(#)Makefile	8.1 (Berkeley) 6/5/93

PROG=	lfs_cleanerd
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the kernel
# source of the running system -- on a Linux build host, sysfs.
# FreeBSD 2.0.5 and OpenBSD 1996 write these paths inside the tree;
# NetBSD 1.0 writes ${DESTDIR}/sys. sbin/newlfs and sbin/dumplfs were
# changed the same way and this directory was missed then.
CFLAGS+=-I${.CURDIR}/../../sys/ufs/lfs -I${.CURDIR} -DDIAGNOSTIC
MAN8=	lfs_cleanerd.0
SRCS=	cleanerd.c lfs_cksum.c library.c misc.c print.c

.PATH:	${.CURDIR}/../../sys/ufs/lfs

.include <bsd.prog.mk>
