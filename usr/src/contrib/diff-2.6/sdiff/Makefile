#	$Id: Makefile,v 1.5 1994/03/09 19:07:03 jtc Exp $

# AI-ONLY NOTE: as gsdiff, running gdiff, in contrib/'s /usr/contrib/bin;
# without NetBSD's libgnumalloc, which this system has not. The page
# is sdiff.1, as gsdiff.0.
PROG=		gsdiff
SRCS=		sdiff.c getopt.c getopt1.c version.c
CFLAGS+=	-I$(.CURDIR)/../diff \
		-DHAVE_CONFIG_H -DDIFF_PROGRAM=\"/usr/contrib/bin/gdiff\"

CLEANFILES+=	gsdiff.0

gsdiff.0: sdiff.1
	nroff -man ${.CURDIR}/../diff/sdiff.1 > ${.TARGET}

.include <bsd.prog.mk>
.PATH: $(.CURDIR)/../diff
