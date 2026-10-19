#	@(#)Makefile	8.2 (Berkeley) 4/19/94

SRCS=	fortunes fortunes2 startrek zippy
BLDS=	fortunes.dat fortunes2.dat startrek.dat zippy.dat \
	fortunes-o fortunes-o.dat

# TO INSTALL THE POTENTIALLY OFFENSIVE FORTUNES, UNCOMMENT THE THREE
# LINES AND COMMENT OUT THE FOURTH LINE.

# THE THREE LINES
SRCS+=	fortunes2-o limerick
BLDS+=	fortunes2-o.dat limerick.dat
TYPE=	real

# THE FOURTH LINE
#TYPE=	fake

CLEANFILES+=${BLDS}

install: ${SRCS} ${BLDS}
	(cd ${.CURDIR} && install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${SRCS} ${DESTDIR}/usr/share/games/fortune)
	install -o ${BINOWN} -g ${BINGRP} -m 444 ${BLDS} \
	    ${DESTDIR}/usr/share/games/fortune

# AI-ONLY NOTE: Lite2's own commands, as STRFILE and CAESAR -- the
# strfile just built, in ../strfile/obj, which FreeBSD 2.0 kept as
# Lite2 had it, and the installed caesar, which games/ installs before
# fortune -- named as variables as NetBSD 1.0's Makefile names STRFILE
# and FreeBSD 2.2's CAESAR, and set by ?=, as NetBSD 1.6 sets its
# ZIC, so that a build on a machine of another kind, which cannot run
# this system's programs, gives its own: build/make.sh gives
# STRFILE=strfile, the tree's own built for the build machine, which
# build/sysroot.sh puts first on the PATH, and CAESAR as tr, as
# OpenBSD (1996), NetBSD 1.6 and FreeBSD 4.0 make the rot13.
STRFILE?=	${.CURDIR}/../strfile/obj/strfile
CAESAR?=	/usr/games/caesar 13

fortunes.dat fortunes2.dat fortunes2-o.dat limerick.dat startrek.dat zippy.dat:
	${STRFILE} -rs \
	    ${.CURDIR}/${.TARGET:R} ${.TARGET}

fortunes-o.dat: fortunes-o
	${STRFILE} -rsx fortunes-o ${.TARGET}

fortunes-o: fortunes-o.${TYPE}
	${CAESAR} < ${.ALLSRC} > ${.TARGET}

.include <bsd.prog.mk>
