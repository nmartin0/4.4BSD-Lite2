#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. The staging follows
# NetBSD's build.sh, which fills DESTDIR, and OpenBSD's
# Makefile.cross, whose cross-includes fills its target root with the
# hierarchy and headers before anything is built. The mtree options
# come from NetBSD: -N from NetBSD 3's etc/Makefile, -W from NetBSD
# 1.6's unprivileged builds.
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
#      points into /sys, which on Linux is sysfs.
#
#      One message is expected: "install: cannot stat 'mp.h'". FILES
#      lists the header of libmp, which Lite2 removed.
#
#   3. Host tools, in ${L2_BUILD}/tools/bin: Lite2's own lorder, from
#      usr.bin/lorder/lorder.sh, which bsd.lib.mk runs to order an
#      archive and which the host does not have. NetBSD 1.6's build.sh
#      installs its own tree's lorder the same way, as nblorder.
#
#   4. The libraries named in $libs, by their own all and install
#      targets, in that order: libc; libutil, which twelve programs
#      here name in LDADD; libterm, which builds libtermcap.a;
#      libedit, which also installs histedit.h; and libl, the lex
#      run-time, which this tree ships the sources for but has no
#      Makefile for until now. Each was built and checked before
#      being added; the rest of lib/ follows the same way.
#
#      The compiler, assembler and linker settings, the object
#      directory, the target root and the install owner all come from
#      build/make.sh, which explains each of them.
#
#      Lite2's own compiler warnings remain, several hundred of them,
#      and tsort reports loops among the profiling objects.
#
#   5. The ELF startup files, crt0.o, gcrt0.o, crtbegin.o and crtend.o,
#      by lib/csu/i386_elf's own all and install targets, into
#      ${ROOT}/usr/lib. They are NetBSD 1.6's (docs/provenance/
#      csu-elf.md); their Makefile carries the flags they need.
#
#      After this a program can be built: run build/make.sh in its
#      directory.
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

TOOLS="$L2_BUILD/tools/bin"

# the compiler for programs this machine runs, as make.sh defines it;
# set here too because the host tools below are built without make.
HOSTCC=${HOSTCC:-"cc -std=gnu89"}

# The libraries to build, in order. These two have been built and
# checked; the rest of lib/ follows as each is tried.
libs="libc libutil libterm libcurses libedit libl libcompat"
libs="$libs libm libkvm libtelnet librpc/rpc"

mkdir -p "$ROOT"

printf '%s\n' "sysroot.sh: hierarchy -> $ROOT"
mtree -N "$SRC/etc" -W -def "$SRC/etc/mtree/4.4BSD.dist" -p "$ROOT" -u \
    >/dev/null

printf '%s\n' "sysroot.sh: headers -> $ROOT/usr/include"
cd "$SRC/include"
sh "$REPO_ROOT/build/make.sh" install SHARED=copies

printf '%s\n' "sysroot.sh: host tools -> $TOOLS"
mkdir -p "$TOOLS"
install -m 755 "$SRC/usr.bin/lorder/lorder.sh" "$TOOLS/lorder"

# yacc, built from this tree with the host compiler. The packaged byacc
# reads these grammars, but what it writes declares `extern int
# yylex(void);', which this tree's own yacc does not, and the kernel's
# config is built from a grammar here too. NetBSD builds its own the
# same way and calls it nbyacc.
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -o "$TOOLS/yacc" "$SRC"/usr.bin/yacc/*.c

# config, for the kernel: the build runs it, so it is built for this
# machine, with the yacc above and the host lex. Three things the host
# needs and 4.4BSD did not: -fcommon, because config.h declares
# variables without extern and GCC has defaulted to -fno-common since
# 10; sys/sysmacros.h, because glibc moved major, minor and makedev out
# of <sys/types.h>; and --noyywrap, since lex's yywrap is in -ll, which
# is a target library here.
cdir="$L2_BUILD/tools/config.build"
rm -rf "$cdir"
mkdir -p "$cdir"
cd "$cdir"
"$TOOLS/yacc" -d "$SRC/usr.sbin/config/config.y"
mv y.tab.c config.c
flex --noyywrap -t "$SRC/usr.sbin/config/lang.l" > lang.c
C=$SRC/usr.sbin/config
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -fcommon -include sys/sysmacros.h -I. -I"$C" \
    -o "$TOOLS/config" config.c lang.c "$C/main.c" "$C/mkioconf.c" \
    "$C/mkmakefile.c" "$C/mkglue.c" "$C/mkheaders.c" "$C/mkswapconf.c"

for lib in $libs; do
	printf '%s\n' "sysroot.sh: $lib -> $ROOT/usr/lib"
	cd "$SRC/lib/$lib"
	# depend first: Lite2's libraries hang generated headers off the
	# .depend target -- libedit makes six that way -- and without it
	# make compiles before they exist.
	sh "$REPO_ROOT/build/make.sh" depend all install NOMAN=noman
done

printf '%s\n' "sysroot.sh: startup files -> $ROOT/usr/lib"
cd "$SRC/lib/csu/i386_elf"
sh "$REPO_ROOT/build/make.sh" all install NOMAN=noman
