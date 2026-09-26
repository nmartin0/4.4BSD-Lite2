#	@(#)Makefile	8.1 (Berkeley) 7/19/93

PROG=	login
SRCS=	login.c
DPADD=	${LIBUTIL}
LDADD=	-lutil

# AI-ONLY NOTE: Kerberos is optional here. 4.4BSD-Lite2 turns it on
# unconditionally -- there is no conditional anywhere in Berkeley's
# tree -- and ships kerberosIV to support it, so this is a
# reduction against what Berkeley intended, not a tidy-up.
#
# Every other BSD took this step: NetBSD 1.0 wraps exactly these
# lines in `.if defined(KERBEROS)', OpenBSD 1996 does the same from
# bsd.own.mk, FreeBSD 2.0.5 tests for the library as well, and all
# three later deleted Kerberos IV outright. It was superseded by
# Kerberos 5 in 1993, two years before this release.
#
# kerberosIV is not unbuildable, and an earlier version of this
# note said it was, wrongly. Four of its seven DES table
# generators compile and run with the host compiler as they
# stand; what is left is a byte-order define, one undeclared
# variable, two link failures, HOSTCC for the generators as
# bin/sh has, and the literal `make' in the parent Makefile. See
# docs/provenance/imports.md. Define KERBEROS to get these lines
# back once it is built.
.if defined(KERBEROS)
CFLAGS+=-DKERBEROS
SRCS+=	klogin.c
DPADD+=	${LIBKRB} ${LIBDES}
LDADD+=	-lkrb -ldes
.endif
BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg

.include <bsd.prog.mk>
