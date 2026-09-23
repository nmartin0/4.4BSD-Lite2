#	@(#)bsd.subdir.mk	8.1 (Berkeley) 6/8/93

.MAIN: all

STRIP?=	-s

BINGRP?=	bin
BINOWN?=	bin
BINMODE?=	555

# AI-ONLY NOTE: `set -e' makes a failed cd end that entry. Without it,
# make reran in this directory without end for every SUBDIR entry
# missing from the tree (bin/Makefile lists ed and expr).
#
# Taken from NetBSD 1.0 (October 1994), share/mk/bsd.subdir.mk, which
# reads `(set -e; if test -d ...'; OpenBSD's has read the same since
# 1996 and still does. Chosen over the other thing both of them did --
# rewriting the loop, which OpenBSD has since done around SKIPDIR and
# Makefile.bsd-wrapper -- because one token leaves Lite2's loop as it
# is. Neither file carries a copyright notice; both descend from the
# 4.4BSD file this one is.
_SUBDIRUSE: .USE
	@for entry in ${SUBDIR}; do \
		(set -e; if test -d ${.CURDIR}/$${entry}.${MACHINE}; then \
			echo "===> $${entry}.${MACHINE}"; \
			cd ${.CURDIR}/$${entry}.${MACHINE}; \
		else \
			echo "===> $$entry"; \
			cd ${.CURDIR}/$${entry}; \
		fi; \
		${MAKE} ${.TARGET:realinstall=install}); \
	done

${SUBDIR}::
	@if test -d ${.TARGET}.${MACHINE}; then \
		cd ${.CURDIR}/${.TARGET}.${MACHINE}; \
	else \
		cd ${.CURDIR}/${.TARGET}; \
	fi; \
	${MAKE} all

.if !target(all)
all: _SUBDIRUSE
.endif

.if !target(clean)
clean: _SUBDIRUSE
.endif

.if !target(cleandir)
cleandir: _SUBDIRUSE
.endif

.if !target(depend)
depend: _SUBDIRUSE
.endif

.if !target(manpages)
manpages: _SUBDIRUSE
.endif

.if !target(install)
.if !target(beforeinstall)
beforeinstall:
.endif
.if !target(afterinstall)
afterinstall:
.endif
install: afterinstall
afterinstall: realinstall
realinstall: beforeinstall _SUBDIRUSE
.endif
.if !target(maninstall)
maninstall: _SUBDIRUSE
.endif

.if !target(lint)
lint: _SUBDIRUSE
.endif

.if !target(obj)
obj: _SUBDIRUSE
.endif

.if !target(objdir)
objdir: _SUBDIRUSE
.endif

.if !target(tags)
tags: _SUBDIRUSE
.endif
