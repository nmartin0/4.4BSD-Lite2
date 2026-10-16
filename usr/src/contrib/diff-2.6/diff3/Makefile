#	$Id: Makefile,v 1.6 1994/03/09 19:07:00 jtc Exp $

# AI-ONLY NOTE: as gdiff3, running gdiff, in contrib/'s /usr/contrib/bin;
# without NetBSD's libgnumalloc, which this system has not. The page
# is diff3.1, as gdiff3.0.
PROG=		gdiff3
SRCS=		diff3.c getopt.c getopt1.c version.c
CFLAGS+=	-I$(.CURDIR)/../diff \
		-DHAVE_CONFIG_H -DDIFF_PROGRAM=\"/usr/contrib/bin/gdiff\"

CLEANFILES+=	gdiff3.0

gdiff3.0: diff3.1
	nroff -man ${.CURDIR}/../diff/diff3.1 > ${.TARGET}

.include <bsd.prog.mk>
.PATH: $(.CURDIR)/../diff
