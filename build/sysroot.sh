#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out of the file rather than written into a sentence, where a
# scanner would read it as a declaration. 4.4BSD-Lite2's terms
# (./COPYRIGHT) cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. The staging follows
# NetBSD's build.sh, which fills DESTDIR, and OpenBSD's
# Makefile.cross, whose cross-includes fills ${DESTDIR}/usr/cross/
# ${TARGET} with the hierarchy and headers before anything is built.
# The mtree options come from NetBSD: -N from NetBSD 3's etc/Makefile,
# -W from NetBSD 1.6's unprivileged builds.
#
# sysroot.sh -- build the target root that Lite2's build reads from.
#
#	sh build/sysroot.sh
#
# 4.4BSD-Lite2 builds natively: it reads headers from /usr/include and
# libraries from /usr/lib of the running system, and its top-level
# `build' target installs onto /. On a Linux host those are the host's,
# so this makes a separate target root and installs into it, the way
# NetBSD's build.sh fills DESTDIR.
#
# NOTHING IS WRITTEN INSIDE THE REPOSITORY. The root is
# ${L2_BUILD}/root. L2_BUILD defaults to ~/.cache/lite2 and must be an
# absolute path outside the repository.
#
# WHAT IT DOES, in Lite2's own order
#
#   1. The directory hierarchy, from Lite2's etc/mtree/4.4BSD.dist, as
#      etc/Makefile's distribution target makes it. -N resolves names
#      such as wheel against Lite2's own etc/group and master.passwd,
#      which the host does not have (NetBSD 3's etc/Makefile passes -N
#      the same way). -W leaves ownership and modes alone, which an
#      unprivileged user cannot set (NetBSD 1.6's unprivileged builds
#      pass -W).
#
#   2. The headers, by Lite2's own include/Makefile install target, run
#      through build/make.sh with SHARED=copies. The default, symlinks,
#      points into /sys, which on Linux is sysfs. BINOWN and BINGRP are
#      the building user's: install -o bin fails for anyone else, and
#      Lite2's install loops ignore the failure.
#
#      One message is expected: "install: cannot stat 'mp.h'". FILES
#      lists the header of libmp, which Lite2 removed.
#
#   3. Host tools, in ${L2_BUILD}/tools/bin: this tree's own lorder,
#      from usr.bin/lorder/lorder.sh, which bsd.lib.mk runs to order an
#      archive and which the host has not got. NetBSD's build.sh
#      installs its own tree's lorder the same way, as nblorder.
#
#   4. libc, by Lite2's own lib/libc all and install targets, with its
#      objects in ${L2_BUILD}/obj. The settings it is given, each
#      answering a failure measured on this tree:
#
#	CC	gcc -m32 for 32-bit i386. -std=gnu89 because GCC 14
#		treats an implicit function declaration as an error,
#		and 14 of 40 sampled libc sources use one -- sleep.c
#		calls sigvec, which <sys/signal.h> does not declare.
#		-fno-stack-protector and -fno-pic undo distribution
#		GCC's defaults, which leave __stack_chk_fail_local and
#		_GLOBAL_OFFSET_TABLE_ unresolved. -fcommon because
#		stdio/glue.h defines __sglue in a header, which without
#		it both fwalk.o and findfp.o define. -nostdinc -isystem
#		for the root's headers rather than the host's.
#	CPP	for assembly: -traditional-cpp, as NetBSD 1.5's
#		COMPILE.S preprocesses it, so Lite2's /**/ pasting
#		works; and -I rather than -isystem, which puts a line
#		marker inside an instruction that uses a system
#		header's macro.
#	AS, LD	--32 and -m elf_i386.
#	LIBMODE	644: install makes a library 444 and then runs
#		ranlib -t on it, which writes.
#
#   5. The ELF startup files, crt0.o, gcrt0.o, crtbegin.o and crtend.o,
#      by lib/csu/i386_elf's own all and install targets, into
#      ${ROOT}/usr/lib. They are NetBSD 1.6's (docs/provenance/
#      csu-elf.md); the flags they need are in their own Makefile, so
#      only the compiler, assembler and linker are passed here.
#
# It is safe to rerun: mtree only adds what is missing, and the header
# install replaces what it installed.
#
# WHAT IT DOES NOT DO
#
#   Ownership in the root is the building user's, not root and bin.
#   That is enough to compile against; a disk image will need the real
#   ownership recorded separately.
#
#   No library but libc, and no programs yet: the startup files are
#   installed, but nothing tells the compiler to link with them.
#
# HOST TOOLS: bmake, mtree and pax --
#
#	sudo apt install bmake mtree-netbsd pax
#
set -e

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC="$REPO_ROOT/usr/src"

: "${L2_BUILD:=${HOME}/.cache/lite2}"
case "$L2_BUILD" in
/*)
	;;
*)
	printf '%s\n' "sysroot.sh: L2_BUILD must be an absolute path" >&2
	exit 1
	;;
esac
case "$L2_BUILD" in
"$REPO_ROOT"|"$REPO_ROOT"/*)
	printf '%s\n' "sysroot.sh: L2_BUILD must be outside the repository" >&2
	exit 1
	;;
esac
# DESTDIR reaches Lite2's recipes unquoted, so a space would split it.
case "$L2_BUILD" in
*[[:space:]]*)
	printf '%s\n' "sysroot.sh: L2_BUILD contains whitespace" >&2
	exit 1
	;;
esac
ROOT="$L2_BUILD/root"

missing=
for t in bmake mtree pax; do
	command -v "$t" >/dev/null 2>&1 || missing="$missing $t"
done
if [ -n "$missing" ]; then
	printf '%s\n' "sysroot.sh: not found:$missing" >&2
	printf '%s\n' "  sudo apt install bmake mtree-netbsd pax" >&2
	exit 1
fi

me=$(id -un)
grp=$(id -gn)
TOOLS="$L2_BUILD/tools/bin"
OBJ="$L2_BUILD/obj"

mkdir -p "$ROOT"

printf '%s\n' "sysroot.sh: hierarchy -> $ROOT"
mtree -N "$SRC/etc" -W -def "$SRC/etc/mtree/4.4BSD.dist" -p "$ROOT" -u \
    >/dev/null

printf '%s\n' "sysroot.sh: headers -> $ROOT/usr/include"
cd "$SRC/include"
sh "$REPO_ROOT/build/make.sh" install SHARED=copies DESTDIR="$ROOT" \
    BINOWN="$me" BINGRP="$grp"

printf '%s\n' "sysroot.sh: host tools -> $TOOLS"
mkdir -p "$TOOLS"
install -m 755 "$SRC/usr.bin/lorder/lorder.sh" "$TOOLS/lorder"

cc_i386="gcc -m32 -std=gnu89 -fcommon -fno-stack-protector -fno-pic"
cc_i386="$cc_i386 -nostdinc -isystem $ROOT/usr/include"
cpp_i386="cpp -m32 -traditional-cpp -nostdinc -I$ROOT/usr/include"

printf '%s\n' "sysroot.sh: libc -> $ROOT/usr/lib"
mkdir -p "$OBJ$SRC/lib/libc"
cd "$SRC/lib/libc"
PATH="$TOOLS:$PATH" MAKEOBJDIRPREFIX="$OBJ" \
    sh "$REPO_ROOT/build/make.sh" all install NOMAN=noman \
    DESTDIR="$ROOT" BINOWN="$me" BINGRP="$grp" \
    LIBOWN="$me" LIBGRP="$grp" LIBMODE=644 \
    CC="$cc_i386" CPP="$cpp_i386" AS="as --32" LD="ld -m elf_i386"

printf '%s\n' "sysroot.sh: startup files -> $ROOT/usr/lib"
mkdir -p "$OBJ$SRC/lib/csu/i386_elf"
cd "$SRC/lib/csu/i386_elf"
MAKEOBJDIRPREFIX="$OBJ" \
    sh "$REPO_ROOT/build/make.sh" all install NOMAN=noman \
    DESTDIR="$ROOT" BINOWN="$me" BINGRP="$grp" \
    CC="$cc_i386" AS="as --32" LD="ld -m elf_i386"
