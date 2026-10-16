#	$Id: Makefile,v 1.6 1994/03/09 19:06:47 jtc Exp $

# AI-ONLY NOTE: as gdiff, in contrib/'s /usr/contrib/bin; without
# NetBSD's libgnumalloc, which this system has not -- libc's malloc
# serves, as it does GNU diff on most systems. The page is diff.1, as
# gdiff.0.
PROG=	gdiff
SRCS=	diff.c analyze.c cmpbuf.o io.c context.c ed.c normal.c util.c dir.c \
	regex.c side.c ifdef.c version.c getopt.c getopt1.c

CFLAGS+=-DHAVE_CONFIG_H -I${.CURDIR} 
CLEANFILES+=gdiff.0

gdiff.0: diff.1
	nroff -man ${.CURDIR}/diff.1 > ${.TARGET}

.include <bsd.prog.mk>
