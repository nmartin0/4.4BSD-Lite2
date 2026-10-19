#Build for this system: BIND.
#
# AI-ONLY NOTE: BIND 4.9.2, as Berkeley shipped it, configured in its
# Makefile for 4.4BSD -- named and named-xfer in /usr/sbin and
# /usr/libexec, nslookup, dig and host, the pages -- built by its own
# Makefiles, all and install, under ${DESTDIR}, which they take.
#
# Built here, among the sources, where Berkeley built all its
# contributed software -- contrib/Makefile gives contrib no object
# directories -- and in a directory of its own only when the build has
# one, MAKEOBJDIRPREFIX, as on another machine, whose build leaves
# the sources as they are: as OpenBSD's perl wrapper (downsj, 1996)
# builds in place unless there is an object directory. There it
# builds in a copy of these sources: BIND's own `links' target, which
# makes a tree of links to build in elsewhere, wants res/ and compat/,
# which Lite2's copy does not have.
#
# CC is given on the command line, which its Makefile passes down in
# MFLAGS; ranlib, ld and the like are run by name, so the PATH begins
# with GCC's and binutils' tooldir, their target's tools by those
# names.

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
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile CC="${CC}" all

install:
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile CC="${CC}" \
	    DESTDIR=${DESTDIR} install

clean cleandir:
.if defined(MAKEOBJDIRPREFIX)
	rm -rf ${BUILD}
.else
	cd ${BUILD} && ${MAKE} -f Makefile clean
.endif

depend lint obj tags:
