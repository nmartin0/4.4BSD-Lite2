#	@(#)Makefile	8.2 (Berkeley) 4/4/94

PROG=	ftpd
# AI-ONLY NOTE: -I${.CURDIR} so that ftpcmd.c, which yacc writes into
# the object directory, finds the "extern.h" that sits in the source
# directory. usr.bin/mklocale needed the same line for the same
# reason. NetBSD 1.0 and OpenBSD 1996 have no such line, their builds
# compiling in the source directory where the question does not
# arise.
CFLAGS+=-DSETPROCTITLE -I${.CURDIR}
SRCS=	ftpd.c ftpcmd.c logwtmp.c popen.c
MAN8=	ftpd.0
CLEANFILES+=ftpcmd.c y.tab.h

.include <bsd.prog.mk>
