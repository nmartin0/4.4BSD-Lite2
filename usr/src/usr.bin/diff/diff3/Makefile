#	@(#)Makefile	8.1 (Berkeley) 06/06/93

# AI-ONLY NOTE: Berkeley's Makefile for diff3, from the CSRG history:
# the program in /usr/libexec, run by Berkeley's diff3.sh, installed
# as /usr/bin/diff3. The program is OpenBSD's diff3prog of July 2003
# -- "a matching diff3 from 32V w/ some UCB changes" (millert) --
# under the name the script calls; the page is Lite2's own.
PROG=	diff3
SRCS=	diff3prog.c
BINDIR=	/usr/libexec

beforeinstall:
	install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    ${.CURDIR}/diff3.sh ${DESTDIR}/usr/bin/${PROG}

.include "../../Makefile.inc"
.include <bsd.prog.mk>
