#	@(#)Makefile	8.1 (Berkeley) 7/19/93

PROG=	rsh
SRCS=	rsh.c

# AI-ONLY NOTE: Kerberos is optional, as in NetBSD 1.0's Makefile
# for this program. See usr.bin/login/Makefile.
.if defined(KERBEROS)
CFLAGS+=-DKERBEROS -DCRYPT
SRCS+=	krcmd.c kcmd.c des_rw.c
DPADD+=	${LIBKRB} ${LIBDES}
LDADD+=	-lkrb -ldes
.endif
BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg
.PATH:	${.CURDIR}/../rlogin

.include <bsd.prog.mk>
