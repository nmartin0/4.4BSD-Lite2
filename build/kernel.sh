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
# (default GENERIC.i386) --
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
# THE COMPILER is the system's, GCC 2.95.3 and binutils 2.11.2, as
# build/sysroot.sh builds them and build/make.sh gives them to the
# userland. Of the kernel's settings two are this project's, because
# the kernel is built against no C library and no installed headers:
#
#   -ffreestanding   there is no hosted C library here.
#   -nostdinc   the kernel's headers are its own, under ${S}, and the
#       target root's must not be reached by default.
#
# Assembly goes through the compiler's cpp, traditional, as the
# userland's does, so this tree's /**/ pasting works.
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
# HOST TOOLS: bmake and flex, and the host's ar, ranlib and nm. The
# sysroot must have been built first -- build/sysroot.sh -- for the
# headers, the toolchain and config itself.
#
# WHAT IT DOES NOT DO: it does not link a bootable image. The link
# runs and reports what is still undefined; docs/status.md records
# what those are and docs/roadmap.md what they need.

set -e

progname=$(basename "$0")
say() { printf '%s: %s\n' "$progname" "$*"; }
die() { printf '%s: %s\n' "$progname" "$*" >&2; exit 1; }

# AI-ONLY NOTE: the configuration name is consumed here, and anything
# after it is passed to make as a target. Without the shift, $@ still
# held the configuration name at the make below, so the documented
# invocation -- `sh build/kernel.sh LINK.i386', docs/status.md -- asked
# bmake to build a target called LINK.i386 and got `don't know how to
# make LINK.i386'. That was harmless only while the make's status was
# discarded; the moment it stopped being discarded it deleted the
# kernel and failed.
config=${1:-GENERIC.i386}
[ $# -eq 0 ] || shift
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
for t in bmake flex ar ranlib nm; do
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
# did not produce the dependency assym.s's generator needs. assym.s
# then goes stale and locore.s assembles against constants the C half
# no longer uses.
PATH="$TOOLS:$PATH"
export PATH
[ -d "$ROOT/usr/include" ] || die "no headers in $ROOT -- run build/sysroot.sh first"

S=$SRC/usr/src/sys
CONFDIR=$S/$MACHINE/conf
[ -f "$CONFDIR/$config" ] || die "no such configuration: $CONFDIR/$config"
COMPILE=$S/compile/$config

GT=i386-unknown-bsd4.4elf
[ -x "$TOOLS/$GT-gcc" ] || die "no $GT-gcc in $TOOLS -- run build/sysroot.sh first"
KCC="$TOOLS/$GT-gcc -ffreestanding -nostdinc"
KCPP="$TOOLS/$GT-cpp -traditional -nostdinc"
KAS="$TOOLS/$GT-as"
KLD="$TOOLS/$GT-ld"
# and its nm, which kern/genassym.sh reads genassym.o with.
KNM="$TOOLS/$GT-nm"

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
	    CC="$KCC" CPP="$KCPP" AS="$KAS" LD="$KLD" NM="$KNM" "$@" )
}

# AI-ONLY NOTE: the old kernel goes before anything can produce an
# object, and make's status is reported rather than discarded.
#
# The make below used to read `kmake "$@" || true', so that a failed
# build still reached the object count that follows -- useful while
# the link did not complete at all. The cost was found the hard way: a
# kernel build failed, the script said nothing, the report described
# the vmunix left over from an earlier build, and the next QEMU run
# spent hours silent on a kernel linked at the KERNBASE this tree used
# before it moved -- loading to a physical address past the end of RAM
# and jumping into nothing.
#
# So: remove vmunix first, so a failed build can never present an old
# kernel as a new one, and exit non-zero when make fails. Losing a
# working kernel on a failed build is the lesser harm; booting the
# wrong one costs an afternoon and lies about what it is.
#
# It goes here and not next to the make, because `set -e' is in force
# and a failure in `make depend' would otherwise exit with the stale
# kernel still in place -- the very case this is for. Everything
# before this point either checks a prerequisite or rewrites the
# Makefile and cannot touch an object.
rm -f "$COMPILE/vmunix"

# The objects of one compiler are up to date to make after a switch to
# the other, so a kernel would be linked from the first one's. The
# compiler and assembler are recorded beside them, and a change
# empties the directory of objects first.
#
# The same holds for the configuration's options. config writes them
# into the Makefile as -D flags on its IDENT and PARAM lines, and
# make, which sees no file change, keeps every object compiled
# without a new one: with `options NFS' and FIFO added, nfs/*.o were
# made, but init_sysent.o kept nfssvc as nosys and ufs_vnops.o kept
# no FIFO, and the kernel said "NFS not available". 4.4BSD's answer
# was `make clean' after a change of options; NetBSD and FreeBSD
# later had config write the options into headers (opt_*.h) that
# make depend can follow. Here the two lines are recorded with the
# compiler, and a change empties the objects as a compiler's does.
stamp="$KCC $KAS
$(grep '^IDENT=\|^PARAM=' "$COMPILE/Makefile")"
if [ "$(cat "$COMPILE/.kcc" 2>/dev/null)" != "$stamp" ]; then
	rm -f "$COMPILE"/*.o "$COMPILE/assym.s"
	printf '%s\n' "$stamp" > "$COMPILE/.kcc"
fi

say "make depend"
kmake depend >/dev/null

say "make"
rc=0
kmake "$@" || rc=$?

objs=$(find "$COMPILE" -maxdepth 1 -name '*.o' | wc -l)
say "$objs objects in $COMPILE"
if [ -f "$COMPILE/vmunix" ]; then
	say "vmunix: $(wc -c < "$COMPILE/vmunix") bytes"
else
	say "no vmunix: the link did not complete (see docs/status.md)"
fi
[ "$rc" -eq 0 ] || die "make failed, status $rc"

# A common symbol must not have become code. The kernel is compiled
# -fcommon, as its headers need: sys/buf.h, vm/vm_object.h and a
# hundred others declare `int nbuf;' and the like in every file that
# includes them, and the linker merges them. It merges a common with a
# function of the same name just as quietly, and that is how
# pmap.c's `struct pte *mmap' became the mmap system call's first four
# bytes (docs/provenance/install.md, fault 10). So every symbol that is
# common in some object is looked up in vmunix, and a text symbol there
# stops the build. -fno-common would make the linker refuse it, at the
# cost of an extern in each of those headers; docs/roadmap.md records
# that as not done.
if [ -f "$COMPILE/vmunix" ]; then
	(cd "$COMPILE" && for o in *.o; do nm "$o"; done) |
	    awk '$2 == "C" { print $3 }' | sort -u > "$COMPILE/.commons"
	nm "$COMPILE/vmunix" | awk '$2 == "T" || $2 == "t" { print $3 }' |
	    sort -u > "$COMPILE/.text"
	bad=$(comm -12 "$COMPILE/.commons" "$COMPILE/.text" | tr '\n' ' ')
	rm -f "$COMPILE/.commons" "$COMPILE/.text"
	if [ -n "$bad" ]; then
		rm -f "$COMPILE/vmunix"	# as above: no kernel, not a wrong one
		die "common symbol resolved to code: $bad"
	fi
	say "no common symbol resolved to code"
fi
