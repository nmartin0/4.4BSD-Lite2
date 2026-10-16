#	@(#)Makefile	8.1 (Berkeley) 6/11/93

PROG=	mkindex
CFLAGS+=-I${.CURDIR}/../boggle
NOMAN=noman
# AI-ONLY NOTE: built with ${HOSTCC}, for the machine doing the
# build, which runs it to make boggle's dictionary; as adventure's
# setup is here, and as NetBSD 1.6 builds this one, a HOSTPROG.
CC=	${HOSTCC}

install:

.include <bsd.prog.mk>
