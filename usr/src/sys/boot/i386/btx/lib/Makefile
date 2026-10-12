# $FreeBSD$
#
# AI-ONLY NOTE: FreeBSD 4.1 built crt0.o through bsd.lib.mk as an
# INTERNALLIB; this tree's bsd.lib.mk would make a libbtx.a and install
# it, so the one target is written out instead. `ld -r' is what that
# ld spelled -i, and -elf goes: this as and ld make nothing else.

OBJS=		btxcsu.o btxsys.o btxv86.o
CLEANFILES+=	crt0.o
NOMAN=		noman

all: crt0.o

crt0.o: ${OBJS}
	${LD} -r -o ${.TARGET} ${OBJS}

.include <bsd.prog.mk>

.s.o:
	${AS} ${AFLAGS} -o ${.TARGET} ${.IMPSRC}
