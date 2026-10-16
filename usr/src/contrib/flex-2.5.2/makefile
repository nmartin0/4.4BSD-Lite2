#Build for this system: flex, as lex.
#
# AI-ONLY NOTE: flex 2.5.2, built by the Makefile it came with, which
# GNU configure wrote on Berkeley's machine, run in the object
# directory with srcdir pointed back here, as OpenBSD's
# Makefile.bsd-wrapper runs GNU's own Makefiles in ${.OBJDIR}. Lite2
# has no other lex: this Makefile installs it under /usr/local, which
# no PATH here names; it is installed, by the maintainer's choice, as
# FreeBSD 2.0 and NetBSD 1.0 install flex: /usr/bin/lex, with flex and
# flex++ linked to it and its page as lex.0 and flex.0 -- NetBSD
# 1.0's names. Its run-time library is lib/libl's libl.a, so libfl.a
# is not installed. Installed by name, not by the Makefile's install
# target, which knows no DESTDIR.

BINOWN?=	bin
BINGRP?=	bin

GNUMAKE=	${MAKE} -f ${.CURDIR}/Makefile srcdir=${.CURDIR} \
		VPATH=${.CURDIR} CC="${CC}" YACC="${YACC}"

all:
	${GNUMAKE} lex
	nroff -man ${.CURDIR}/flex.1 > lex.man

install:
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 lex \
	    ${DESTDIR}/usr/bin/lex
	rm -f ${DESTDIR}/usr/bin/flex ${DESTDIR}/usr/bin/flex++
	ln ${DESTDIR}/usr/bin/lex ${DESTDIR}/usr/bin/flex
	ln ${DESTDIR}/usr/bin/lex ${DESTDIR}/usr/bin/flex++
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 lex.man \
	    ${DESTDIR}/usr/share/man/cat1/lex.0
	rm -f ${DESTDIR}/usr/share/man/cat1/flex.0
	ln ${DESTDIR}/usr/share/man/cat1/lex.0 \
	    ${DESTDIR}/usr/share/man/cat1/flex.0

clean cleandir:
	rm -f lex lex.man libfl.a *.o .bootstrap scan.c skel.c \
	    parse.c parse.h y.tab.c y.tab.h

depend lint obj tags:
