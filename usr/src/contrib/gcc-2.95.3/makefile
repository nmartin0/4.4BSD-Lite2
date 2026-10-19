#Build for this system, as contrib/gcc-2.3.3's makefile builds gcc.
#
# AI-ONLY NOTE: GCC 2.95.3, the system's C compiler, built to run on
# this system and installed where Lite2's own GCC was installed: in
# /usr/libexec/gcc2 itself, which contrib/gcc-2.3.3's Makefile makes
# its libsubdir, the directory its passes, libgcc and headers go in --
# the driver as gcc-2.95.3, with gcc-i386-unknown-bsd4.4elf linked to
# it; GCC's cpp, the driver that runs cpp0 as gcc -E does, as cpp,
# which usr.bin/cpp's script runs, as OpenBSD's script ran GCC 2.95's
# cpp as /usr/libexec/cpp (espie, 1999): GCC 2.95 gives the target's
# predefinitions (__ELF__, unix, __i386__) to the preprocessor through
# its drivers, and cccp alone, which gcc-2.3.3 installed there, knows
# none of them; cc1, cpp0, collect2, libgcc.a and GCC's own headers,
# in include -- the libsubdir given to GCC's Makefiles, and the
# driver told by config/i386/bsd44-elf.h's MD_EXEC_PREFIX -- and
# /usr/bin/gcc a symbolic link to the driver, as gcc-2.3.3's install
# meant to make it (its line lacks the separator that would run it).
# And /usr/bin/cc, linked to gcc, by the maintainer's choice: mkdep
# runs `cc -M', every BSD has cc, and Lite2 has none. The pages,
# formatted, in /usr/contrib/man/cat1, where gcc-2.3.3's went. See
# README.BSD.
#
# With its C++, as gcc-2.3.3 was built (its LANGUAGES = c c++): cc1plus
# beside cc1, and the g++ driver as /usr/libexec/gcc2/g++-2.95.3, with
# c++-2.95.3 linked to it, where gcc-2.3.3 installed g++-2.3.3 and
# c++-2.3.3; /usr/bin/g++ and /usr/bin/c++ linked to it as gcc and cc
# are to gcc's driver; and g++.0 beside gcc.0, as gcc-2.3.3's was.
# libgcc carries C++'s own support -- new, delete, exceptions, type
# information -- compiled by ${CC}, which on another machine is
# build/sysroot.sh's GCC, built with its C++ for this. And GCC 2.95.3's
# C++ library, libstdc++ 2.10.0 with libio, its iostreams, which the
# g++ driver links with every program (-lstdc++), built by GCC's own
# target-libstdc++ and installed as its install target installs it,
# by name: the archive beside libgcc.a as libstdc++.a; its headers,
# libio's and _G_config.h in ${PREFIX}/g++-include, the directory
# gcc-2.3.3 searched for C++'s (GPLUSPLUS_INCLUDE_DIR), where Lite2's
# libg++ would have put its own.
#
# GNU's configure and its own Makefiles do the work, as for
# contrib/binutils-2.11.2: here, among the sources, where Berkeley
# built gcc-2.3.3 and ran gdb-4.7.LBL's ./configure, and in a directory
# of its own only when the build has one, MAKEOBJDIRPREFIX, as on
# another machine, whose build leaves the sources as they are -- as
# OpenBSD's perl wrapper (downsj, 1996) builds in place unless there is
# an object directory. GNU's Makefile is named, -f Makefile, as
# gcc-2.3.3's makefile names it, beside this one. On another
# machine, the build machine is named as build/sysroot.sh names it,
# i686-pc-linux-gnu: GCC 2.95.3's config.sub has no x86_64; the
# programs GCC runs while it builds -- genattr, gencodes and the rest
# -- are compiled by ${HOSTCC}, as HOST_CC, with the flags those
# sources of 2001 need from a compiler of today; libgcc by ${CC}, the
# cross compiler, since the xgcc built here is this system's program.
# On this system all three are one, and configure finds them so.
#
# libio is configured for the target before libstdc++, which its own
# Makefile configures first: libstdc++'s configure reads
# ../libio/config.shared, which, built in place, is the target's libio,
# there only once that is configured -- with none, libstdc++'s
# Makefile has no rules at all.
#
# configure, GCC's and that of each library it builds for the target,
# runs with LD_PRELOAD empty: under fakeroot the build machine's
# library is preloaded into every program, and each 32-bit one --
# the cross compiler, its preprocessor -- says it cannot be, which
# configure reads as a header not found. Nothing here needs fakeroot
# before install.
#
# Installed by name, from the build directory, and not by GNU's
# install targets, which know no DESTDIR. Its directories are in
# etc/mtree/4.4BSD.dist, and made here only if they are not there, by
# the line gcc-2.3.3's Makefile makes its own with.

CONFIG=		i386-unknown-bsd4.4elf
VERSION=	2.95.3
BINOWN?=	bin
BINGRP?=	bin
HOSTCC?=	cc
PREFIX=		/usr/libexec/gcc2
LIBSUBDIR=	${PREFIX}
GXXINC=		${PREFIX}/g++-include
TBUILD=		${BUILD}/${CONFIG}
# libstdc++'s HEADERS, as its Makefile.in names them.
STDCXXHDRS=	cassert cctype cerrno cfloat ciso646 climits clocale cmath \
		complex csetjmp csignal cstdarg cstddef cstdio cstdlib \
		cstring ctime cwchar cwctype string stdexcept algorithm deque \
		functional hash_map hash_set iterator list map memory numeric \
		pthread_alloc queue rope set slist stack utility vector \
		fstream iomanip iostream strstream iosfwd bitset valarray \
		sstream

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
CONFIGURE=	${.CURDIR}/configure
.else
BUILD=	${.CURDIR}
CONFIGURE=	./configure
.endif
GFLAGS=	-f Makefile MAKEINFO=true libsubdir=${LIBSUBDIR}

.if !empty(CC:M*-gcc)
CROSS=		--build=i686-pc-linux-gnu
BUILDCC=	${HOSTCC} -fcommon -w -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
CROSSFLAGS=	HOST_CC="${BUILDCC}" GCC_FOR_TARGET="${CC}" \
		AR="${CC:S/-gcc$/-ar/}" RANLIB="${CC:S/-gcc$/-ranlib/}" \
		CC_FOR_TARGET="${CC}" CXX_FOR_TARGET="${CC}" \
		AR_FOR_TARGET="${CC:S/-gcc$/-ar/}" \
		RANLIB_FOR_TARGET="${CC:S/-gcc$/-ranlib/}" \
		NM_FOR_TARGET="${CC:S/-gcc$/-nm/}"
.else
CROSS=		--build=${CONFIG}
BUILDCC=	${CC}
.endif

# GCC's distribution has a directory named install, which would
# otherwise stand for the target of that name.
.PHONY: all install clean cleandir

all:
.if ${MACHINE} == "i386"
.if defined(MAKEOBJDIRPREFIX)
	mkdir -p ${BUILD}
.endif
	cd ${BUILD} && { [ -f config.status ] || \
	    LD_PRELOAD= CC_FOR_BUILD="${BUILDCC}" CC="${CC}" \
	    sh ${CONFIGURE} ${CROSS} --host=${CONFIG} \
	    --target=${CONFIG} --prefix=${PREFIX} --libdir=${PREFIX} \
	    --with-gnu-as --with-gnu-ld --enable-languages=c,c++ --disable-nls \
	    --with-gxx-include-dir=${PREFIX}/g++-include; }
	cd ${BUILD} && ${MAKE} ${GFLAGS} ${CROSSFLAGS} all-gcc
	cd ${BUILD}/gcc && ${MAKE} ${GFLAGS} ${CROSSFLAGS} cpp
	cd ${BUILD} && LD_PRELOAD= ${MAKE} ${GFLAGS} ${CROSSFLAGS} \
	    configure-target-libio all-target-libstdc++
	nroff -man ${.CURDIR}/gcc/gcc.1 > ${BUILD}/gcc.0
	nroff -man ${.CURDIR}/gcc/cccp.1 > ${BUILD}/cccp.0
	nroff -man ${.CURDIR}/gcc/cp/g++.1 > ${BUILD}/g++.0
.endif

install:
.if ${MACHINE} == "i386"
	-if [ -d ${DESTDIR}${LIBSUBDIR}/include ] ; then true ; \
	    else mkdir ${DESTDIR}${LIBSUBDIR}/include ; fi
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 ${BUILD}/gcc/xgcc \
	    ${DESTDIR}${PREFIX}/gcc-${VERSION}
	rm -f ${DESTDIR}${PREFIX}/gcc-${CONFIG}
	ln ${DESTDIR}${PREFIX}/gcc-${VERSION} ${DESTDIR}${PREFIX}/gcc-${CONFIG}
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 ${BUILD}/gcc/g++ \
	    ${DESTDIR}${PREFIX}/g++-${VERSION}
	rm -f ${DESTDIR}${PREFIX}/c++-${VERSION}
	ln ${DESTDIR}${PREFIX}/g++-${VERSION} ${DESTDIR}${PREFIX}/c++-${VERSION}
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 ${BUILD}/gcc/cpp \
	    ${DESTDIR}${PREFIX}/cpp
	for f in cc1 cc1plus cpp0 collect2; do \
		install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 \
		    ${BUILD}/gcc/$$f ${DESTDIR}${LIBSUBDIR}/$$f; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/gcc/libgcc.a \
	    ${DESTDIR}${LIBSUBDIR}/libgcc.a
	for f in ${BUILD}/gcc/include/*; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f \
		    ${DESTDIR}${LIBSUBDIR}/include; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${TBUILD}/libstdc++/libstdc++.a ${DESTDIR}${LIBSUBDIR}/libstdc++.a
	-if [ -d ${DESTDIR}${GXXINC} ] ; then true ; \
	    else mkdir ${DESTDIR}${GXXINC} ; fi
	-if [ -d ${DESTDIR}${GXXINC}/std ] ; then true ; \
	    else mkdir ${DESTDIR}${GXXINC}/std ; fi
	cd ${.CURDIR}/libstdc++ && for f in ${STDCXXHDRS}; do \
		if [ -f stl/$$f ]; then s=stl/$$f; else s=$$f; fi; \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$s \
		    ${DESTDIR}${GXXINC}/$$f; \
	done
	cd ${.CURDIR}/libstdc++ && for f in *.h std/*.h std/*.cc std/*.tcc; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f \
		    ${DESTDIR}${GXXINC}/$$f; \
	done
	cd ${.CURDIR}/libstdc++/stl && for f in *.h; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f \
		    ${DESTDIR}${GXXINC}/$$f; \
	done
	cd ${.CURDIR}/libio && for f in *.h; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f \
		    ${DESTDIR}${GXXINC}/$$f; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${TBUILD}/libio/_G_config.h ${DESTDIR}${GXXINC}/_G_config.h
	rm -f ${DESTDIR}/usr/bin/gcc ${DESTDIR}/usr/bin/cc
	ln -s ${PREFIX}/gcc-${VERSION} ${DESTDIR}/usr/bin/gcc
	ln -s gcc ${DESTDIR}/usr/bin/cc
	rm -f ${DESTDIR}/usr/bin/g++ ${DESTDIR}/usr/bin/c++
	ln -s ${PREFIX}/g++-${VERSION} ${DESTDIR}/usr/bin/g++
	ln -s g++ ${DESTDIR}/usr/bin/c++
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/gcc.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/gcc.0
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/cccp.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/cccp.0
	rm -f ${DESTDIR}/usr/contrib/man/cat1/cpp.0
	ln -s cccp.0 ${DESTDIR}/usr/contrib/man/cat1/cpp.0
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/g++.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/g++.0
.endif

clean cleandir:
.if defined(MAKEOBJDIRPREFIX)
	rm -rf ${BUILD}
.else
	cd ${BUILD} && { [ ! -f Makefile ] || ${MAKE} -f Makefile distclean; }
	rm -f ${BUILD}/gcc.0 ${BUILD}/cccp.0 ${BUILD}/g++.0
.endif

depend lint obj tags:
