#	@(#)Makefile	5.3 (Berkeley) 6/5/93

PROG=	scsiformat
MAN8=	scsiformat.0
# AI-ONLY NOTE: ${.CURDIR}/../../sys, not /sys, which names the
# kernel source of the running system -- on a Linux build host,
# sysfs. OpenBSD 1996 and FreeBSD 2.0.5 both write this Makefile
# that way; NetBSD 1.0 writes ${DESTDIR}/sys, which needs the
# sources inside the target root.
CFLAGS+=-I${.CURDIR}/../../sys

.include <bsd.prog.mk>
