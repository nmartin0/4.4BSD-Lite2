# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# i386 ELF startup code, imported from NetBSD 1.6 (see
# docs/provenance/csu-elf.md), built the way Lite2's own
# lib/csu/i386/Makefile builds its a.out crt0.o and gcrt0.o.
#
# Each flag answers a measured failure:
#
#   -fno-toplevel-reorder  dot_init.h switches sections with top-level
#	__asm. Without this, GCC 14 moves _init's own code into .init
#	where init_fallthru also starts, so _init calls itself and the
#	program dies at startup (measured: segmentation fault), and it
#	discards crtend.c's unreferenced list terminators.
#   -I- -I${.CURDIR}  NetBSD's own include order: dot_init.h must be
#	this directory's, not the empty one in ../common_elf that a
#	plain -I would find first for crtbegin.c and crtend.c. GCC calls
#	-I- obsolete but still honours it.
#   no -DLIBC_SCCS  Lite2's a.out Makefile sets it; here it would make
#	crt0.c use NetBSD's __RCSID, which Lite2's <sys/cdefs.h> lacks.

CFLAGS=	-O -fno-toplevel-reorder
CFLAGS+=-I- -I${.CURDIR} -I${.CURDIR}/../common_elf
OBJS=	crt0.o gcrt0.o crtbegin.o crtend.o
CLEANFILES+=	core a.out

.PATH:	${.CURDIR}/../common_elf

all: ${OBJS}

crt0.o: crt0.c
	${CC} ${CFLAGS} -c -DCRT0 ${.ALLSRC}
	${LD} -x -r ${.TARGET}
	mv a.out ${.TARGET}

gcrt0.o: crt0.c
	${CC} ${CFLAGS} -c -DMCRT0 ${.ALLSRC} -o ${.TARGET}
	${LD} -x -r ${.TARGET}
	mv a.out ${.TARGET}

crtbegin.o: crtbegin.c
	${CC} ${CFLAGS} -c ${.ALLSRC}
	${LD} -x -r ${.TARGET}
	mv a.out ${.TARGET}

crtend.o: crtend.c
	${CC} ${CFLAGS} -c ${.ALLSRC}
	${LD} -x -r ${.TARGET}
	mv a.out ${.TARGET}

install:
	install -o ${BINOWN} -g ${BINGRP} -m 444 ${OBJS} \
	    ${DESTDIR}/usr/lib

depend lint tags:

.include <bsd.prog.mk>
