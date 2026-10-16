#	@(#)Makefile	8.2 (Berkeley) 5/23/94

# The following modules do not build/install:
# 10.gdb, 13.rcs

BINDIR=	/usr/share/doc/psd
FILES=	Makefile Title
SUBDIR=	01.cacm 02.implement 03.iosys 04.uprog 05.sysman 06.Clang \
	07.pascal 08.f77 09.f77io 11.adb 12.make 14.sccs 15.yacc \
	16.lex 17.m4 18.gprof 19.curses 20.ipctut 21.ipc

Title.ps: ${FILES}
	groff Title > ${.TARGET}

# AI-ONLY NOTE: from ${.CURDIR}, as games/fortune/datfiles installs
# its files: the build runs this in an object directory, which this
# directory has as the parent of its subdirectories' (build/make.sh).
beforeinstall:
	(cd ${.CURDIR} && install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${FILES} ${DESTDIR}${BINDIR})

.include <bsd.subdir.mk>
