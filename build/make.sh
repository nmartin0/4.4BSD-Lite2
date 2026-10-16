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
#		Before it, -m ${L2_BUILD}/tools/mk, for a sys.mk that
#		reads this tree's after one line of its own: the shell
#		as Lite2's make runs it. That make runs every command
#		with `sh -ec' (usr.bin/make/compat.c), so that a command
#		that fails stops the make, even inside a loop such as
#		bsd.subdir.mk's over SUBDIR; bmake runs `sh -c', and
#		such a failure passes unseen, unless a .SHELL line asks
#		for -e, which this one does.
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
#		sysroot.sh installs there: lorder, mkdep, yacc, config,
#		and install, build/install.sh, which turns the owners and
#		groups the Makefiles name into this tree's numbers when
#		run as root and drops them otherwise.
#   LIBMODE	libraries writable: install makes them 444 and then
#		runs ranlib -t, which writes.
#
#   BINOWN, BINGRP, LIBOWN, LIBGRP
#		unprivileged, the building user, as before: a few
#		Makefiles run chown ${BINOWN}.${BINGRP} directly, not
#		through install -- include/Makefile's header loops, and
#		lib/librpc/rpc's without a `-' -- and only the building
#		user can be given there. As root, or under fakeroot,
#		the numbers of this tree's bin user and group, which is
#		what bsd.prog.mk and bsd.lib.mk default them to by
#		name: a number means the same to the host's chown as
#		to build/install.sh. A Makefile's own BINGRP= kmem
#		still overrides them, and the wrapper turns it into
#		this tree's kmem. Names a Makefile writes into a chown
#		of its own -- games.bin, in a few places -- are still
#		the host's.
#   HOSTCC	cc for programs the build runs here rather than on the
#		target: bin/sh compiles mkinit, mknodes and mksyntax and
#		runs them to generate source.
#   CC, CPP, AS, LD
#		the system's compiler, preprocessor, assembler and
#		linker: GCC 2.95.3 and binutils 2.11.2, from
#		contrib/gcc-2.95.3 and contrib/binutils-2.11.2, which
#		build/sysroot.sh builds for this machine as a cross
#		toolchain for i386-unknown-bsd4.4elf. Lite2 defaults all
#		four with ?=, so these take effect and a command-line
#		assignment wins.
#
# THE COMPILER
#
# Lite2's sys.mk says CC ?= gcc, and this is that, built here to make
# the target's code. It needs no flags: it makes i386 ELF, puts
# uninitialised variables in common, and links statically, and it
# finds the target's headers, startup files and libraries in the root
# by itself, through the sys-include and lib that sysroot.sh points
# there. Assembly goes through its cpp, traditional, as Lite2's
# /usr/bin/cpp ran GCC's (usr.bin/cpp/cpp.sh) -- for the /**/ pasting
# Lite2's assembly does -- and with the names GCC predefines for the
# target: __ELF__, which lib/libc/i386/SYS.h tests, among them.
#
# WHAT IT DOES NOT DO
#
#   No guard against a SUBDIR entry naming a directory that is not
#   here; share/mk ends such an entry, and 72 of them are reported and
#   skipped.
#
# HOST TOOLS: bmake, and the toolchain build/sysroot.sh builds --
#
#	sudo apt install bmake
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

ROOT="$L2_BUILD/root"
OBJ="$L2_BUILD/obj"
TOOLS="$L2_BUILD/tools/bin"

mkdir -p "$L2_BUILD"

# bmake uses MAKEOBJDIRPREFIX only for a directory that already exists,
# and builds in the source directory otherwise, so object directories
# are made here: for this directory and every one below it with a
# Makefile, since a recursive target runs make in each. Except:
#
#   not where the Makefile sets NOOBJ: it installs files by relative
#   name, which only resolve in the source directory, and bmake
#   changes into the object directory before running anything;
#
#   not for a document, whose Makefile includes bsd.doc.mk: that obj
#   target does nothing, and these install their sources by relative
#   name too;
#
#   and none below a Makefile with an obj target of its own, which
#   stops Lite2's `make obj' going further: contrib/'s, which does
#   nothing, for programs that build in their own directories.
#
# A directory that only recurses had none under Lite2's `make obj';
# here it has one, as the parent of its subdirectories' -- bmake uses
# a directory that exists -- so the few such Makefiles that install
# files of their own name them from ${.CURDIR}, as Lite2's
# games/fortune/datfiles does (share/doc/usd and the like).
#
# Grepping for these is enough here, and costs one pass rather than a
# make per directory.
case "$PWD" in
"$REPO_ROOT"|"$REPO_ROOT"/*)
	objre='^([A-Za-z_.-]+[[:space:]]+)*obj([[:space:]]+[A-Za-z_.-]+)*[[:space:]]*:'
	# None at all below an obj target above this directory.
	above=
	d=$PWD
	while [ "${d#"$SRC"/}" != "$d" ]; do
		d=${d%/*}
		if [ -f "$d/Makefile" ] && grep -Eq "$objre" "$d/Makefile"; then
			above=$d
		fi
	done
	if [ -z "$above" ]; then
		# the directories whose own obj target stops `make obj'
		stops=$(find . -name Makefile -type f -exec grep -El "$objre" \
		    {} + 2>/dev/null | sed 's|/Makefile$||')
		find . -name Makefile -type f 2>/dev/null |
		while read -r mf; do
			dir=${mf%/Makefile}
			skip=
			for t in $stops; do
				case "$dir" in
				"$t"|"$t"/*) skip=1 ;;
				esac
			done
			[ -z "$skip" ] || continue
			grep -q '^NOOBJ' "$mf" && continue
			grep -q 'bsd\.doc\.mk' "$mf" && continue
			mkdir -p "$OBJ$PWD/${dir#./}"
		done
	fi
	;;
esac

LC_ALL=C
MACHINE=i386
# the shell as Lite2's make runs it; see MAKEFLAGS above.
mkdir -p "$L2_BUILD/tools/mk"
cat > "$L2_BUILD/tools/mk/sys.mk" <<EOF
# Written by build/make.sh: commands run with sh -e, as Lite2's
# make runs them, and then this tree's own sys.mk.
.SHELL: name=sh path=/bin/sh hasErrCtl=yes check="set -e" \\
    ignore="set +e" echo="set -v" quiet="set +v" filter="set +v" \\
    echoFlag=v errFlag=e newline="\\n"
.include "$MKDIR/sys.mk"
EOF
MAKEFLAGS="-m $L2_BUILD/tools/mk -m $MKDIR${MAKEFLAGS:+ $MAKEFLAGS}"
DESTDIR="$ROOT"
MAKEOBJDIRPREFIX="$OBJ"
# the host's own cc, by its full path, for HOSTCC below.
hostcc=$(command -v cc)
GT=i386-unknown-bsd4.4elf
PATH="$TOOLS:$PATH"
if [ "$(id -u)" -ne 0 ]; then
	BINOWN=$(id -un)
	BINGRP=$(id -gn)
else
	BINOWN=$(awk -F: '$1 == "bin" { print $3 }' "$SRC/etc/master.passwd")
	BINGRP=$(awk -F: '$1 == "bin" { print $3 }' "$SRC/etc/group")
fi
LIBOWN="$BINOWN"
LIBGRP="$BINGRP"
export BINOWN BINGRP LIBOWN LIBGRP
LIBMODE=644
CC="$TOOLS/$GT-gcc"
CPP="$TOOLS/$GT-cpp -traditional"
AS="$TOOLS/$GT-as"
LD="$TOOLS/$GT-ld"
# The few programs a build runs on this machine rather than the
# target: -std=gnu89, because GCC 14 treats an implicit function
# declaration as an error, which these sources of 1993 make; and -m32,
# so that a generator that writes binary data writes it as the target
# reads it -- games/monop's initdeck writes longs and structures with
# pointers in them into cards.pck, which monop reads back on the i386.
HOSTCC="$hostcc -m32 -std=gnu89"
# yacc and lex are the tree's own, which sysroot.sh builds into the
# tools directory, first on the PATH: usr.bin/yacc, and flex 2.5.2
# from contrib/. The host's would do for most grammars, but each
# writes what this tree's sources do not expect -- byacc declares
# yylex, which ftpd's ftpcmd.y defines static; flex 2.6 defines
# yylineno, which amd's fsinfo defines itself -- and with the tree's
# own both stand as Berkeley wrote them. NetBSD builds its own the
# same way, as nbyacc and nblex.
YACC="yacc"
export LC_ALL MACHINE MAKEFLAGS DESTDIR MAKEOBJDIRPREFIX PATH
export LIBMODE CC CPP AS LD HOSTCC YACC

exec bmake "$@"
