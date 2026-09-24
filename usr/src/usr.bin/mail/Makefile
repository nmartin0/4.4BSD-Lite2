#	@(#)Makefile	8.3 (Berkeley) 4/20/95

PROG=	mail
# AI-ONLY NOTE: no -R. It told the older compilers to put initialized
# data into read-only text; no version of GCC has ever had it, and
# GCC 14 stops with "unrecognized command-line option '-R'".
# FreeBSD 2.0.5 drops it from this line and keeps the rest of it;
# OpenBSD 1996 drops the line altogether. 4.4BSD-Lite and NetBSD 1.0
# still have it.
SRCS=	version.c aux.c cmd1.c cmd2.c cmd3.c cmdtab.c collect.c edit.c fio.c \
	getname.c head.c v7.local.c lex.c list.c main.c names.c popen.c \
	quit.c send.c strings.c temp.c tty.c vars.c
SFILES=	mail.help mail.tildehelp
EFILES=	mail.rc
LINKS=	${BINDIR}/mail ${BINDIR}/Mail
MLINKS=	mail.1 Mail.1

beforeinstall:
	cd ${.CURDIR}/misc; install -c -o ${BINOWN} -g ${BINGRP} \
	    -m 444 ${SFILES} ${DESTDIR}/usr/share/misc
	cd ${.CURDIR}/misc; install -c -o root -g wheel \
	    -m 644 ${EFILES} ${DESTDIR}/etc

.include <bsd.prog.mk>
