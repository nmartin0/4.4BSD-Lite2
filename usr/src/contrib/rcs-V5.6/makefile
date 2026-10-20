#Build for this system: rcs.
#
# AI-ONLY NOTE: RCS 5.6 as Berkeley configured it -- src/Makefile's
# configuration section, which installs ci, co, ident, merge, rcs,
# rcsdiff, rcsmerge and rlog in /usr/contrib/bin, and man/Makefile,
# their pages formatted in /usr/contrib/man -- with GNU diff's lines of
# that section, the system's diff being GNU diff 2.6. The programs are
# built and installed by src/Makefile, as Berkeley's Makefile here
# runs it, under ${DESTDIR}, which it takes. The pages are
# man/Makefile's, formatted as it formats them, and installed by
# install rather than by its own chown bin.bin, whose names another
# machine reads as its own users, as hunt's and perl's are here.
#
# With src/conf.h as rcs's conf.sh wrote it on this system, kept, as
# FreeBSD 2.0 keeps the conf.h Garrett Wollman regenerated with
# conf.sh on its 4.4BSD-Lite system (1994), and as Berkeley shipped
# emacs's, gcc's and perl's configuration made on its own machine. conf.sh
# compiles and runs programs, which another machine cannot run for this
# one. conf_h, empty, is the Makefile's own way of taking a conf.h as
# it is: the objects no longer depend on it, so make neither remakes
# it nor, from this file, removes it; `make conf.h' in src remakes it.
#
# Built here, among the sources, where Berkeley built all its
# contributed software -- contrib/Makefile gives contrib no object
# directories -- and in a copy of them in a directory of its own only
# when the build has one, MAKEOBJDIRPREFIX, as on another machine,
# whose build leaves the sources as they are: as OpenBSD's perl
# wrapper (downsj, 1996) builds in place unless there is an object
# directory. The Makefile runs cc by name, so the PATH begins with
# GCC's and binutils' tooldir, as for MH.

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
.else
BUILD=	${.CURDIR}
.endif

.if !empty(CC:M*-gcc)
TOOLPATH=	PATH=${CC:H:H}/${CC:T:S/-gcc$//}/bin:$$PATH
.endif

BINOWN?=	bin
BINGRP?=	bin
PAGES1=		ci co ident merge rcs rcsdiff rcsintro rcsmerge rlog
PAGES5=		rcsfile

# What src/Makefile's clean removes, but conf.h.
CLEAN=	a.* *.o conf.error ci co ident merge rcs rcsdiff rcsmerge rlog \
	rcsclean

all:
.if defined(MAKEOBJDIRPREFIX)
	mkdir -p ${BUILD}
	[ -f ${BUILD}/src/Makefile ] || (cd ${.CURDIR} && pax -rw -pp . ${BUILD})
	rm -f ${BUILD}/makefile
.endif
	cd ${BUILD}/src && ${TOOLPATH} ${MAKE} -f Makefile conf_h= all
	cd ${BUILD}/man && for p in ${PAGES1}; do \
		nroff -man $$p.1 > $$p.0; \
	done
	cd ${BUILD}/man && for p in ${PAGES5}; do \
		nroff -man $$p.5 > $$p.0; \
	done

install:
	cd ${BUILD}/src && ${TOOLPATH} ${MAKE} -f Makefile conf_h= \
	    DESTDIR=${DESTDIR} install
	cd ${BUILD}/man && for p in ${PAGES1}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$p.0 \
		    ${DESTDIR}/usr/contrib/man/cat1; \
	done
	cd ${BUILD}/man && for p in ${PAGES5}; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$p.0 \
		    ${DESTDIR}/usr/contrib/man/cat5; \
	done

clean cleandir:
.if defined(MAKEOBJDIRPREFIX)
	rm -rf ${BUILD}
.else
	cd ${BUILD}/src && rm -f ${CLEAN}
	cd ${BUILD}/man && rm -f *.0
.endif

depend lint obj tags:
