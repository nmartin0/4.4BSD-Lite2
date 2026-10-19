#Build for this system, as contrib/gdb-4.7.LBL's makefile builds gdb.
#
# AI-ONLY NOTE: binutils 2.11.2, the system's assembler, linker and
# binary tools, built to run on this system. Lite2's own for the i386
# -- usr.bin's ld, nm, ranlib, size, strip, strings, ar and gprof, and
# contrib/gas-1.38 -- read and write a.out only, and this system is
# ELF; NetBSD 1.4 replaced the same programs with binutils' "instead of
# the obsolete versions left over from 4.4BSD Lite", and NetBSD 1.6,
# whose binutils this is, installs the fourteen below. See README.BSD.
#
# GNU's configure and its own Makefiles do the work, as gdb-4.7.LBL's
# and gcc-2.3.3's makefiles have them do, run by this system's make.
# Built here, among the sources, where Berkeley built all its
# contributed software, gcc-2.3.3 and gdb-4.7.LBL among it --
# contrib/Makefile gives contrib no object directories -- and in a
# directory of its own only when the build has one, MAKEOBJDIRPREFIX,
# as on another machine, whose build leaves the sources as they are:
# as OpenBSD's perl wrapper (downsj, 1996) builds in place unless there
# is an object directory. GNU's Makefile is named, -f Makefile, as
# gcc-2.3.3's makefile names it, beside this one. Installed by name,
# program by program, and not by GNU's install targets: those copy
# as, ld, nm and strip to ${prefix} without ${DESTDIR}, onto the
# machine doing the build.
#
# configure runs with LD_PRELOAD empty. It judges a test failed if the
# compiler wrote anything to its error output, and under fakeroot(1)
# on a 64-bit build machine every 32-bit program -- the cross compiler
# is one -- is told by the dynamic linker that fakeroot's 64-bit
# library cannot be loaded into it; configure makes no file that
# needs a fake owner.

CONFIG=	i386-unknown-bsd4.4elf
BINOWN?=	bin
BINGRP?=	bin
HOSTCC?=	cc

.if defined(MAKEOBJDIRPREFIX)
BUILD=	${MAKEOBJDIRPREFIX}${.CURDIR}
CONFIGURE=	${.CURDIR}/configure
.else
BUILD=	${.CURDIR}
CONFIGURE=	./configure
.endif

# program	built as
PROGS=	addr2line	binutils/addr2line \
	ar		binutils/ar \
	as		gas/as-new \
	c++filt		binutils/cxxfilt \
	gprof		gprof/gprof \
	ld		ld/ld-new \
	nm		binutils/nm-new \
	objcopy		binutils/objcopy \
	objdump		binutils/objdump \
	ranlib		binutils/ranlib \
	readelf		binutils/readelf \
	size		binutils/size \
	strings		binutils/strings \
	strip		binutils/strip-new

# manual page	source; readelf has none in this release. c++filt's is
# a template, its name @PROGRAM@, as binutils' Makefile fills it in.
MANS=	addr2line	binutils/addr2line.1 \
	ar		binutils/ar.1 \
	as		gas/doc/as.1 \
	c++filt		binutils/cxxfilt.man \
	gprof		gprof/gprof.1 \
	ld		ld/ld.1 \
	nm		binutils/nm.1 \
	objcopy		binutils/objcopy.1 \
	objdump		binutils/objdump.1 \
	ranlib		binutils/ranlib.1 \
	size		binutils/size.1 \
	strings		binutils/strings.1 \
	strip		binutils/strip.1

all:
.if ${MACHINE} == "i386"
.if defined(MAKEOBJDIRPREFIX)
	mkdir -p ${BUILD}
.endif
	cd ${BUILD} && { [ -f config.status ] || \
	    LD_PRELOAD= CC_FOR_BUILD="${HOSTCC}" sh ${CONFIGURE} \
	    --host=${CONFIG} --target=${CONFIG} --prefix=/usr --disable-nls; }
	cd ${BUILD} && ${MAKE} -f Makefile MAKEINFO=true \
	    all-gas all-ld all-binutils all-gprof
.endif

# AI-ONLY NOTE: strip is stripped before it is installed, by the strip
# already there, and installed without -s, as FreeBSD 3.0's
# gnu/usr.bin/binutils/strip/Makefile does it (jdp, 1998): install -s
# runs /usr/bin/strip on what it has just put there, and on this
# system that is the new strip itself, still mode 0600, which cannot
# run.
install:
.if ${MACHINE} == "i386"
	@set ${PROGS}; while [ $$# -ge 2 ]; do \
		if [ $$1 = strip ]; then \
			echo strip -o ${BUILD}/$$2.s ${BUILD}/$$2; \
			strip -o ${BUILD}/$$2.s ${BUILD}/$$2 || exit 1; \
			echo install -c -o ${BINOWN} -g ${BINGRP} -m 555 \
			    ${BUILD}/$$2.s ${DESTDIR}/usr/bin/$$1; \
			install -c -o ${BINOWN} -g ${BINGRP} -m 555 \
			    ${BUILD}/$$2.s ${DESTDIR}/usr/bin/$$1 || exit 1; \
		else \
			echo install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 \
			    ${BUILD}/$$2 ${DESTDIR}/usr/bin/$$1; \
			install -c -s -o ${BINOWN} -g ${BINGRP} -m 555 \
			    ${BUILD}/$$2 ${DESTDIR}/usr/bin/$$1 || exit 1; \
		fi; \
		shift 2; \
	done
	@set ${MANS}; while [ $$# -ge 2 ]; do \
		echo "nroff -man ${.CURDIR}/$$2 > ${BUILD}/$$1.0"; \
		sed -e "s/@PROGRAM@/$$1/" ${.CURDIR}/$$2 | \
		    nroff -man > ${BUILD}/$$1.0 || exit 1; \
		install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${BUILD}/$$1.0 \
		    ${DESTDIR}/usr/share/man/cat1/$$1.0 || exit 1; \
		shift 2; \
	done
.endif

clean cleandir:
.if defined(MAKEOBJDIRPREFIX)
	rm -rf ${BUILD}
.else
	cd ${BUILD} && { [ ! -f Makefile ] || ${MAKE} -f Makefile distclean; }
	rm -f ${BUILD}/binutils/strip-new.s
.endif

depend lint obj tags:
