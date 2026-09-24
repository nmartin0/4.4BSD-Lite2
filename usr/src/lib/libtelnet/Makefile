#	@(#)Makefile	8.2 (Berkeley) 12/15/93

LIB=	telnet
SRCS=	auth.c encrypt.c genget.c getent.c misc.c
#SRCS+=	kerberos5.c
CFLAGS+= -DAUTHENTICATION -DHAS_CGETENT

# AI-ONLY NOTE: the Kerberos half of this library is optional now, as
# the Kerberos half of every program that uses it is; see
# usr.bin/login/Makefile. Its -I named /usr/include/kerberosIV, the
# running system's headers, which a build into a separate root must
# not read; under KERBEROS it reads the target root's instead.
.if defined(KERBEROS)
SRCS+=	kerberos.c enc_des.c
CFLAGS+= -DENCRYPTION -DKRB4 -DDES_ENCRYPTION
CFLAGS+= -I${DESTDIR}/usr/include/kerberosIV
.endif

# These are the sources that have encryption stuff in them.
CRYPT_SRC= auth.c enc-proto.h enc_des.c encrypt.c
CRYPT_SRC+= encrypt.h kerberos.c kerberos5.c krb4encpwd.c
CRYPT_SRC+= misc.c spx.c Makefile
NOCRYPT_DIR=${.CURDIR}/Nocrypt

.include <bsd.lib.mk>

nocrypt:
#ifdef	ENCRYPTION
	@for i in ${CRYPT_SRC}; do \
	    if [ ! -d ${NOCRYPT_DIR} ]; then \
		echo Creating subdirectory ${NOCRYPT_DIR}; \
		mkdir ${NOCRYPT_DIR}; \
	    fi; \
	    echo ${NOCRYPT_DIR}/$$i; \
	    unifdef -UENCRYPTION ${.CURDIR}/$$i | \
		sed "s/ || defined(ENCRYPTION)//" > ${NOCRYPT_DIR}/$$i; \
	done

placeholder:
#else	/* ENCRYPTION */
	@echo "Encryption code already removed."
#endif	/* ENCRYPTION */
