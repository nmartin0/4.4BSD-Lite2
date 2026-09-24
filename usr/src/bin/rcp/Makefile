#	@(#)Makefile	8.1 (Berkeley) 7/19/93

PROG=	rcp
SRCS=	rcp.c util.c

# AI-ONLY NOTE: Kerberos is optional here, as it is in the programs
# under usr.bin and libexec that offer it; see usr.bin/login/Makefile
# for the reasoning and the donors. That change swept usr.bin and this
# directory was missed, so rcp went on failing at des.h.
.if defined(KERBEROS)
CFLAGS+=-DKERBEROS -DCRYPT
SRCS+=	krcmd.c kcmd.c
DPADD+=	${LIBKRB} ${LIBDES}
LDADD+=	-lkrb -ldes
.endif
BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg
.PATH:	${.CURDIR}/../../usr.bin/rlogin

.include <bsd.prog.mk>
