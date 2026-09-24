#	@(#)Makefile	8.1 (Berkeley) 7/19/93

PROG=	login
SRCS=	login.c
DPADD=	${LIBUTIL}
LDADD=	-lutil

# AI-ONLY NOTE: Kerberos is optional here, as it is in every BSD of
# the period. NetBSD 1.0 wraps exactly these lines in
# `.if defined(KERBEROS)'; OpenBSD 1996 does the same, driven from
# bsd.own.mk; FreeBSD 2.0.5 tests for the library as well. This
# tree turns it on unconditionally and cannot build it: kerberosIV
# generates its DES tables by compiling seven programs and running
# them, which a build for another machine cannot do. Define
# KERBEROS to get these back.
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
