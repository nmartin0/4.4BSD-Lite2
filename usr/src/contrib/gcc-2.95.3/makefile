#Build for this system, as contrib/gcc-2.3.3's makefile builds gcc.
#
# AI-ONLY NOTE: GCC 2.95.3, the system's C compiler, built to run on
# this system and installed where Lite2's own GCC was installed
# (contrib/gcc-2.3.3's Makefile, and etc/mtree/4.4BSD.usr): under
# /usr/libexec/gcc2 -- the driver as gcc-2.95.3, with
# gcc-i386-unknown-bsd4.4elf linked to it; cccp as /usr/libexec/gcc2/cpp,
# which usr.bin/cpp's script runs; cc1, cpp0, collect2, libgcc.a and
# GCC's own headers in gcc-lib/i386-unknown-bsd4.4elf/2.95.3 -- and
# /usr/bin/gcc a symbolic link to the driver, as gcc-2.3.3's install
# meant to make it (its line lacks the separator that would run it).
# And /usr/bin/cc, linked to gcc, by the maintainer's choice: mkdep
# runs `cc -M', every BSD has cc, and Lite2 has none. The pages,
# formatted, in /usr/contrib/man/cat1, where gcc-2.3.3's went. See
# README.BSD.
#
# GNU's configure and its own Makefiles do the work, as for
# contrib/binutils-2.11.2, in a directory of its own. On another
# machine, the build machine is named as build/sysroot.sh names it,
# i686-pc-linux-gnu: GCC 2.95.3's config.sub has no x86_64; the
# programs GCC runs while it builds -- genattr, gencodes and the rest
# -- are compiled by ${HOSTCC}, as HOST_CC, with the flags those
# sources of 2001 need from a compiler of today; libgcc by ${CC}, the
# cross compiler, since the xgcc built here is this system's program.
# On this system all three are one, and configure finds them so.
#
# Installed by name, from the build directory, and not by GNU's
# install targets, which know no DESTDIR.

CONFIG=		i386-unknown-bsd4.4elf
VERSION=	2.95.3
BINOWN?=	bin
BINGRP?=	bin
HOSTCC?=	cc
PREFIX=		/usr/libexec/gcc2
LIBSUBDIR=	${PREFIX}/gcc-lib/${CONFIG}/${VERSION}

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
.else
BUILD=	${.CURDIR}/obj.${MACHINE}
.endif

.if !empty(CC:M*-gcc)
CROSS=		--build=i686-pc-linux-gnu
BUILDCC=	${HOSTCC} -fcommon -w -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
CROSSFLAGS=	HOST_CC="${BUILDCC}" GCC_FOR_TARGET="${CC}" \
		AR="${CC:S/-gcc$/-ar/}" RANLIB="${CC:S/-gcc$/-ranlib/}"
.else
CROSS=		--build=${CONFIG}
BUILDCC=	${CC}
.endif

# GCC's distribution has a directory named install, which would
# otherwise stand for the target of that name.
.PHONY: all install clean cleandir

all:
.if ${MACHINE} == "i386"
	mkdir -p ${BUILD}
	cd ${BUILD} && { [ -f config.status ] || \
	    LD_PRELOAD= CC_FOR_BUILD="${BUILDCC}" CC="${CC}" \
	    sh ${.CURDIR}/configure ${CROSS} --host=${CONFIG} \
	    --target=${CONFIG} --prefix=${PREFIX} --libdir=${PREFIX} \
	    --with-gnu-as --with-gnu-ld --enable-languages=c --disable-nls; }
	cd ${BUILD} && ${MAKE} MAKEINFO=true ${CROSSFLAGS} all-gcc
	nroff -man ${.CURDIR}/gcc/gcc.1 > ${BUILD}/gcc.0
	nroff -man ${.CURDIR}/gcc/cccp.1 > ${BUILD}/cccp.0
.endif

install:
.if ${MACHINE} == "i386"
	mkdir -p ${DESTDIR}${LIBSUBDIR}/include
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 ${BUILD}/gcc/xgcc \
	    ${DESTDIR}${PREFIX}/gcc-${VERSION}
	rm -f ${DESTDIR}${PREFIX}/gcc-${CONFIG}
	ln ${DESTDIR}${PREFIX}/gcc-${VERSION} ${DESTDIR}${PREFIX}/gcc-${CONFIG}
	install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 ${BUILD}/gcc/cccp \
	    ${DESTDIR}${PREFIX}/cpp
	for f in cc1 cpp0 collect2; do \
		install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 \
		    ${BUILD}/gcc/$$f ${DESTDIR}${LIBSUBDIR}/$$f; \
	done
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/gcc/libgcc.a \
	    ${DESTDIR}${LIBSUBDIR}/libgcc.a
	for f in ${BUILD}/gcc/include/*; do \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 $$f \
		    ${DESTDIR}${LIBSUBDIR}/include; \
	done
	rm -f ${DESTDIR}/usr/bin/gcc ${DESTDIR}/usr/bin/cc
	ln -s ${PREFIX}/gcc-${VERSION} ${DESTDIR}/usr/bin/gcc
	ln -s gcc ${DESTDIR}/usr/bin/cc
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/gcc.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/gcc.0
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/cccp.0 \
	    ${DESTDIR}/usr/contrib/man/cat1/cccp.0
	rm -f ${DESTDIR}/usr/contrib/man/cat1/cpp.0
	ln -s cccp.0 ${DESTDIR}/usr/contrib/man/cat1/cpp.0
.endif

clean cleandir:
	rm -rf ${BUILD}

depend lint obj tags:
