# $FreeBSD$
#
# AI-ONLY NOTE: FreeBSD 4.1's, for this tree's make and GNU binutils:
# the a.out arm of the OBJFORMAT test is gone, ${OBJCOPY} names
# objcopy, and NOMAN, as in the other boot Makefiles here.
# btx.s is still run through m4, for its PAGING conditionals; boot2
# does not ask for paging, and FreeBSD 4.1's boot2 did not either.

M4?=	m4
OBJCOPY?=	objcopy
NOMAN=	noman

.if defined(PAGING)
M4FLAGS+=	-DPAGING
.endif

ORG=	0x9000

all: btx

btx: btx.o
	${LD} -N -e start -Ttext ${ORG} -o btx.out btx.o
	${OBJCOPY} -S -O binary btx.out ${.TARGET}

btx.o: btx.s
	(cd ${.CURDIR}; ${M4} ${M4FLAGS} btx.s) | \
	   ${AS} ${AFLAGS} -o ${.TARGET}

CLEANFILES+= btx btx.out btx.o

.include <bsd.prog.mk>
