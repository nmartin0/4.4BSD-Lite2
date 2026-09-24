#	@(#)Makefile	8.3 (Berkeley) 5/29/94

PROG=	config
SRCS=	files.c gram.y hash.c main.c mkheaders.c mkioconf.c mkmakefile.c \
	mkswap.c pack.c scan.l sem.c util.c
CFLAGS+=-I${.CURDIR} -I.
CLEANFILES=gram.c scan.c y.tab.h
MAN8=	config.0

# AI-ONLY NOTE (revision): -ll for the yywrap() that lex's output
# declares; scan.l defined it as a macro and no longer does. This is
# what usr.sbin/config, this program's predecessor, has done all
# along. The explanation went into scan.l and not here, so this note
# is later than the change.
DPADD=	${LIBL}
LDADD=	-ll

.include <bsd.prog.mk>

.depend: gram.c scan.c
