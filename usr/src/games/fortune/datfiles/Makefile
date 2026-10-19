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

# AI-ONLY NOTE: strfile and the rot13 as FreeBSD 4.0's Makefile runs
# them. strfile by name, the strfile just built after anything on the
# PATH: on a build machine of another kind that is the tree's own
# strfile built for it, which build/sysroot.sh puts first on the
# PATH, as FreeBSD's build-tools did; Lite2's ../strfile/obj/strfile
# is a program for the target. Then where the build left it: in the
# object directory beside this one, or, on this system, where `make
# obj' makes none here, in Lite2's ../strfile/obj. And tr for
# /usr/games/caesar 13, which is the build machine's and may not be
# there; NetBSD 1.6's uses tr too.
fortunes.dat fortunes2.dat fortunes2-o.dat limerick.dat startrek.dat zippy.dat:
	PATH=$$PATH:${.OBJDIR}/../strfile:${.CURDIR}/../strfile/obj \
	    strfile -rs ${.CURDIR}/${.TARGET:R} ${.TARGET}

fortunes-o.dat: fortunes-o
	PATH=$$PATH:${.OBJDIR}/../strfile:${.CURDIR}/../strfile/obj \
	    strfile -rsx fortunes-o ${.TARGET}

fortunes-o: fortunes-o.${TYPE}
	tr a-zA-Z n-za-mN-ZA-M < ${.ALLSRC} > ${.TARGET}

.include <bsd.prog.mk>
