#Build for this system: bib.
#
# AI-ONLY NOTE: bib, built from Berkeley's own sources and installed
# where Berkeley's Makefile installs it -- the programs in
# /usr/contrib/bin, the macro libraries in /usr/contrib/lib/bmac, and
# the pages, formatted, in /usr/contrib/man/cat1 -- under ${DESTDIR},
# which it does not know. bmac and its two directories are made as
# macs/Makefile makes them, by -mkdir as it installs: 4.4BSD.usr has
# them, and 4.4BSD.dist, which makes the installed hierarchy, does not.
#
# Not by that Makefile and src/Makefile themselves, which build among
# the sources: src/Makefile's own .c.o rule compiles $*.c, which only
# the source directory has, so it cannot build in an object directory
# as contrib/ builds here. Its programs, objects and CFLAGS are the
# same below; sys.mk's .c.o finds the sources by .PATH.
#
# bibinc.fullnames, bibinc.shortnames, tibinc.fullnames and
# tibinc.shortnames are installed as Berkeley shipped them, made by
# bibinc from bibinc.names; macs/Makefile remakes them by running
# bibinc, a program built for this system, which the machine doing a
# cross build cannot run. Makefile.e.g., in macs/Makefile's list, is
# not in Berkeley's distribution, and is not installed.

BINOWN?=	bin
BINGRP?=	bin
B=		${DESTDIR}/usr/contrib/bin
L=		${DESTDIR}/usr/contrib/lib/bmac
M=		${DESTDIR}/usr/contrib/man/cat1

.PATH:		${.CURDIR}/src
CFLAGS+=	-DBASEDIR=\"/usr/contrib\" -DINCORE -fwritable-strings

INVo=		invert.o streams.o makekey.o
LOOKo=		lookup.o locate.o alpha.seek.o streams.o makekey.o
BIBo=		bib.o bibargs.o locate.o alpha.seek.o streams.o makekey.o
LISTo=		listrefs.o bibargs.o streams.o makekey.o
BIBINCo=	bibinc.o

PROGS=		invert lookup listrefs bib bibinc
PAGES=		bib invert
BIBSRCS=	bib.astro bib.compsurv bib.foot bib.hnf bib.jrnl bib.lib \
		bib.list bib.llist bib.opena bib.openn bib.spe bib.spe2 \
		bib.stda bib.stdafull bib.stdn bib.stdsn bib.supn \
		bibinc.names bibmac.me bmac.open bmac.std common macros.me
BIBMACS=	bibinc.fullnames bibinc.shortnames bibinc.publishers \
		bibinc.fulllocal bibinc.shortlocal
TIBMACS=	tibinc.fullnames tibinc.shortnames \
		bibinc.fulllocal bibinc.shortlocal

all: ${PROGS}
	for f in ${PAGES}; do \
		nroff -man -h ${.CURDIR}/man/$$f.1 > $$f.0; \
	done

invert: ${INVo}
	${CC} ${CFLAGS} ${INVo} -o $@
lookup: ${LOOKo}
	${CC} ${CFLAGS} ${LOOKo} -o $@
bib: ${BIBo}
	${CC} ${CFLAGS} ${BIBo} -o $@
listrefs: ${LISTo}
	${CC} ${CFLAGS} ${LISTo} -o $@
bibinc: ${BIBINCo}
	${CC} ${CFLAGS} ${BIBINCo} -o $@

bib.o bibargs.o listrefs.o makekey.o: bib.h
invert.o locate.o streams.o: streams.h bib.h
alpha.seek.o: streams.h
bibinc.o: adsdebug.h comments.h streams.h

install:
	for f in ${PROGS}; do \
		install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 $$f ${B}/$$f; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 555 ${.CURDIR}/src/bib2tib \
	    ${B}/bib2tib
	-mkdir ${L}
	-mkdir ${L}/bibmacs
	-mkdir ${L}/tibmacs
	for f in ${BIBSRCS}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
		    ${.CURDIR}/macs/$$f ${L}/$$f; \
	done
	for f in ${BIBMACS}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
		    ${.CURDIR}/macs/$$f ${L}/bibmacs/$$f; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/macs/tibinc.fullnames ${L}/tibmacs/bibinc.fullnames
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/macs/tibinc.shortnames ${L}/tibmacs/bibinc.shortnames
	for f in bibinc.fulllocal bibinc.shortlocal; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
		    ${.CURDIR}/macs/$$f ${L}/tibmacs/$$f; \
	done
	for f in ${PAGES}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f.0 ${M}/$$f.0; \
	done

clean cleandir:
	rm -f ${PROGS} *.o *.0

depend lint obj tags:
