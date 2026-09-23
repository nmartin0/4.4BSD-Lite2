#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. Its shape follows two
# that solve the same problem: the nbmake-MACHINE wrapper NetBSD
# 1.6's build.sh generates, and OpenBSD's Makefile.cross, whose
# CROSSENV points AR, AS, CC, CPP, LD, RANLIB and the rest at the
# target's tools.
#
# make.sh -- run this tree's own make rules, building for i386 ELF.
#
#	sh build/make.sh [target ...] [VAR=value ...]
#
# Run it from the directory you want to build, as you would make(1):
#
#	cd usr/src/bin/cat && sh ../../../../build/make.sh
#
# It builds nothing itself. 4.4BSD-Lite2's own Makefiles do the work,
# exactly as they would on a Lite2 machine; this passes in what a
# person would otherwise type by hand, and every setting can still be
# overridden on the command line, which beats the environment.
#
# WHY bmake
#
# Lite2's Makefiles need a BSD make. Its own usr.bin/make does not
# build on a modern Linux host without source changes: glibc has no
# `union wait' (compat.c, job.c) and Linux has no <ranlib.h> (arch.c).
# bmake is the maintained portable release of NetBSD's make, which is
# where Lite2's make came from -- usr.bin/make/Makefile carries a
# NetBSD RCS id (cgd, 1994). Measured with bmake 20200710, the version
# Debian 13 packages: it reads Lite2's sys.mk and bsd.*.mk and none of
# its own.
#
# The shape follows the nbmake-MACHINE wrapper NetBSD's build.sh writes
# (NetBSD 1.6): set the locale, export the target settings, exec make.
#
# WHAT IT SETS
#
#   MAKEFLAGS	-m <this tree>/usr/src/share/mk, so this tree's rules
#		are read and not bmake's own. The path must be absolute:
#		bmake changes into the object directory before parsing.
#   MACHINE	i386; Lite2 picks machine-dependent directories by it.
#   LC_ALL	C, so sort, tr and awk behave the same on every host.
#   DESTDIR	${L2_BUILD}/root, the target root build/sysroot.sh
#		fills. Lite2's rules install there, and look there for
#		the libraries a program depends on.
#   MAKEOBJDIRPREFIX
#		${L2_BUILD}/obj, so objects land outside this
#		repository. bmake uses it only for a directory that
#		already exists, so the one for the current directory is
#		created here.
#   PATH	${L2_BUILD}/tools/bin first, for the host tools
#		sysroot.sh installs there (lorder).
#   BINOWN, BINGRP, LIBOWN, LIBGRP, LIBMODE
#		the building user, and libraries writable: install makes
#		them 444 and then runs ranlib -t, which writes.
#   CC, CPP, AS, LD
#		the settings below. Lite2 defaults all four with ?=, so
#		these take effect and a command-line assignment wins.
#
# THE COMPILER SETTINGS, each answering a measured failure
#
#   -m32	32-bit i386.
#   -std=gnu89	GCC 14 treats an implicit function declaration as an
#		error; 14 of 40 sampled libc sources use one, sleep.c
#		among them, which calls sigvec without <sys/signal.h>
#		declaring it. GCC 13 only warns, so an older host hides
#		this.
#   -fno-stack-protector -fno-pic
#		distribution GCC defaults to both; they leave
#		__stack_chk_fail_local and _GLOBAL_OFFSET_TABLE_
#		unresolved.
#   -fcommon	stdio/glue.h defines __sglue in a header; without this
#		fwalk.o and findfp.o both define it.
#   -nostdinc -isystem ROOT/usr/include
#		the target root's headers, never the host's.
#   -static	Lite2 has no shared libraries.
#   -specs=	which startup files and C library to link; see below.
#   --sysroot=ROOT
#		"the system is here, not /". NetBSD 10 passes
#		--sysroot=${DESTDIR}, FreeBSD 13 --sysroot=${WORLDTMP}.
#   -Wl,-nostdlib
#		stop the linker searching the directories built into it.
#		Without it a program asking for -lutil silently linked
#		the host's /lib32/libutil.a. NetBSD 10's bsd.prog.mk
#		passes it too.
#   CPP	for assembly: -traditional-cpp, as NetBSD 1.5's COMPILE.S
#		preprocesses assembly, so Lite2's /**/ pasting works, and
#		-I rather than -isystem, which puts a line marker inside
#		an instruction using a system header's macro.
#
# THE SPECS FILE
#
# Written to ${L2_BUILD}/build.specs each run. A specs file is how GCC
# is told which startup files and libraries to link. The recipe is
# GCC's own NetBSD ELF target definition -- crt0.o, gcrt0.o when
# profiling, crtbegin.o first, crtend.o last, -lc or -lc_p -- with full
# paths into the target root, because the host compiler's own search
# path holds its crtbegin.o, crti.o and libc.a and it finds those
# first: measured, a plain link took the host's crti.o, crtbeginT.o and
# glibc.
#
# No BSD uses a specs file: they build a compiler for their target, and
# then -B alone suffices, as NetBSD 10's bsd.prog.mk does. Measured
# here, -B does not suffice with a host compiler. This file is a
# stand-in for a cross-compiler and goes away when there is one.
#
# WHAT IT DOES NOT DO
#
#   No object directories for subdirectories, so a recursive build
#   writes those objects into the source tree.
#
#   No guard against a SUBDIR entry naming a directory that is not
#   here; share/mk ends such an entry, and 72 of them are reported and
#   skipped.
#
# HOST TOOLS: bmake, and GCC with 32-bit support --
#
#	sudo apt install bmake lib32gcc-14-dev
#
set -e

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC="$REPO_ROOT/usr/src"
MKDIR="$SRC/share/mk"

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

: "${L2_BUILD:=${HOME}/.cache/lite2}"
case "$L2_BUILD" in
/*)
	;;
*)
	printf '%s\n' "make.sh: L2_BUILD must be an absolute path" >&2
	exit 1
	;;
esac
case "$L2_BUILD" in
"$REPO_ROOT"|"$REPO_ROOT"/*)
	printf '%s\n' "make.sh: L2_BUILD must be outside the repository" >&2
	exit 1
	;;
*[[:space:]]*)
	printf '%s\n' "make.sh: L2_BUILD contains whitespace" >&2
	exit 1
	;;
esac

command -v bmake >/dev/null 2>&1 || {
	printf '%s\n' "make.sh: bmake not found; install it with" >&2
	printf '%s\n' "  sudo apt install bmake" >&2
	exit 1
}

libgcc=$(gcc -m32 -print-libgcc-file-name 2>/dev/null || true)
[ -f "$libgcc" ] || {
	printf '%s\n' "make.sh: no 32-bit libgcc.a for gcc; install with" >&2
	printf '%s\n' "  sudo apt install lib32gcc-14-dev" >&2
	exit 1
}

ROOT="$L2_BUILD/root"
OBJ="$L2_BUILD/obj"
TOOLS="$L2_BUILD/tools/bin"
SPECS="$L2_BUILD/build.specs"

mkdir -p "$L2_BUILD"
cat > "$SPECS" <<SPECSEOF
*startfile:
%{pg:$ROOT/usr/lib/gcrt0.o} %{!pg:%{p:$ROOT/usr/lib/gcrt0.o} %{!p:$ROOT/usr/lib/crt0.o}} $ROOT/usr/lib/crtbegin.o

*endfile:
$ROOT/usr/lib/crtend.o

*lib:
%{!p:%{!pg:-lc}} %{p:-lc_p} %{pg:-lc_p}

*libgcc:
$libgcc

*link_libgcc:
-L$ROOT/usr/lib

SPECSEOF

# bmake uses MAKEOBJDIRPREFIX only for a directory that already exists,
# and builds in the source directory otherwise, so the one for this
# directory is made here. Not for a directory Lite2 marks NOOBJ: it
# installs files by relative name, which only resolve in the source
# directory, and bmake changes into the object directory before running
# anything.
case "$PWD" in
"$REPO_ROOT"|"$REPO_ROOT"/*)
	# shellcheck disable=SC2016  # ${NOOBJ} is bmake's to expand
	noobj=$(MACHINE=i386 MAKEOBJDIRPREFIX='' bmake -m "$MKDIR" \
	    -V '${NOOBJ}' 2>/dev/null || true)
	[ -n "$noobj" ] || mkdir -p "$OBJ$PWD"
	;;
esac

LC_ALL=C
MACHINE=i386
MAKEFLAGS="-m $MKDIR${MAKEFLAGS:+ $MAKEFLAGS}"
DESTDIR="$ROOT"
MAKEOBJDIRPREFIX="$OBJ"
PATH="$TOOLS:$PATH"
BINOWN=$(id -un)
BINGRP=$(id -gn)
LIBOWN="$BINOWN"
LIBGRP="$BINGRP"
LIBMODE=644
CC="gcc -m32 -std=gnu89 -fcommon -fno-stack-protector -fno-pic"
CC="$CC -static"
CC="$CC -specs=$SPECS --sysroot=$ROOT -Wl,-nostdlib"
CC="$CC -nostdinc -isystem $ROOT/usr/include"
CPP="cpp -m32 -traditional-cpp -nostdinc -I$ROOT/usr/include"
AS="as --32"
LD="ld -m elf_i386"
export LC_ALL MACHINE MAKEFLAGS DESTDIR MAKEOBJDIRPREFIX PATH
export BINOWN BINGRP LIBOWN LIBGRP LIBMODE CC CPP AS LD

exec bmake "$@"
