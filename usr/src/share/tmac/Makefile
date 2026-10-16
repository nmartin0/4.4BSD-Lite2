#	@(#)Makefile	8.1 (Berkeley) 6/8/93

# AI-ONLY NOTE: tmac.r is not here: the Lite cut removed it, and
# FreeBSD 2.0, NetBSD 1.0 and OpenBSD drop it from this list.
FILES=	tmac.andoc tmac.s tmac.e
MSRCS=  doc doc-ditroff doc-common doc-nroff doc-syms

LINKS=	${BINDIR}/tmac/tmac.andoc ${BINDIR}/tmac/tmac.an
NOOBJ=	noobj

# old macros not currently installed...
# OFILES=	tmac.a tmac.cp

all clean cleandir depend lint tags:

beforeinstall:
	for i in ${MSRCS}; do \
		sed -f strip.sed < $$i > ${DESTDIR}${BINDIR}/tmac/tmac.$$i; \
		chown ${BINOWN}.${BINGRP} ${DESTDIR}${BINDIR}/tmac/tmac.$$i; \
		chmod 444 ${DESTDIR}${BINDIR}/tmac/tmac.$$i; \
	done
	install -c -o ${BINOWN} -g ${BINOWN} -m 444 ${FILES} \
		${DESTDIR}${BINDIR}/tmac


.include <bsd.prog.mk>
