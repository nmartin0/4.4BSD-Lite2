#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
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
#   3. Host tools, in ${L2_BUILD}/tools/bin: Lite2's own lorder, from
#      usr.bin/lorder/lorder.sh, which bsd.lib.mk runs to order an
#      archive and which the host does not have. NetBSD 1.6's build.sh
#      installs its own tree's lorder the same way, as nblorder.
#
#   4. libc, by Lite2's own lib/libc all and install targets, with its
#      objects in ${L2_BUILD}/obj. Each setting answers a measured
#      failure:
#
#	CC	gcc -m32 for 32-bit i386 ELF. -fno-stack-protector and
#		-fno-pic undo distribution GCC's defaults, which leave
#		__stack_chk_fail_local and _GLOBAL_OFFSET_TABLE_
#		unresolved. -fcommon because stdio/glue.h defines
#		__sglue in a header, which without it both fwalk.o and
#		findfp.o define. -nostdinc -isystem: the root's headers,
#		not the host's.
#	CPP	for assembly. -traditional-cpp, as NetBSD 1.5 preprocesses
#		assembly, so Lite2's _/**/x and SYS_/**/x pasting works.
#		-I rather than -isystem, which puts line markers inside an
#		instruction that uses a system header's macro.
#	AS, LD	--32 and -m elf_i386.
#	NOMAN	no manual pages yet.
#	LIBOWN, LIBGRP, LIBMODE=644
#		the building user's, and writable: install makes the
#		library 444 and then runs ranlib -t on it, which writes.
#
#      Lite2's own compiler warnings remain, several hundred of them,
#      and tsort reports loops among the profiling objects.
#
#   5. The ELF startup files, crt0.o, gcrt0.o, crtbegin.o and crtend.o,
#      by lib/csu/i386_elf's own all and install targets, into
#      ${ROOT}/usr/lib. They are NetBSD 1.6's (docs/provenance/
#      csu-elf.md); their Makefile carries the flags they need.
#
# It is safe to rerun: mtree only adds what is missing, the header
# install replaces what it installed, and the libc and startup-file
# builds redo only what has changed.
#
# WHAT IT DOES NOT DO
#
#   Ownership in the root is the building user's, not root and bin.
#   That is enough to compile against; a disk image will need the real
#   ownership recorded separately.
#
#   No library but libc, and no programs yet: the startup files are
#   installed, but the compiler is not yet told to link with them.
#
# HOST TOOLS: bmake, mtree, pax and ctags, besides GCC and binutils --
#
#	sudo apt install bmake mtree-netbsd pax universal-ctags
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

pkgs="bmake mtree-netbsd pax universal-ctags"
missing=
for t in bmake mtree pax ctags gcc cpp as ld ar ranlib nm tsort; do
	command -v "$t" >/dev/null 2>&1 || missing="$missing $t"
done
if [ -n "$missing" ]; then
	printf '%s\n' "sysroot.sh: not found:$missing" >&2
	printf '%s\n' "  sudo apt install $pkgs" >&2
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

cc_i386="gcc -m32 -fcommon -fno-stack-protector -fno-pic"
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
