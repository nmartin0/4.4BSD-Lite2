#	@(#)Makefile	8.1 (Berkeley) 6/5/93

# AI-ONLY NOTE: eqnchar.7, man.7, ms.7 and term.7 are not here: the
# Lite cut removed them, and FreeBSD 2.0, NetBSD 1.0 and OpenBSD drop
# them from this list.
MAN7=	ascii.0 environ.0 hier.0 hostname.0 intro.0 mailaddr.0 \
	mdoc.0 mdoc.samples.0 operator.0
MLINKS=	intro.7 miscellaneous.7

all: ${MAN7}

clean depend lint tags:

cleandir:
	rm -f ${MAN7}

install: maninstall

.include <bsd.prog.mk>
