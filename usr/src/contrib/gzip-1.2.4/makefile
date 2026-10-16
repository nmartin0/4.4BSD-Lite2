#Build for this system: gzip.
#
# AI-ONLY NOTE: gzip 1.2.4, built by the Makefile it came with --
# which GNU configure wrote on Berkeley's machine, with prefix
# /usr/contrib, and which Berkeley's installbin target finished by
# formatting the pages into /usr/contrib/man/cat1 -- run in the object
# directory with srcdir pointed back here, as OpenBSD's
# Makefile.bsd-wrapper runs GNU's own Makefiles in ${.OBJDIR}. Then
# installed by name as that installbin installs it, under ${DESTDIR},
# which it does not know: gzip, gunzip and gzcat in /usr/contrib/bin,
# gzdiff, gzcmp, gzgrep, gzmore, gznew, gzforce and gzexe beside them,
# and the seven pages.

BINOWN?=	bin
BINGRP?=	bin
B=		${DESTDIR}/usr/contrib/bin
M=		${DESTDIR}/usr/contrib/man/cat1
SCRIPTS=	gzdiff gzgrep gzmore gznew gzforce gzexe
PAGES=		gzexe gzip zdiff zforce zgrep zmore znew

all:
	${MAKE} -f ${.CURDIR}/Makefile srcdir=${.CURDIR} VPATH=${.CURDIR} \
	    CC="${CC}" all
	for f in ${PAGES}; do \
		nroff -man ${.CURDIR}/$$f.1 > $$f.0; \
	done

install:
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 gzip ${B}/gzip
	for f in ${SCRIPTS}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 555 $$f ${B}/$$f; \
	done
	rm -f ${B}/gzcmp ${B}/gunzip ${B}/gzcat
	ln ${B}/gzdiff ${B}/gzcmp
	ln ${B}/gzip ${B}/gunzip
	ln ${B}/gzip ${B}/gzcat
	for f in ${PAGES}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f.0 ${M}/$$f.0; \
	done

clean cleandir:
	rm -f gzip gunzip gzcat *.o ${SCRIPTS} *.0

depend lint obj tags:
