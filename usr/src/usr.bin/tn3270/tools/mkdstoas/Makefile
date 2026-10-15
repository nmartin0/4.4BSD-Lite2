#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	mkdstoas
SRCS=	mkdstoas.c asc_ebc.c ebc_disp.c
CFLAGS+=-I${.CURDIR}/.. -I.
NOMAN=	noman
.PATH:	${.CURDIR}/../../api

# AI-ONLY NOTE: run during the build, so made for the machine doing the
# build, with ${HOSTCC}; NetBSD 1.6 builds these tools as HOSTPROGs.
CC=	${HOSTCC}

.include <bsd.prog.mk>
