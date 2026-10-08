#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. 4.4BSD builds a kernel
# by running config in sys/${MACHINE}/conf and then make in the
# directory it writes, and that is what this does; what it adds is
# the target settings a Linux host needs, in the same spirit as
# build/make.sh adds them for the userland.
#
# WHAT IT DOES, for the configuration named on the command line
# (default LINK.i386) --
#
#   1. config, from ${L2_BUILD}/tools/bin, run in
#      usr/src/sys/${MACHINE}/conf. It writes the build directory at
#      ../../compile/NAME, inside the tree: the path is compiled into
#      the program as `#define CDIR "../../compile/"' with no option
#      to move it, every BSD of the period does the same, and
#      .gitignore covers it. The directory is created first, since
#      config does not create it and exits 0 when it is absent.
#
#   2. libkern.a, the kernel's own C library, which nothing else in
#      this tree builds. Its sources are usr/src/sys/libkern; the
#      Makefile links ${S}/libkern/obj/libkern.a, and that obj is one
#      of 468 symlinks shipped in the 1995 tarball pointing at
#      /usr/obj, so it has always dangled. The archive is built here
#      directly into the compile directory instead: the Makefile's
#      rule has no prerequisites and is prefixed -@, so a libkern.a
#      that already exists is left alone and the dangling symlink is
#      never followed.
#
#   3. make depend, then make, with bmake reading this tree's own
#      share/mk.
#
# THE KERNEL'S COMPILER SETTINGS ARE THIS PROJECT'S OWN. No BSD of
# this period cross-compiled a kernel, so there is no donor line for
# any of them; each is here because a build on this host needed it,
# and each is named rather than buried:
#
#   -m32, as --32, ld -m elf_i386   this is a 32-bit i386 kernel and
#       the host compiler is not.
#   -std=gnu89 -fno-zero-initialized-in-bss   as the userland carries it: GCC 14 makes an implicit
#       function declaration an error rather than a warning.
#   -fcommon   GCC has defaulted to -fno-common since 10, and this
#       kernel declares variables in headers.
#   -fno-stack-protector, -fno-pic   undo distribution GCC's
#       defaults, which a freestanding kernel cannot satisfy.
#   -ffreestanding   there is no hosted C library here.
#   -nostdinc   the host's headers must not be reachable.
#   -traditional-cpp for assembly, as NetBSD 1.5's COMPILE.S does, so
#       this tree's /**/ pasting works.
#
# The kernel Makefile sets AS, CC, CPP and LD with `=' rather than
# `?=', so they are passed on bmake's command line, where they beat a
# makefile assignment; the environment would not.
#
# MAKEOBJDIRPREFIX is cleared. build/make.sh sets it, and the kernel
# Makefile reaches its sources through ${S}, a path relative to the
# compile directory, so under an object directory it looks for
# ../../i386/i386/genassym.c from inside ${L2_BUILD} and does not
# find it. That is why this script exists rather than the kernel
# being built through make.sh.
#
# libkern is compiled with the kernel's own settings plus two include
# paths the kernel Makefile does not need: the compile directory, for
# the machine/ symlink config writes there, and the target root's
# headers, because libkern/bcmp.c includes <string.h>. Mixing kernel
# and userland include paths inside the kernel's own library is not a
# settled decision; it is what compiles, it is confined to this one
# archive, and it is said here rather than left to be rediscovered.
#
# HOST TOOLS: bmake and flex, besides GCC and binutils. The sysroot
# must have been built first -- build/sysroot.sh -- for the headers
# and for config itself.
#
# WHAT IT DOES NOT DO: it does not link a bootable image. The link
# runs and reports what is still undefined; docs/status.md records
# what those are and docs/roadmap.md what they need.

set -e

progname=$(basename "$0")
say() { printf '%s: %s\n' "$progname" "$*"; }
die() { printf '%s: %s\n' "$progname" "$*" >&2; exit 1; }

config=${1:-LINK.i386}
MACHINE=${MACHINE:-i386}

SRC=$(cd "$(dirname "$0")/.." && pwd)
case "$SRC" in
*[[:space:]]*) die "the tree's path contains whitespace" ;;
esac

L2_BUILD=${L2_BUILD:-$HOME/.cache/lite2}
case "$L2_BUILD" in
/*) ;;
*) die "L2_BUILD must be an absolute path: $L2_BUILD" ;;
esac
case "$L2_BUILD" in
"$SRC"|"$SRC"/*) die "L2_BUILD must be outside the repository: $L2_BUILD" ;;
esac
case "$L2_BUILD" in
*[[:space:]]*) die "L2_BUILD contains whitespace: $L2_BUILD" ;;
esac

ROOT=$L2_BUILD/root
TOOLS=$L2_BUILD/tools/bin
MK=$SRC/usr/src/share/mk

pkgs="bmake flex"
missing=
for t in bmake flex gcc cpp as ld ar ranlib nm; do
	command -v "$t" >/dev/null 2>&1 || missing="$missing $t"
done
[ -z "$missing" ] || {
	for t in $missing; do say "not found: $t"; done
	die "  sudo apt install $pkgs"
}
[ -x "$TOOLS/config" ] || die "no config in $TOOLS -- run build/sysroot.sh first"

# AI-ONLY NOTE: $TOOLS first, as make.sh:240 already does. config is
# called by full path below, but everything make invokes by name is
# not -- mkdep above all, which the kernel Makefile runs to work out
# what to rebuild when a header changes. Without this the host's is
# used, and on a Linux host that is OpenBSD's mkdep.gcc.sh, which
# does not produce the program dependency `assym.s: genassym' needs.
# assym.s then goes stale and locore.s assembles against constants
# the C half no longer uses.
PATH="$TOOLS:$PATH"
export PATH
[ -d "$ROOT/usr/include" ] || die "no headers in $ROOT -- run build/sysroot.sh first"

S=$SRC/usr/src/sys
CONFDIR=$S/$MACHINE/conf
[ -f "$CONFDIR/$config" ] || die "no such configuration: $CONFDIR/$config"
COMPILE=$S/compile/$config

KCC="gcc -m32 -std=gnu89 -fno-zero-initialized-in-bss -fcommon -fno-stack-protector -fno-pic"
KCC="$KCC -ffreestanding -nostdinc"
KCPP="cpp -m32 -traditional-cpp -nostdinc"

# 1. config
say "config $config -> $COMPILE"
mkdir -p "$COMPILE"
( cd "$CONFDIR" && "$TOOLS/config" "$config" >/dev/null )
[ -f "$COMPILE/Makefile" ] || die "config wrote no Makefile in $COMPILE"

# 2. libkern.a, which nothing else in this tree builds
say "libkern.a"
LK=$COMPILE/.libkern
rm -rf "$LK"
mkdir -p "$LK"
for f in "$S"/libkern/*.c; do
	b=$(basename "$f" .c)
	# mcount is the profiling counter and is built with the kernel,
	# not into this archive; Berkeley's own Makefile leaves it out.
	[ "$b" = mcount ] && continue
	$KCC -O -I"$S/libkern" -I"$COMPILE" -I"$S" -I"$S/sys" \
	    -isystem "$ROOT/usr/include" -DKERNEL -D"$MACHINE" \
	    -c "$f" -o "$LK/$b.o"
done
rm -f "$COMPILE/libkern.a"
ar cr "$COMPILE/libkern.a" "$LK"/*.o
ranlib "$COMPILE/libkern.a"
say "libkern.a: $(ar t "$COMPILE/libkern.a" | wc -l) members"

# 3. depend, then the kernel
kmake() {
	( cd "$COMPILE" && MAKEOBJDIRPREFIX='' bmake -m "$MK" \
	    CC="$KCC" CPP="$KCPP" AS="as --32" LD="ld -m elf_i386" "$@" )
}

say "make depend"
kmake depend >/dev/null

say "make"
kmake "$@" || true

objs=$(find "$COMPILE" -maxdepth 1 -name '*.o' | wc -l)
say "$objs objects in $COMPILE"
if [ -f "$COMPILE/vmunix" ]; then
	say "vmunix: $(wc -c < "$COMPILE/vmunix") bytes"
else
	say "no vmunix: the link did not complete (see docs/status.md)"
fi
