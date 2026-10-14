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
#   3. Host tools, in ${L2_BUILD}/tools/bin. First install, which is
#      build/install.sh and is put there before the headers are
#      installed, since every install a Makefile runs through
#      build/make.sh from here on goes through it. Then eight from this
#      tree's own sources: lorder, from usr.bin/lorder/lorder.sh,
#      which bsd.lib.mk runs to order an archive and which the host
#      does not have; mkdep, for the same reason; yacc, because the
#      packaged byacc reads these grammars but writes a yylex
#      declaration this tree's own yacc does not; config, which
#      the kernel build runs; btxld, which sys/boot/i386/boot2
#      runs to join boot2 to BTX; write_mfs_in_kernel, which puts
#      a root filesystem into an install kernel; and crunchgen and
#      crunchide, which make the one binary that root holds. And a
#      `make' that is bmake, for crunchgen. NetBSD 1.6's
#      build.sh installs its own tree's lorder the same way, as
#      nblorder, and builds its own yacc as nbyacc. Each carries its
#      reasoning at its site below.
#
#      Then the system's toolchain, binutils 2.11.2 and GCC 2.95.3
#      (contrib/), built for this machine as a cross toolchain for
#      i386-unknown-bsd4.4elf, in ${L2_BUILD}/tools, as NetBSD 1.6's
#      tools/toolchain built the same releases. Everything after it
#      is compiled with it.
#
#   4. The libraries named in $libs, by their own all and install
#      targets, in that order: libc, libutil, libterm, libcurses,
#      libedit, libl, libcompat, libm, libkvm, libtelnet and
#      librpc/rpc. Some of what each is for: libutil is named in
#      LDADD by twelve programs here; libterm builds libtermcap.a;
#      libedit also installs histedit.h; libl is the lex run-time,
#      whose sources this tree ships with no Makefile until now.
#      Each was built and checked before being added; the rest of
#      lib/ follows the same way.
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
#   Ownership in the root is the building user's when this runs
#   unprivileged: build/install.sh drops -o and -g then. Run as root,
#   or under fakeroot, it is the tree's own -- bin, kmem, operator --
#   as numbers from etc/master.passwd and etc/group.
#
#   Not the whole of lib/. Eleven libraries are built; the rest are
#   added one at a time as each is tried. Nor any program: this fills
#   the root a program is built against, and building one is a
#   separate run of build/make.sh in its own directory.
#
# HOST TOOLS: bmake, mtree, pax, ctags, flex and m4, GNU make, and a
# GCC that builds 32-bit programs, for the toolchain --
#
#	sudo apt install bmake mtree-netbsd pax universal-ctags flex m4 \
#	    make gcc-multilib
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

pkgs="bmake mtree-netbsd pax universal-ctags flex m4 make gcc-multilib"
missing=
# flex is here because config's scanner is generated from lang.l below.
# It was used without being checked for, so a host without it got past
# this point and failed two hundred lines later, inside the config
# build, with flex's own "not found" and nothing naming the package.
# m4 and objcopy are for the boot blocks: btx.s goes through m4.
for t in bmake mtree pax ctags flex m4 gcc ar ranlib nm tsort \
    objcopy make; do
	command -v "$t" >/dev/null 2>&1 || missing="$missing $t"
done
# and 32-bit support in the host's GCC, for the toolchain below.
[ -f "$(gcc -m32 -print-libgcc-file-name 2>/dev/null)" ] ||
	missing="$missing gcc-multilib"
if [ -n "$missing" ]; then
	printf '%s\n' "sysroot.sh: not found:$missing" >&2
	printf '%s\n' "  sudo apt install $pkgs" >&2
	exit 1
fi

TOOLS="$L2_BUILD/tools/bin"

# the compiler for programs this machine runs, as make.sh defines it;
# set here too because the host tools below are built without make.
HOSTCC=${HOSTCC:-"cc -std=gnu89"}

# The libraries to build, in order. Each was built and checked before
# being added; the rest of lib/ follows as each is tried.
libs="libc libutil libterm libcurses libedit libl libcompat"
libs="$libs libm libkvm libtelnet librpc/rpc"

mkdir -p "$ROOT"

printf '%s\n' "sysroot.sh: hierarchy -> $ROOT"
mtree -N "$SRC/etc" -W -def "$SRC/etc/mtree/4.4BSD.dist" -p "$ROOT" -u \
    >/dev/null

# install, before anything is installed: build/install.sh, with the
# tree's etc directory written in. make.sh puts this directory first
# on PATH, so every Makefile's install comes through it; its head says
# what it changes and why.
mkdir -p "$TOOLS"
sed "s|@ETC@|$SRC/etc|" "$REPO_ROOT/build/install.sh" > "$TOOLS/install"
chmod 755 "$TOOLS/install"

printf '%s\n' "sysroot.sh: headers -> $ROOT/usr/include"
cd "$SRC/include"
sh "$REPO_ROOT/build/make.sh" install SHARED=copies

printf '%s\n' "sysroot.sh: host tools -> $TOOLS"
mkdir -p "$TOOLS"
install -m 755 "$SRC/usr.bin/lorder/lorder.sh" "$TOOLS/lorder"

# mkdep, from this tree, for the same reason as lorder: the host's is
# not this tree's. On a Linux host /usr/bin/mkdep is OpenBSD's
# mkdep.gcc.sh, which accepts -p and emits prog.o rather than prog,
# and the tree's own had the same fault until `mkdep: -p has not
# worked since cc stopped writing `prog.o : ''.
#
# It matters because mkdep decides what gets rebuilt when a header
# changes. sys/i386/conf/Makefile.i386 has `assym.s: genassym' and
# generates genassym's dependencies with `mkdep -a -p', so that
# changing machine/pmap.h regenerates assym.s and locore.s assembles
# against the same constants the C half uses. With the wrong mkdep
# that rule names genassym.o, assym.s goes stale, and the kernel
# builds cleanly with its assembly and its C disagreeing -- measured,
# while moving KERNBASE: locore.s kept the old address and the kernel
# faulted on the jump to high memory.
#
# NetBSD builds its own for cross-building and calls it nbmkdep:
# tools/mkdep/Makefile, `HOSTPROGNAME=${_TOOL_PREFIX}mkdep' with
# `HOST_SRCDIR=usr.bin/mkdep'. Same reasoning -- when you are not
# self-hosting, every tool that shapes the output comes from the tree
# rather than the machine underneath.
install -m 755 "$SRC/usr.bin/mkdep/mkdep.sh" "$TOOLS/mkdep"

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

# btxld, for the boot blocks: sys/boot/i386/boot2's Makefile runs it
# here to join boot2 to BTX, so it is built for this machine, as
# FreeBSD builds it for the host before the boot blocks. _GNU_SOURCE
# for glibc's asprintf.
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -D_GNU_SOURCE -o "$TOOLS/btxld" "$SRC/usr.sbin/btxld/btxld.c"

# write_mfs_in_kernel, for an install kernel: it writes a filesystem
# image into a kernel built with options MFS_ROOT, here, after the link,
# as FreeBSD 2.0.5's release build did.
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -o "$TOOLS/write_mfs_in_kernel" \
    "$SRC/release/write_mfs_in_kernel.c"

# crunchgen and crunchide, for an install root: crunchgen writes a
# makefile that links many programs into one binary, and runs make to
# learn each program's objects; crunchide, run by that makefile, hides
# all but each program's entry point. Both run here. crunchide is
# built against the host's <elf.h> with build/elf-host.h, which names
# the FreeBSD aliases its ELF back end uses.
#
# make, as a name: crunchgen runs `make' to read each program's
# Makefile and its makefile runs `make' in each program's directory,
# and both mean the BSD make. So a `make' that is bmake goes here,
# first on the PATH build/make.sh gives.
C=$SRC/usr.sbin/crunch
cdir="$L2_BUILD/tools/crunch.build"
rm -rf "$cdir"
mkdir -p "$cdir"
sh -e "$C/crunchgen/mkskel.sh" "$C/crunchgen/crunched_main.c" \
    > "$cdir/crunched_skel.c"
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -o "$TOOLS/crunchgen" "$C/crunchgen/crunchgen.c" \
    "$cdir/crunched_skel.c"
# shellcheck disable=SC2086
$HOSTCC -w -include "$REPO_ROOT/build/elf-host.h" -o "$TOOLS/crunchide" \
    "$C/crunchide/crunchide.c" "$C/crunchide/exec_elf32.c"
ln -sf "$(command -v bmake)" "$TOOLS/make"

# binutils and GCC, the system's assembler, linker and C compiler
# (contrib/binutils-2.11.2 and contrib/gcc-2.95.3), built here as a
# cross toolchain for i386-unknown-bsd4.4elf, the way NetBSD 1.6's
# tools/toolchain built the same two releases for its own build: GNU
# configure with --prefix in the tools directory, no NLS, binutils
# first so that GCC is built with the target's as and ld. Each program
# is installed under the target's name, i386-unknown-bsd4.4elf-gcc and
# so on, and also, as GNU installs a cross toolchain, by its plain
# name in tools/i386-unknown-bsd4.4elf/bin, where the GCC driver looks
# for as and ld.
#
# The target's headers and libraries are where GCC and ld of this age
# look for a cross target's: tools/i386-unknown-bsd4.4elf/sys-include
# and .../lib, here symbolic links to the root's usr/include and
# usr/lib. So libgcc is compiled against this tree's headers, the
# driver finds crt0.o and the rest there, ld finds -lc there, and what
# is installed in the root later is seen without rebuilding either.
# ld's scripts, which binutils would install into that lib, go beside
# it instead; ld has them compiled in and does not read them.
#
# Built as 32-bit programs, as they were when these were current:
# GCC 2.95.3's config.sub has no x86_64, so the build and host are
# named i686-pc-linux-gnu and the host compiler given -m32. With it,
# the flags these sources of 2001 need from a compiler of today:
# -std=gnu89 (HOSTCC), -fcommon, -w; and _FORTIFY_SOURCE off, which
# Ubuntu's GCC turns on and which stops ar at its first archive: bfd's
# archive.c writes each header field with sprintf, whose terminating
# NUL runs one byte into the next field, which is written next. GNU
# make, not bmake: these are configure's Makefiles. makeinfo is not
# run, and so not the manuals' install targets either.
GT=i386-unknown-bsd4.4elf
GHOST=i686-pc-linux-gnu
GPFX="$L2_BUILD/tools"
# shellcheck disable=SC2086  # HOSTCC carries its own flags
GCC_HOSTCC="$HOSTCC -m32 -fcommon -w -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
gmake=$(command -v gmake || command -v make)
mkdir -p "$GPFX/$GT"
ln -sfn "$ROOT/usr/include" "$GPFX/$GT/sys-include"
ln -sfn "$ROOT/usr/lib" "$GPFX/$GT/lib"

B=$SRC/contrib/binutils-2.11.2
bdir="$L2_BUILD/tools/binutils-2.11.2.build"
printf '%s\n' "sysroot.sh: binutils 2.11.2 for $GT -> $GPFX"
rm -rf "$bdir"
mkdir -p "$bdir"
cd "$bdir"
CC="$GCC_HOSTCC" sh "$B/configure" --build=$GHOST --host=$GHOST \
    --target=$GT --prefix="$GPFX" --disable-nls >configure.log 2>&1 ||
	{ tail configure.log >&2; exit 1; }
"$gmake" -j"$(nproc)" MAKEINFO=true all-gas all-ld all-binutils \
    >make.log 2>&1 || { tail make.log >&2; exit 1; }
"$gmake" MAKEINFO=true scriptdir="$GPFX/$GT" \
    install-gas install-ld install-binutils \
    >install.log 2>&1 || { tail install.log >&2; exit 1; }

G=$SRC/contrib/gcc-2.95.3
gdir="$L2_BUILD/tools/gcc-2.95.3.build"
printf '%s\n' "sysroot.sh: GCC 2.95.3 for $GT -> $GPFX"
rm -rf "$gdir"
mkdir -p "$gdir"
cd "$gdir"
PATH="$GPFX/bin:$PATH" CC="$GCC_HOSTCC" sh "$G/configure" \
    --build=$GHOST --host=$GHOST --target=$GT --prefix="$GPFX" \
    --with-gnu-as --with-gnu-ld --enable-languages=c --disable-nls \
    >configure.log 2>&1 || { tail configure.log >&2; exit 1; }
PATH="$GPFX/bin:$PATH" "$gmake" -j"$(nproc)" MAKEINFO=true all-gcc \
    >make.log 2>&1 || { tail make.log >&2; exit 1; }
PATH="$GPFX/bin:$PATH" "$gmake" -C gcc MAKEINFO=true cpp \
    >>make.log 2>&1 || { tail make.log >&2; exit 1; }
(cd gcc && PATH="$GPFX/bin:$PATH" "$gmake" MAKEINFO=true \
    install-common install-headers install-libgcc install-driver) \
    >install.log 2>&1 || { tail install.log >&2; exit 1; }
# and cpp, the preprocessor as a command, which the Makefiles run as a
# filter on assembly. GCC's own install-cpp would put it in tools/bin
# as plain `cpp', ahead of the host's; it goes under the target's name.
install -m 755 gcc/cpp "$GPFX/bin/$GT-cpp"

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
