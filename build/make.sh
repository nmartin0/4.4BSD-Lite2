#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# make.sh -- run bmake on this tree's own make rules for an i386 target.
#
#	sh build/make.sh [bmake arguments ...]
#
# Run it from the directory you want to build, as you would make(1):
#
#	cd usr/src/bin/cat && sh ../../../../build/make.sh -n
#
# WHY bmake
#
# 4.4BSD-Lite2's Makefiles need a BSD make. Its own usr.bin/make does
# not build on a modern Linux host without source changes: glibc has no
# `union wait' (compat.c, job.c) and Linux has no <ranlib.h> (arch.c).
# bmake is the maintained portable release of NetBSD's make, which is
# where Lite2's make came from -- usr.bin/make/Makefile carries a NetBSD
# RCS id (cgd, 1994). Measured with bmake 20200710, the version Debian
# 13 packages: it reads Lite2's sys.mk and bsd.*.mk and none of its own.
#
# This is a thin wrapper, not a build system. It follows the nbmake
# wrapper that NetBSD's build.sh writes (NetBSD 1.6): set the locale,
# export MACHINE and MAKEFLAGS, and exec make with the caller's
# arguments.
#
# WHAT IT SETS
#
#   MAKEFLAGS	-m <this tree>/usr/src/share/mk, so this tree's rules are
#		read rather than bmake's own. The path must be absolute:
#		bmake changes into the object directory before parsing,
#		and a relative -m then stops resolving. Exported, as
#		NetBSD's wrapper does; measured, sub-makes started by
#		bsd.subdir.mk find the rules through it.
#   MACHINE	i386. bmake otherwise uses the host's, and Lite2 picks
#		machine-dependent directories by it (lib/libc/${MACHINE}).
#		Lite2's rules do not use MACHINE_ARCH, so it is not set.
#   LC_ALL	C, as NetBSD's wrapper does, so sort, tr and awk behave
#		the same on every host.
#
# WHAT IT DOES NOT DO -- read this before running a real target
#
#   No object directory. Lite2's 466 committed obj symlinks point into
#   /usr/obj, which does not exist here, so bmake builds in the source
#   directory and writes into this repository.
#
#   No sysroot and no compiler settings.
#
# Until those are dealt with, use it with -n or -V only. The
# exceptions are the stages build/sysroot.sh runs: include/ is marked
# NOOBJ and installs only under the DESTDIR it is given, and libc and
# the startup files are built in object directories the script
# creates first.
#
# 72 SUBDIR entries in this tree name directories that are not in it
# (bin/Makefile lists ed and expr). A recursive target prints
# "cd: can't cd to ..." for each and moves on to the next entry; the
# recursion loops in share/mk end the entry when cd fails.
#
set -e

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
MKDIR="$REPO_ROOT/usr/src/share/mk"

[ -f "$MKDIR/sys.mk" ] || {
	printf '%s\n' "make.sh: no sys.mk in $MKDIR" >&2
	exit 1
}

# MAKEFLAGS is split on whitespace, so a path containing a space would
# arrive at bmake as two words.
case "$REPO_ROOT" in
*[[:space:]]*)
	printf '%s\n' "make.sh: the tree's path contains whitespace:" >&2
	printf '%s\n' "  $REPO_ROOT" >&2
	exit 1
	;;
esac

command -v bmake >/dev/null 2>&1 || {
	printf '%s\n' "make.sh: bmake not found; install it with" >&2
	printf '%s\n' "  sudo apt install bmake" >&2
	exit 1
}

LC_ALL=C
MACHINE=i386
MAKEFLAGS="-m $MKDIR${MAKEFLAGS:+ $MAKEFLAGS}"
export LC_ALL MACHINE MAKEFLAGS

exec bmake "$@"
