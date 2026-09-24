#	@(#)Makefile	8.3 (Berkeley) 3/27/94

PROG=	mount_null
SRCS=	mount_null.c getmntopts.c
MAN8=	mount_null.0

MOUNT=	${.CURDIR}/../mount
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile
# that way; NetBSD 1.0 writes ${DESTDIR}/sys, which needs the
# sources inside the target root.
CFLAGS+= -I${.CURDIR}/../../sys -I${MOUNT}
.PATH:	${MOUNT}

.include <bsd.prog.mk>
