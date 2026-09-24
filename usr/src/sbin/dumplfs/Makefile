#	@(#)Makefile	8.1 (Berkeley) 6/18/93

PROG=	dumplfs
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile
# that way; NetBSD 1.0 writes ${DESTDIR}/sys, which needs the
# sources inside the target root.
CFLAGS+=-I${.CURDIR}/../../sys/ufs/lfs
SRCS=	dumplfs.c lfs_cksum.c misc.c
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. FreeBSD 2.0.5 and OpenBSD 1996 both write this .PATH that
# way; NetBSD 1.0 writes ${DESTDIR}/sys.
.PATH:	${.CURDIR}/../../sys/ufs/lfs
MAN8=	dumplfs.0

.include <bsd.prog.mk>
