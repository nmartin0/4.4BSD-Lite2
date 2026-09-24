#	@(#)Makefile	8.4 (Berkeley) 5/5/95

PROG=	sh
SRCS=	alias.c builtins.c cd.c echo.c error.c eval.c exec.c expand.c \
	histedit.c input.c jobs.c mail.c main.c memalloc.c miscbltin.c \
	mystring.c nodes.c options.c parser.c redir.c show.c syntax.c \
	trap.c output.c var.c
OBJS+=	init.o arith.o arith_lex.o
LDADD+=	-ll -ledit -ltermcap
LFLAGS= -8	# 8-bit lex scanner for arithmetic
CFLAGS+=-DSHELL -I. -I${.CURDIR}
.PATH:	${.CURDIR}/bltin ${.CURDIR}/../../usr.bin/printf
CLEANFILES+=\
	builtins.c builtins.h init.c mkinit mknodes mksyntax \
	nodes.c nodes.h printf.o syntax.c syntax.h token.def y.tab.h

.depend parser.o: token.def
token.def: mktokens
	sh ${.CURDIR}/mktokens

builtins.h builtins.c: ${.CURDIR}/mkbuiltins ${.CURDIR}/builtins.def
	cd ${.CURDIR}; sh mkbuiltins ${.OBJDIR}

init.c: mkinit ${SRCS}
	./mkinit '${CC} -c ${CFLAGS} init.c' ${.ALLSRC:S/^mkinit$//}

# AI-ONLY NOTE: these three are compiled with ${HOSTCC} and run during
# the build to generate source, so they must be built for the machine
# doing the building. OpenBSD's bin/sh/Makefile has used ${HOSTCC}
# here since 1996. Built with ${CC} for a target that is not this
# machine, mknodes dies with SIGSEGV as soon as make runs it.
mkinit: ${.CURDIR}/mkinit.c
	${HOSTCC} ${CFLAGS} ${.CURDIR}/mkinit.c -o $@

nodes.c nodes.h: mknodes ${.CURDIR}/nodetypes ${.CURDIR}/nodes.c.pat
	./mknodes ${.CURDIR}/nodetypes ${.CURDIR}/nodes.c.pat

mknodes: ${.CURDIR}/mknodes.c
	${HOSTCC} ${CFLAGS} ${.CURDIR}/mknodes.c -o $@

syntax.c syntax.h: mksyntax
	./mksyntax

mksyntax: ${.CURDIR}/mksyntax.c ${.CURDIR}/parser.h
	${HOSTCC} ${CFLAGS} ${.CURDIR}/mksyntax.c -o $@

.include <bsd.prog.mk>
