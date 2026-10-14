#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is added when it is.
#
# build/mknative-binutils.sh -- write contrib/binutils' arch/i386
# files, the config.h, defs.mk and generated headers NetBSD's reachover
# Makefiles build binutils for the target with, for this system.
#
# This is NetBSD's tools/binutils `native-binutils' target (NetBSD 5,
# tools/binutils/Makefile): GNU configure run as a "canadian cross" --
# built here, for a host and target both i386--netbsdelf -- with the
# target's own compiler, then contrib/binutils/mknative-binutils
# reading what configure wrote. The compiler is pcc with this tree's
# sysroot, from build/sysroot.sh, so configure's answers -- which
# functions libc has, which headers exist, how wide off_t is -- are
# this system's, not NetBSD 5's, which the files imported with
# contrib/binutils describe. Without NLS: this libc has no libintl.
#
# mknative-binutils writes NetBSD's layout, gnu/lib/libbfd/arch/i386
# and so on, under a top directory; this script gives it a scratch one
# and copies each file to its place in contrib/binutils. gprof is not
# imported, and its part of the script fails and is left out.
#
# Run after build/sysroot.sh, with the same L2_BUILD:
#
#	L2_BUILD=... sh build/mknative-binutils.sh
#
# then review the diff and commit it, as NetBSD does after
# `nbmake-i386 native-binutils'.

set -eu

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${L2_BUILD:?set L2_BUILD to the build directory sysroot.sh used}"
C=$REPO_ROOT/usr/src/contrib/binutils
TOOLS=$L2_BUILD/tools/bin
ROOT=$L2_BUILD/root
[ -x "$TOOLS/pcc" ] || {
	echo "mknative-binutils.sh: no pcc in $TOOLS -- run sysroot.sh" >&2
	exit 1
}

T=i386--netbsdelf
W=$L2_BUILD/tools/binutils.native
TOP=$L2_BUILD/tools/binutils.native.top
rm -rf "$W" "$TOP"
mkdir -p "$W" "$TOP/tools/gcc"
cp "$C/mknative.common" "$TOP/tools/gcc/"
# The files as they are, in NetBSD's layout, so that mknative.common's
# writefile keeps each one whose content, but for its `Generated from'
# lines, has not changed, as it does in NetBSD's tree.
(cd "$C" && find lib usr.bin -path '*/arch/i386/*' -type f) |
while read -r t; do
	case $t in
	usr.bin/*) f=gnu/usr.bin/binutils/${t#usr.bin/} ;;
	*) f=gnu/$t ;;
	esac
	mkdir -p "$TOP/$(dirname "$f")"
	cp "$C/$t" "$TOP/$f"
done

cd "$W"
CC_FOR_BUILD="cc -std=gnu89 -fcommon -w" \
CC="$TOOLS/pcc --sysroot=$ROOT -static -std=gnu89" \
AR="$TOOLS/$T-ar" RANLIB="$TOOLS/$T-ranlib" \
CFLAGS= CPPFLAGS= LDFLAGS= \
ac_cv_prog_cc_cross=yes ac_cv_func_strcoll_works=yes \
    sh "$C/dist/configure" --build="$(sh "$C/dist/config.guess")" \
    --host=$T --target=$T --disable-nls >configure.log 2>&1 ||
	{ tail configure.log >&2; exit 1; }
gmake=$(command -v gmake || command -v make)
"$gmake" configure-host >make.log 2>&1 || { tail make.log >&2; exit 1; }
(cd bfd && "$gmake" bfd.h bfdver.h) >>make.log 2>&1
(cd ld && "$gmake" ldemul-list.h) >>make.log 2>&1

# BSD make, not GNU: mknative.common's getvars reads the Makefiles
# with whichever $MAKE names, and NetBSD's files were written by its
# own make, whose quoting this matches.
MACHINE_ARCH=i386 MAKE=$(command -v bmake) \
    sh "$C/mknative-binutils" binutils "$W" "$TOP" $T 2>mknative.log ||
	true
grep -v 'changed$' mknative.log | grep -v gprof >&2 || true
grep 'changed$' mknative.log | grep -v gprof | sed 's/^/mknative-binutils.sh: /' || true

cd "$TOP/gnu"
find lib usr.bin -type f ! -path '*/gprof/*' | sort | while read -r f; do
	case $f in
	usr.bin/binutils/*) t=usr.bin/${f#usr.bin/binutils/} ;;
	*) t=$f ;;
	esac
	cmp -s "$f" "$C/$t" || cp "$f" "$C/$t"
done
