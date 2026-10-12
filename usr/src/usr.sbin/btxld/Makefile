# $FreeBSD$
#
# AI-ONLY NOTE: elfh.c is not imported (btxld.c says why). btxld is
# built for the host by build/sysroot.sh, not by this Makefile, and
# usr.sbin/Makefile does not name it.

PROG=	btxld
SRCS=	btxld.c
MAN8=	btxld.8

CFLAGS+=	-Wall

.include <bsd.prog.mk>
