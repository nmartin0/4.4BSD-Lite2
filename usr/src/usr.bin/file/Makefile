# AI-ONLY NOTE: written (2026-10-24) -- file(1), Ian Darwin's, built
# from the copy 4.4BSD-Lite2 ships in contrib/file-3.12, in the shape of
# NetBSD 1.0's usr.bin/file/Makefile: the program, /etc/magic made by
# concatenating the Magdir pieces, Header and Localstuff first, and
# installed 444 as every BSD installed it until 1996. Berkeley's own
# file was AT&T's and the Lite cut took it, leaving the page it had,
# Berkeley's file.1; Darwin's file.1 and magic.5 are made here from
# contrib's file.man and magic.man, by the sed rule of Darwin's own
# Makefile, as each BSD's usr.bin/file carries Darwin's pages. Darwin's
# LEGAL.NOTICE asks that altered versions be plainly marked; each change
# to contrib/file-3.12 is, at its site.

MAGIC=		/etc/magic
MAGICOWN=	bin
MAGICGRP=	bin
MAGICMODE=	444

FILESRC=	${.CURDIR}/../../contrib/file-3.12
.PATH:		${FILESRC}

PROG=		file
SRCS=		file.c apprentice.c fsmagic.c softmagic.c ascmagic.c is_tar.c \
		print.c compress.c
CFLAGS+=	-I${FILESRC} -DMAGIC='"${MAGIC}"'
MAN1=		file.0
MAN5=		magic.0

CLEANFILES+=	magic file.1 magic.5
# The pages named here too: Lite2's bsd.prog.mk gives a Makefile that
# defines `all' the whole target (`.if !target(all)'), where NetBSD 1.0's
# adds its own `all: ${PROG}' and pages to it, as this line assumed.
all:		file magic file.0 magic.0

MAGFILES=	${FILESRC}/Magdir/Header\
		${FILESRC}/Magdir/Localstuff\
		${FILESRC}/Magdir/[a-z]*
magic:		${MAGFILES}
	cat ${MAGFILES} > ${.TARGET}

file.1:		${FILESRC}/file.man
	sed -e s@__SECTION__@5@g -e s@__MAGIC__@${MAGIC}@g \
	    ${FILESRC}/file.man > ${.TARGET}
magic.5:	${FILESRC}/magic.man
	sed -e s@__SECTION__@5@g -e s@__MAGIC__@${MAGIC}@g \
	    ${FILESRC}/magic.man > ${.TARGET}

afterinstall:
	install -c -o ${MAGICOWN} -g ${MAGICGRP} -m ${MAGICMODE} magic \
	    ${DESTDIR}${MAGIC}

.include <bsd.prog.mk>
