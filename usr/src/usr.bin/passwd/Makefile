#	@(#)Makefile	8.3 (Berkeley) 4/2/94

PROG=	passwd
SRCS=	local_passwd.c passwd.c pw_copy.c pw_util.c
.PATH:  ${.CURDIR}/../../usr.bin/chpass ${.CURDIR}/../../usr.sbin/vipw \
	${.CURDIR}/../rlogin
CFLAGS+=-I${.CURDIR} -I${.CURDIR}/../../usr.sbin/vipw \
	-I${.CURDIR}/../../usr.bin/chpass

# AI-ONLY NOTE: Kerberos is optional, as in NetBSD 1.0's Makefile
# for this program. See usr.bin/login/Makefile.
.if defined(KERBEROS)
CFLAGS+=-DKERBEROS -DCRYPT
SRCS+=	des_rw.c krb_passwd.c
DPADD+=	${LIBKRB} ${LIBDES}
LDADD+=	-lkrb -ldes
.endif
BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg

.include <bsd.prog.mk>
