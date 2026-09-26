#	@(#)Makefile	8.1 (Berkeley) 6/4/93

PROG=	rshd
SRCS=	rshd.c
MAN8=	rshd.0

# AI-ONLY NOTE: Kerberos is optional here, as it is in the programs
# under usr.bin that offer it; see usr.bin/login/Makefile for the
# reasoning and the donors. kerberosIV is not built here yet -- it
# can be, and an earlier version of this note said otherwise -- so
# with Kerberos always on this program did not build at all.
.if defined(KERBEROS)
CFLAGS+=-DKERBEROS -DCRYPT
SRCS+=	des_rw.c
DPADD+=	${LIBKRB} ${LIBDES}
LDADD+=	-lkrb -ldes
.endif
.PATH: ${.CURDIR}/../../usr.bin/rlogin

.include <bsd.prog.mk>
