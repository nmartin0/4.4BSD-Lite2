#	@(#)Makefile	8.1 (Berkeley) 6/5/93

# AI-ONLY NOTE: connectives and words are not here: the Lite cut
# removed both. words is a link to web2, as FreeBSD 2.0's Makefile
# makes it; NetBSD 1.0's and OpenBSD's drop the same two files and
# link words to web2 too.
FILES=	README propernames web2 web2a
NOOBJ=	noobj

all clean cleandir depend lint tags:

install:
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${FILES} \
	    ${DESTDIR}${BINDIR}/dict
	(cd ${DESTDIR}${BINDIR}/dict; rm -f words; ln -s web2 words)

.include <bsd.prog.mk>
