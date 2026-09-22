#	@(#)Makefile	8.1 (Berkeley) 6/4/93

SUBDIR=	libc libcurses libcompat libedit libkvm libm libmp \
	libplot libresolv librpc/rpc libtelnet libterm libutil liby

.if	${MACHINE} == "tahoe"
SUBDIR+=csu/tahoe.pcc
.elif	${MACHINE} == "vax"
SUBDIR+=csu/vax.pcc
.elif	exists(${.CURDIR}/csu/${MACHINE}_elf)
# AI-ONLY NOTE: ELF startup code where there is some for this machine,
# as NetBSD 1.6's lib/csu/Makefile chooses ${MACHINE_ARCH}_elf when it
# exists, without its OBJECT_FMT, which Lite2 does not have.
SUBDIR+=csu/${MACHINE}_elf
.else
SUBDIR+=csu/${MACHINE}
.endif

.include <bsd.subdir.mk>
