#	$Id: Makefile,v 1.6 1994/03/09 19:07:00 jtc Exp $

# AI-ONLY NOTE: without NetBSD's libgnumalloc, which this system has not.
PROG=		diff3
SRCS=		diff3.c getopt.c getopt1.c version.c
CFLAGS+=	-I$(.CURDIR)/../diff \
		-DHAVE_CONFIG_H -DDIFF_PROGRAM=\"/usr/bin/diff\"

.include <bsd.prog.mk>
.PATH: $(.CURDIR)/../diff
