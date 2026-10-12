# AI-ONLY NOTE: this Makefile is this project's; the sources beside it
# are Kernighan's, version 20121220, from his own repository's first
# commit, unchanged. docs/provenance/imports.md (kind E) has the why.
#
# Its shape is OpenBSD's usr.bin/awk/Makefile: the grammar through
# ${YACC}, then maketab -- a program this build runs on the host, as
# bin/sh's generators are, with ${HOSTCC} -- reading the token numbers
# yacc wrote and printing the dispatch table proctab.c. The names are
# Kernighan's makefile's, ytab.c and ytab.h, because maketab.c opens
# "ytab.h" by that name and the sources include it.

PROG=	awk
SRCS=	ytab.c lex.c b.c main.c parse.c proctab.c tran.c lib.c run.c
CFLAGS+=-I. -I${.CURDIR}
DPADD=	${LIBM}
LDADD=	-lm
CLEANFILES+=ytab.c ytab.h proctab.c maketab

ytab.c ytab.h: awkgram.y
	${YACC} -d ${.CURDIR}/awkgram.y
	mv y.tab.c ytab.c
	mv y.tab.h ytab.h

proctab.c: maketab
	./maketab >proctab.c

maketab: ytab.h ${.CURDIR}/maketab.c
	${HOSTCC} -I. -I${.CURDIR} ${.CURDIR}/maketab.c -o maketab

lex.o b.o main.o parse.o proctab.o tran.o lib.o run.o: ytab.h

.include <bsd.prog.mk>
