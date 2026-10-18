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
#      build/make.sh from here on goes through it. Then twelve from
#      this tree's own sources: lorder, from usr.bin/lorder/lorder.sh,
#      which bsd.lib.mk runs to order an archive and which the host
#      does not have; mkdep, for the same reason; yacc, because the
#      packaged byacc reads these grammars but writes a yylex
#      declaration this tree's own yacc does not; config, which
#      the kernel build runs; btxld, which sys/boot/i386/boot2
#      runs to join boot2 to BTX; write_mfs_in_kernel, which puts
#      a root filesystem into an install kernel; and crunchgen and
#      crunchide, which make the one binary that root holds; and zic,
#      cap_mkdb, strfile and col, which make the time zones,
#      termcap.db and vgrindefs.db, fortune's files and the whatis
#      database, as FreeBSD 2.2's build-tools built its own. And a
#      `make' that is bmake, for crunchgen. NetBSD 1.6's
#      build.sh installs its own tree's lorder the same way, as
#      nblorder, and builds its own yacc as nbyacc. Each carries its
#      reasoning at its site below.
#
#      Then the system's toolchain, binutils 2.11.2 and GCC 2.95.3
#      (contrib/), built for this machine as a cross toolchain for
#      i386-unknown-bsd4.4elf, in ${L2_BUILD}/tools, as NetBSD 1.6's
#      tools/toolchain built the same releases. Everything after it
#      is compiled with it. And groff 1.08 (contrib/), Lite2's nroff,
#      which formats the manual pages every Makefile installs, built
#      with GCC 2.95.3's g++, and the tree's own macros beside it.
#
#   4. The libraries named in $libs, by their own all and install
#      targets, in that order: libc, libutil, libterm, libcurses,
#      libedit, libl, libcompat, libm, libkvm, libtelnet,
#      librpc/rpc, libresolv and liby. Some of what each is for:
#      libutil is named in LDADD by twelve programs here; libterm
#      builds libtermcap.a; libedit also installs histedit.h; libl is
#      the lex run-time, whose sources this tree ships with no
#      Makefile until now; liby is yacc's main and yyerror. That is
#      every library lib/Makefile names but libmp and libplot, which
#      the Lite cut removed.
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
#   Not any program: this fills the root a program is built against,
#   and building one is a separate run of build/make.sh in its own
#   directory.
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
libs="$libs libm libkvm libtelnet librpc/rpc libresolv liby"

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
#
# And the compiler it runs is the target's: mkdep sets its own PATH,
# /bin:/usr/bin and the rest, and runs `cc -M', which on the system is
# the system's compiler and here would be the host's, reading the
# host's headers -- glibc's <varargs.h> and <regexp.h> refuse to be
# read at all. So the copy here puts the tools directory's
# i386-unknown-bsd4.4elf/bin, where the target's cc is, first in that
# PATH, as NetBSD's nbmkdep is given the target's compiler.
sed "s|^PATH=/bin:|PATH=$L2_BUILD/tools/i386-unknown-bsd4.4elf/bin:/bin:|" \
    "$SRC/usr.bin/mkdep/mkdep.sh" > "$TOOLS/mkdep"
chmod 755 "$TOOLS/mkdep"
grep -q "^PATH=$L2_BUILD/tools/" "$TOOLS/mkdep" ||
	{ echo "sysroot.sh: mkdep's PATH not found to set" >&2; exit 1; }

# yacc, built from this tree with the host compiler. The packaged byacc
# reads these grammars, but what it writes declares `extern int
# yylex(void);', which this tree's own yacc does not, and the kernel's
# config is built from a grammar here too. NetBSD builds its own the
# same way and calls it nbyacc.
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -o "$TOOLS/yacc" "$SRC"/usr.bin/yacc/*.c

# lex, which is flex 2.5.2 in this tree (contrib/flex-2.5.2), built
# from it for the same reason as yacc: the host's flex, 2.6, defines
# yylineno in every scanner it writes, which amd's fsinfo, written for
# a lex that did not, defines itself. Built as flex's own Makefile
# builds it -- parse.y through yacc, the scan.c it ships, its skel.c
# -- by this machine's compiler, into the build directory.
fdir="$L2_BUILD/tools/flex.build"
F=$SRC/contrib/flex-2.5.2
rm -rf "$fdir"
mkdir -p "$fdir"
(cd "$fdir" && "$TOOLS/yacc" -d "$F/parse.y" &&
    sed '/extern char.*malloc/d' y.tab.c > parse.c && mv y.tab.h parse.h)
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$HOSTCC -w -I"$fdir" -I"$F" -o "$TOOLS/lex" "$fdir/parse.c" \
    "$F/ccl.c" "$F/dfa.c" "$F/ecs.c" "$F/gen.c" "$F/main.c" "$F/misc.c" \
    "$F/nfa.c" "$F/scan.c" "$F/skel.c" "$F/sym.c" "$F/tblcmp.c" \
    "$F/yylex.c"

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

# zic, cap_mkdb, strfile and col, for the data the build makes with
# them: share/zoneinfo's zones, termcap.db and vgrindefs.db, fortune's
# .dat files, and the whatis database. Lite2's Makefiles run them on
# the machine doing the
# build -- zic and strfile as just built, which here are programs for
# the target -- so the tree's own are built for this machine and put
# first on the PATH, and the Makefiles call them by name: FreeBSD
# 2.2's build-tools (1997), which built zic, cap_mkdb and the rest
# from its own tree and installed them where its build looked first
# -- col among them -- and FreeBSD 4.0's, which added strfile. (col
# the build machine may have, but not as a part of the base system.)
#
# With -m32, as the generators' HOSTCC is, so that what they write is
# laid out as the i386 reads it: strfile's header is unsigned longs,
# and cap_mkdb's hash database is db(3)'s pages, sized and aligned as
# an i386 has them. The headers they want that glibc has not --
# <tzfile.h>, <machine/endian.h> -- come from the target root, after
# the host's own.
#
# cap_mkdb reads through libc's getcap.c and writes through db(3),
# which glibc has neither of. db is built as Lite2 ships it to be
# built elsewhere, by its own porting kit, lib/libc/db/PORT, from the
# linux directory Berkeley left there, in a copy of the db sources.
# getcap.c wants fgetln, and a second fclose of a FILE to be harmless,
# as 4.4BSD's stdio has them; build/getcap-host.c gives both.
hdir="$L2_BUILD/tools/hosttools.build"
rm -rf "$hdir"
mkdir -p "$hdir"
H32="$HOSTCC -m32 -w -idirafter $ROOT/usr/include"
Z=$SRC/share/zoneinfo
# shellcheck disable=SC2086  # HOSTCC carries its own flags
$H32 -DTM_GMTOFF=tm_gmtoff -DTM_ZONE=tm_zone -o "$TOOLS/zic" \
    "$Z/zic.c" "$Z/scheck.c" "$Z/ialloc.c"
# shellcheck disable=SC2086
$H32 -o "$TOOLS/strfile" "$SRC/games/fortune/strfile/strfile.c"
# shellcheck disable=SC2086
$H32 -o "$TOOLS/col" "$SRC/usr.bin/col/col.c"
cp -R "$SRC/lib/libc/db" "$hdir/db"
(cd "$hdir/db/PORT/linux" &&
    PATH="$TOOLS:$PATH" make -s CC="$HOSTCC -m32 -w" >make.log 2>&1) ||
	{ tail "$hdir/db/PORT/linux/make.log" >&2; exit 1; }
DBI="$hdir/db/PORT/linux/include"
# shellcheck disable=SC2086
$HOSTCC -m32 -w -D_GNU_SOURCE -I"$DBI" -Dfclose=getcap_fclose \
    -c -o "$hdir/getcap.o" "$SRC/lib/libc/gen/getcap.c"
# shellcheck disable=SC2086
$HOSTCC -m32 -w -D_GNU_SOURCE -I"$DBI" -o "$TOOLS/cap_mkdb" \
    "$SRC/usr.bin/cap_mkdb/cap_mkdb.c" "$hdir/getcap.o" \
    "$REPO_ROOT/build/getcap-host.c" "$hdir/db/PORT/linux/libdb.a"

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
# GCC with its C++, as NetBSD 1.6's tools/toolchain built it: the
# system's groff (contrib/groff-1.08) is C++, and Berkeley configured
# it to be compiled by gcc, which runs cc1plus for a .cc file; and the
# system's own GCC, built with this one, puts C++'s support -- new,
# delete and the rest -- in its libgcc, compiled by this compiler.
# cc1plus is made first, alone: libgcc2's C++ members are compiled
# with it, and GCC 2.95's Makefile does not say so, so under -j they
# can start before it exists.
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
# GCC's generated sources -- the parsers bison writes, configure -- are
# in the release beside what they are generated from, and make rebuilds
# one, into contrib/, whenever its source looks newer. Out of git that
# is a matter of checkout order, so they are given their proper order
# first by GCC's own contrib/egcs_update --touch, which exists for a
# tree checked out of version control. It changes times, not contents.
(cd "$G" && sh contrib/egcs_update --touch >/dev/null)
printf '%s\n' "sysroot.sh: GCC 2.95.3 for $GT -> $GPFX"
rm -rf "$gdir"
mkdir -p "$gdir"
cd "$gdir"
PATH="$GPFX/bin:$PATH" CC="$GCC_HOSTCC" sh "$G/configure" \
    --build=$GHOST --host=$GHOST --target=$GT --prefix="$GPFX" \
    --with-gnu-as --with-gnu-ld --enable-languages=c,c++ --disable-nls \
    >configure.log 2>&1 || { tail configure.log >&2; exit 1; }
PATH="$GPFX/bin:$PATH" "$gmake" -C gcc -j"$(nproc)" MAKEINFO=true \
    cc1plus >make.log 2>&1 || { tail make.log >&2; exit 1; }
PATH="$GPFX/bin:$PATH" "$gmake" -j"$(nproc)" MAKEINFO=true all-gcc \
    >>make.log 2>&1 || { tail make.log >&2; exit 1; }
PATH="$GPFX/bin:$PATH" "$gmake" -C gcc MAKEINFO=true cpp \
    >>make.log 2>&1 || { tail make.log >&2; exit 1; }
(cd gcc && PATH="$GPFX/bin:$PATH" "$gmake" MAKEINFO=true \
    install-common install-headers install-libgcc install-driver) \
    >install.log 2>&1 || { tail install.log >&2; exit 1; }
# and C++'s own headers, exception, new, new.h and typeinfo, which
# install-headers leaves to a step not named here: that step would
# also put GCC's float.h and limits.h ahead of the tree's for C.
for f in exception new new.h typeinfo; do
	install -m 644 gcc/include/$f \
	    "$GPFX/lib/gcc-lib/$GT/2.95.3/include/$f"
done
# and cpp, the preprocessor as a command, which the Makefiles run as a
# filter on assembly. GCC's own install-cpp would put it in tools/bin
# as plain `cpp', ahead of the host's; it goes under the target's name.
install -m 755 gcc/cpp "$GPFX/bin/$GT-cpp"
# and cc, the name Lite2's own scripts give the compiler -- mkdep runs
# `cc -M' -- beside gcc, as, ld and the rest in the tools directory's
# $GT/bin, which mkdep's PATH starts with (above).
ln -sf gcc "$GPFX/$GT/bin/cc"

# groff, for the manual pages: Lite2's Makefiles format each page with
# `nroff -man' into the .0 file installed under /usr/share/man/cat*,
# and Lite2's nroff is groff 1.08's (contrib/groff-1.08), so that is
# built here for this machine, as NetBSD 1.6's tools/groff builds its
# tree's groff for its build.
#
# groff 1.08 is C++ of 1993, and no C++ compiler of today takes it:
# it uses a loop variable after its `for', as C++ allowed until 1995,
# and declares functions with no return type. GCC 2.95.3's g++ does,
# given -fno-for-scope and -fpermissive, so the tree's own GCC is
# built a second time, as a C++ compiler for this machine -- an
# i686-pc-linux-gnu compiler, with the host's assembler and linker
# told --32 -- and groff compiled with it, as Lite2 compiled groff
# with its GCC. Only its compiler proper and libgcc are built; groff
# uses none of libstdc++. It links against the host's 32-bit C
# library, wherever the host GCC finds it.
HG=$SRC/contrib/gcc-2.95.3
hgdir="$L2_BUILD/tools/gcc-2.95.3-host.build"
printf '%s\n' "sysroot.sh: GCC 2.95.3's g++ for this machine, for groff"
rm -rf "$hgdir"
mkdir -p "$hgdir/w"
printf '#!/bin/sh\nexec as --32 "$@"\n' > "$hgdir/w/as"
printf '#!/bin/sh\nexec ld -m elf_i386 "$@"\n' > "$hgdir/w/ld"
chmod 755 "$hgdir/w/as" "$hgdir/w/ld"
cd "$hgdir"
CC="$GCC_HOSTCC" sh "$HG/configure" --build=$GHOST --host=$GHOST \
    --target=$GHOST --with-as="$hgdir/w/as" --with-ld="$hgdir/w/ld" \
    --enable-languages=c++ --disable-nls >configure.log 2>&1 ||
	{ tail configure.log >&2; exit 1; }
# One job at a time: libgcc2's C++ members (new.o and the rest) are
# compiled with cc1plus, and GCC 2.95's Makefile does not say so, so
# under -j they can start before cc1plus exists ("installation
# problem, cannot exec `cc1plus'"), as a release build here once did.
"$gmake" MAKEINFO=true all-gcc >make.log 2>&1 ||
	{ tail make.log >&2; exit 1; }
lib32=$(dirname "$(gcc -m32 -print-file-name=crt1.o)")
HOSTCXX="$hgdir/gcc/xgcc -B$hgdir/gcc/ -B$lib32/ -L$lib32"
HOSTCXX="$HOSTCXX -w -fno-for-scope -fpermissive"

# groff itself, by its own configure and Makefiles, in a copy of its
# directory: groff 1.08 configures and builds in its source directory,
# and this tree's copy is configured for the target. Only what nroff
# needs: troff, grotty and the groff driver, with tbl, eqn and soelim,
# which pages are sometimes run through, the ascii and latin1 devices,
# and the macro packages; and grops and its devps, for the one paper
# the build itself formats: contrib/mh-6.8.3a's doc/makefile, which
# MH's own `make all' runs, makes paper.ps with bsd.doc.mk's groff, as
# it did on Berkeley's machine (share/doc's papers are installed as
# sources, and not formatted). _XOPEN_SOURCE=500 keeps glibc from declaring
# strsignal, which groff 1.08 defines itself, as a libc of its day did
# not.
GR=$SRC/contrib/groff-1.08
grdir="$L2_BUILD/tools/groff-1.08.build"
printf '%s\n' "sysroot.sh: groff 1.08 for this machine -> $GPFX"
rm -rf "$grdir"
cp -R "$GR" "$grdir"
cd "$grdir"
grsub='LIBDIRS=libgroff libdriver'
grsub2='CCPROGDIRS=groff troff tbl eqn grotty grops soelim'
CC="$HOSTCC -m32 -w" CCC="$HOSTCXX" sh ./configure --prefix="$GPFX" \
    >configure.log 2>&1 || { tail configure.log >&2; exit 1; }
mkdir -p "$GPFX/bin" "$GPFX/share/tmac" "$GPFX/share/groff_font" \
    "$GPFX/share/man/cat1" "$GPFX/share/man/cat5" "$GPFX/share/man/cat7"
for t in all install; do
	"$gmake" CCC="$HOSTCXX -D_XOPEN_SOURCE=500" YACC=byacc \
	    "$grsub" "$grsub2" CPROGDIRS= DEVDIRS="devascii devlatin1 devps" \
	    OTHERDIRS="tmac nroff" $t >>make.log 2>&1 ||
		{ tail make.log >&2; exit 1; }
done
# and the tree's own macros, as share/tmac's Makefile installs them:
# -mdoc's files through its strip.sed, and tmac.andoc, which Lite2's
# `nroff -man' reads as tmac.an and which picks -mdoc or groff's -man
# by what the page is written in; and tmac.e and tmac.s. (The
# Makefile names a tmac.r too, which Lite2 does not have.) They name
# /usr/share/tmac, the system's macro directory, in their .so
# requests; here that is the tools directory's.
TM=$SRC/share/tmac
GTM="$GPFX/share/tmac"
for f in doc doc-ditroff doc-common doc-nroff doc-syms; do
	sed -f "$TM/strip.sed" "$TM/$f" | sed "s|/usr/share/tmac/|$GTM/|" \
	    > "$GTM/tmac.$f"
done
sed "s|/usr/share/tmac/|$GTM/|" "$TM/tmac.andoc" > "$GTM/tmac.andoc"
cp "$GTM/tmac.andoc" "$GTM/tmac.an"
for f in tmac.e tmac.s; do
	cp "$TM/$f" "$GTM/$f"
done

for lib in $libs; do
	printf '%s\n' "sysroot.sh: $lib -> $ROOT/usr/lib"
	cd "$SRC/lib/$lib"
	# depend first: Lite2's libraries hang generated headers off the
	# .depend target -- libedit makes six that way -- and without it
	# make compiles before they exist.
	sh "$REPO_ROOT/build/make.sh" depend all install
done

printf '%s\n' "sysroot.sh: startup files -> $ROOT/usr/lib"
cd "$SRC/lib/csu/i386_elf"
sh "$REPO_ROOT/build/make.sh" all install NOMAN=noman
