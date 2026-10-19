#Build for this system: MH.
#
# AI-ONLY NOTE: MH 6.8.3a, as Berkeley configured it -- conf/MH.bsd44,
# already run through mhconfig into every Makefile here and into
# config/config.c -- built by those Makefiles, all and install, as
# its Makefile runs them: the programs in /usr/contrib/mh-6.8/bin, its
# library in /usr/contrib/mh-6.8/lib, both of which
# etc/mtree/4.4BSD.usr makes, and its pages in /usr/contrib/man, all
# under ${DESTDIR}, which those Makefiles take.
#
# Built here, among the sources, where Berkeley built all its
# contributed software -- contrib/Makefile gives contrib no object
# directories -- and in a directory of its own only when the build has
# one, MAKEOBJDIRPREFIX, as on another machine, whose build leaves
# the sources as they are: as OpenBSD's perl wrapper (downsj, 1996)
# builds in place unless there is an object directory. There it
# builds in a copy of these sources, since the Makefiles name one
# another's directories, ../h and ../sbr.
#
# They run cc, ld, ar and ranlib by those names. On another machine
# they are its own, so the PATH begins with the directory where GCC
# and binutils, built for this system, install them under those
# names: their tooldir, beside ${CC}'s directory, named by its prefix.

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
.else
BUILD=	${.CURDIR}
.endif

.if !empty(CC:M*-gcc)
TOOLPATH=	PATH=${CC:H:H}/${CC:T:S/-gcc$//}/bin:$$PATH
.endif

all:
.if defined(MAKEOBJDIRPREFIX)
	mkdir -p ${BUILD}
	[ -f ${BUILD}/Makefile ] || (cd ${.CURDIR} && pax -rw -pp . ${BUILD})
	rm -f ${BUILD}/makefile
.endif
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile all

install:
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile DESTDIR=${DESTDIR} \
	    inst-all

clean cleandir:
.if defined(MAKEOBJDIRPREFIX)
	rm -rf ${BUILD}
.else
	cd ${BUILD} && ${MAKE} -f Makefile clean
.endif

depend lint obj tags:
