#	@(#)Makefile	8.1 (Berkeley) 6/11/93

SUBDIR=	boggle mkdict mkindex

# AI-ONLY NOTE: mkdict and mkindex found in their object directories
# as OpenBSD's Makefile of 1995 finds them, wherever those are; and
# the words read from this tree's dictionary, as NetBSD 1.6's reads
# them, not from /usr/share/dict on the machine doing the build.
# words is web2 (share/dict).
MKDICT!=cd $(.CURDIR)/mkdict; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkdict\n" | ${MAKE} -r -s -f - xxx
MKINDEX!=cd $(.CURDIR)/mkindex; \
	printf "xxx:\n\techo \$${.OBJDIR}/mkindex\n" | ${MAKE} -r -s -f - xxx
WORDS=	${.CURDIR}/../../share/dict/web2

beforeinstall:
	${MKDICT} < ${WORDS} > \
	    ${DESTDIR}/usr/share/games/boggle/dictionary
	${MKINDEX} < ${DESTDIR}/usr/share/games/boggle/dictionary > \
	    ${DESTDIR}/usr/share/games/boggle/dictindex
	chown ${BINOWN}.${BINGRP} ${DESTDIR}/usr/share/games/boggle/dictionary
	chmod 444 ${DESTDIR}/usr/share/games/boggle/dictionary
	chown ${BINOWN}.${BINGRP} ${DESTDIR}/usr/share/games/boggle/dictindex
	chmod 444 ${DESTDIR}/usr/share/games/boggle/dictindex
	
.include <bsd.subdir.mk>
