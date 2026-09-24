#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	kdump
CFLAGS+=-I${.CURDIR}/../ktrace
SRCS=	kdump.c ioctl.c subr.c
.PATH:	${.CURDIR}/../ktrace
CLEANFILES+=ioctl.c

# AI-ONLY NOTE: DESTDIR is passed to the script, which reads the two
# ioctl headers to build its table. It named /usr/include directly,
# the running system's, which a build into a separate root must not
# use. FreeBSD 2.0.5 writes $DESTDIR/usr/include/... in the same
# script; NetBSD 1.0 and OpenBSD 1996 rewrote it to take the paths as
# arguments, which changes both files rather than one line.
ioctl.c: mkioctls
	DESTDIR=${DESTDIR} /bin/sh ${.CURDIR}/mkioctls > ioctl.c

.include <bsd.prog.mk>
