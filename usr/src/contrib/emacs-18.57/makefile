#Build for this system: emacs.
#
# AI-ONLY NOTE: GNU Emacs 18.57 as Berkeley configured it, built and
# installed by its own Makefile, all and install: emacs, etags, ctags
# and emacsclient in /usr/contrib/bin, the Lisp, etc and info in
# /usr/contrib/lib/emacs, the lock directory /var/emacs/lock, and its
# page in /usr/contrib/man/cat1. On the i386 with src/config.i386bsd.h.
#
# src/makefile links config.${MACHINE}bsd.h as config.h, but etc is
# built first and its programs include ../src/config.h, which Lite2
# ships linked to config.hp300bsd.h; so the link is made here first,
# as src/makefile makes it.
#
# Emacs is made by running temacs, which loads the Lisp and dumps
# itself as emacs: a program of this system, which only this system
# can run. So it is built on the system, by the system's own make
# build -- decided 2026-10-22 -- and a build on another machine, which
# has an object directory, MAKEOBJDIRPREFIX, passes over it.

all:
.if defined(MAKEOBJDIRPREFIX)
	@echo "emacs: dumped by running temacs, so built on the system"
.else
	cd ${.CURDIR}/src && rm -f config.h && \
	    ln -s config.${MACHINE}bsd.h config.h
	cd ${.CURDIR} && ${MAKE} -f Makefile all
.endif

install:
.if !defined(MAKEOBJDIRPREFIX)
	cd ${.CURDIR} && ${MAKE} -f Makefile install
.endif

clean cleandir:
.if !defined(MAKEOBJDIRPREFIX)
	cd ${.CURDIR} && ${MAKE} -f Makefile clean
.endif

depend lint obj tags:
