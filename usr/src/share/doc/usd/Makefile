#	@(#)Makefile	8.3 (Berkeley) 5/16/94

# The following modules do not build/install:
# 08.mh

BINDIR=	/usr/share/doc/usd
FILES=	Makefile Title
# AI-ONLY NOTE: not named: 01.begin, 02.learn, 03.shell, 05.dc, 06.bc,
# 09.edtut, 10.edadv, 15.sed, 16.awk, 17.msmacros, 21.troff,
# 22.trofftut, 23.eqn, 24.eqnguide, 25.tbl, 26.refer, 27.invert and
# 29.diction, documents the Lite cut deleted with their directories. A
# name with no directory stops the build here, as it stopped Lite2's
# own make, which ran each command with `sh -ec' (docs/deferred.md,
# *Sources and directories named but absent*).
SUBDIR=	04.csh 07.mail \
	11.vitut 12.exref 13.viref 14.jove \
	18.msdiffs 19.memacros 20.meref \
	28.bib 30.rogue \
	31.trek

Title.ps: ${FILES}
	groff -ms Title > ${.TARGET}

# AI-ONLY NOTE: from ${.CURDIR}, as games/fortune/datfiles installs
# its files: the build runs this in an object directory, which this
# directory has as the parent of its subdirectories' (build/make.sh).
beforeinstall:
	(cd ${.CURDIR} && install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${FILES} ${DESTDIR}${BINDIR})

.include <bsd.subdir.mk>
