#Build for this system: BIND.
#
# AI-ONLY NOTE: BIND 4.9.2, as Berkeley shipped it, configured in its
# Makefile for 4.4BSD -- named and named-xfer in /usr/sbin and
# /usr/libexec, nslookup, dig and host, the pages -- built by its own
# Makefiles, all and install, under ${DESTDIR}, which they take.
#
# Those Makefiles build among the sources, and its own `links' target,
# which makes a tree of links to build in elsewhere, wants res/ and
# compat/, which Lite2's copy does not have; so they are run in a copy
# of these sources in a directory of their own -- the object directory
# when the build has one, obj.${MACHINE} here when not -- as MH's are
# (contrib/mh-6.8.3a/makefile), and not here. CC is given on the
# command line, which its Makefile passes down in MFLAGS; ranlib, ld
# and the like are run by name, so the PATH begins with GCC's and
# binutils' tooldir, their target's tools by those names.

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
.else
BUILD=	${.CURDIR}/obj.${MACHINE}
.endif

.if !empty(CC:M*-gcc)
TOOLPATH=	PATH=${CC:H:H}/${CC:T:S/-gcc$//}/bin:$$PATH
.endif

# AI-ONLY NOTE: the copy leaves out obj.${MACHINE}, where it is made
# on a build with no MAKEOBJDIRPREFIX, inside this directory. Its
# directory is made only if it is not there, as share/mk's obj target
# makes its own: Lite2's mkdir -p, unlike POSIX's, fails on one that
# exists.
all:
	test -d ${BUILD} || mkdir -p ${BUILD}
	[ -f ${BUILD}/Makefile ] || (cd ${.CURDIR} && \
	    pax -rw -pp -s ',^\./obj\.${MACHINE}.*$$,,' . ${BUILD})
	rm -f ${BUILD}/makefile
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile CC="${CC}" all

install:
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile CC="${CC}" \
	    DESTDIR=${DESTDIR} install

clean cleandir:
	rm -rf ${BUILD}

depend lint obj tags:
