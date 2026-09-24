#	@(#)Makefile	8.4 (Berkeley) 7/13/94

PROG=	mount_union
SRCS=	mount_union.c getmntopts.c
MAN8=	mount_union.0

MOUNT=	${.CURDIR}/../mount
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile
# that way; NetBSD 1.0 writes ${DESTDIR}/sys, which needs the
# sources inside the target root.
CFLAGS+= -I${.CURDIR}/../../sys -I${MOUNT}
.PATH:	${MOUNT}

.include <bsd.prog.mk>
