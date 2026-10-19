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
# Those Makefiles build among the sources and name one another's
# directories, ../h and ../sbr, so they are run in a copy of these
# sources in a directory of their own -- the object directory when the
# build has one, obj.${MACHINE} here when not -- as the descendants'
# ports build a program in a copy of its sources, and are not run
# here, which they would fill with objects.
#
# They run cc, ld, ar and ranlib by those names. On another machine
# they are its own, so the PATH begins with the directory where GCC
# and binutils, built for this system, install them under those
# names: their tooldir, beside ${CC}'s directory, named by its prefix.

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
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile all

install:
	cd ${BUILD} && ${TOOLPATH} ${MAKE} -f Makefile DESTDIR=${DESTDIR} \
	    inst-all

clean cleandir:
	rm -rf ${BUILD}

depend lint obj tags:
