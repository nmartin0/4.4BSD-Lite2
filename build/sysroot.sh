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
# It is safe to rerun: mtree only adds what is missing, and the header
# install replaces what it installed.
#
# WHAT IT DOES NOT DO
#
#   Ownership in the root is the building user's, not root and bin.
#   That is enough to compile against; a disk image will need the real
#   ownership recorded separately.
#
#   No libraries or programs yet, and no object directories.
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

mkdir -p "$ROOT"

printf '%s\n' "sysroot.sh: hierarchy -> $ROOT"
mtree -N "$SRC/etc" -W -def "$SRC/etc/mtree/4.4BSD.dist" -p "$ROOT" -u \
    >/dev/null

printf '%s\n' "sysroot.sh: headers -> $ROOT/usr/include"
cd "$SRC/include"
sh "$REPO_ROOT/build/make.sh" install SHARED=copies DESTDIR="$ROOT" \
    BINOWN="$(id -un)" BINGRP="$(id -gn)"
