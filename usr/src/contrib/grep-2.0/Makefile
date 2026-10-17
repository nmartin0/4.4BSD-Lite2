#	$NetBSD: Makefile,v 1.6 1995/04/23 07:58:41 cgd Exp $

PROG=	grep
SRCS=	dfa.c grep.c getopt.c kwset.c obstack.c regex.c search.c
CFLAGS+=-DGREP -DHAVE_STRING_H=1 -DHAVE_SYS_PARAM_H=1 -DHAVE_UNISTD_H=1 \
	-DHAVE_GETPAGESIZE=1 -DHAVE_MEMCHR=1 -DHAVE_STRERROR=1 \
	-DHAVE_VALLOC=1

# AI-ONLY NOTE: the system's grep, egrep and fgrep, by the maintainer's
# choice, as in NetBSD 1.1, whose this is, and FreeBSD 2.0: in /usr/bin
# with the pages beside the system's, as contrib/ed is. Lite2's own
# usr.bin/grep, James Woods' egrep, hands -v, -b, -w, -x and -y to the
# old AT&T greps in /usr/libexec, which the Lite cut removed. Without
# NetBSD's libgnumalloc, which this system has not.
BINDIR=	/usr/bin
MANDIR=	/usr/share/man/cat

LINKS+= ${BINDIR}/grep ${BINDIR}/egrep \
	${BINDIR}/grep ${BINDIR}/fgrep
MLINKS= grep.1 egrep.1 grep.1 fgrep.1

check:	all
	sh ${.CURDIR}/tests/check.sh ${.CURDIR}/tests

.include <bsd.prog.mk>
