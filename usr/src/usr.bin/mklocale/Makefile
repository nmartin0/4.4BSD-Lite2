#	@(#)Makefile	8.1 (Berkeley) 6/7/93

PROG=	mklocale
SRCS=	yacc.c lex.c

# AI-ONLY NOTE: yacc.c is written into the object directory and does
# `#include "ldef.h"', which is in the source directory, so the quoted
# include finds nothing. ${.CURDIR} is how this tree spells that path
# elsewhere -- usr.bin/passwd and sbin/mount_null among others.
# FreeBSD 2.0.5 writes -I. here, which is the object directory under
# this build and does not hold the header; NetBSD 1.0 and OpenBSD 1996
# have no CFLAGS line at all, their builds compiling in the source
# directory where the question does not arise.
CFLAGS+=-I${.CURDIR}
CFLAGS+=-I.
CLEANFILES+=y.tab.h yacc.c lex.c

.include <bsd.prog.mk>
