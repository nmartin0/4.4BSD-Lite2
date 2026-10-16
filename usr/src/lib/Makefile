#	@(#)Makefile	8.1 (Berkeley) 6/4/93

# AI-ONLY NOTE: not named: libmp and libplot, libraries the Lite cut
# deleted with their directories. A name with no directory stops the
# build here, as it stopped Lite2's own make, which ran each command
# with `sh -ec' (docs/deferred.md, *Sources and directories named but
# absent*).
SUBDIR=	libc libcurses libcompat libedit libkvm libm \
	libresolv librpc/rpc libtelnet libterm libutil liby

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
