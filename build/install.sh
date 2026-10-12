#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# install -- the host's install(1), with owners and groups as this
# tree names them.
#
# build/sysroot.sh copies this into ${L2_BUILD}/tools/bin as `install',
# with @ETC@ replaced by the tree's usr/src/etc, and build/make.sh puts
# that directory first on PATH. Every `install' a Makefile runs then
# comes here.
#
# WHY. Lite2's Makefiles name owners and groups -- BINGRP= kmem for ps,
# operator for df, tty for dump -- and a Linux install(1) looks those
# names up in the host's /etc/group, where they are other numbers or
# absent: Debian's kmem is 15 and this tree's 2, Debian's tty is 5 and
# here 5 is operator. Installed as root, ps came out setgid to a group
# the target's /dev/kmem does not have, and showed every command in
# parentheses; installed as anyone else, the chgrp fails outright.
#
# WHAT IT DOES, to the arguments, and nothing else:
#
#   -o name, -g name	as uid 0 (root, or under fakeroot), the name is
#			turned into a number from the tree's own
#			master.passwd and group -- etc/Makefile's
#			distribution target and build/distribution.sh
#			resolve them there too -- and an unknown name
#			is an error. As anyone else both are dropped:
#			an unprivileged user cannot give files away,
#			and the archive's owners are settled later, as
#			install.md says.
#   -f flags		dropped, with its argument, and -fflags too.
#			4.4BSD file flags (rcp's Makefile asks for
#			schg) do not exist on the host's filesystem,
#			and GNU install has no -f. The target sets
#			them itself if it wants them.
#
# Everything else goes to the host's install unchanged, and anything
# after `--' is not looked at. Forms the Makefiles do not write --
# -obin, --owner=bin -- pass through and are the host's to resolve.
#
# NetBSD's build solves the same problem with its own install, built
# for the host, that takes -N etcdir; this is the same idea for a tree
# whose install(1) cannot be built on Linux, since it calls chflags(2).
#
# Nothing here is copied from another system.

etc="@ETC@"

me=$(cd "$(dirname "$0")" && pwd -P)

# The host's install: the first on PATH that is not this file.
real=
IFS_save=$IFS
IFS=:
for d in $PATH; do
	[ -n "$d" ] || continue
	[ "$(cd "$d" 2>/dev/null && pwd -P)" = "$me" ] && continue
	if [ -x "$d/install" ]; then
		real="$d/install"
		break
	fi
done
IFS=$IFS_save
if [ -z "$real" ]; then
	echo "install: no host install(1) on PATH" >&2
	exit 1
fi

# name -> number, from a colon-separated file: field 1 the name, field
# 3 the number. A number is passed through.
lookup() {	# file name what
	case "$2" in
	*[!0-9]*)
		;;
	*)
		echo "$2"
		return 0
		;;
	esac
	n=$(awk -F: -v n="$2" '$1 == n { print $3; exit }' "$1")
	if [ -z "$n" ]; then
		echo "install: $3 \`$2' is not in $1" >&2
		return 1
	fi
	echo "$n"
}

root=no
[ "$(id -u)" -eq 0 ] && root=yes

# Rebuild the argument list in place: each argument is shifted off the
# front and, changed or not, appended at the back. The options parsed
# are BSD install's -- c, s, o, g, m, f, and m's argument is carried as
# it is -- since those are what this tree's Makefiles write; anything
# after `--', and any other option, goes through untouched.
n=$#
i=0
opts=yes
while [ "$i" -lt "$n" ]; do
	a=$1
	shift
	i=$((i + 1))
	if [ "$opts" = no ]; then
		set -- "$@" "$a"
		continue
	fi
	case "$a" in
	--)
		opts=no
		set -- "$@" "$a"
		;;
	-o|-g|-f)
		if [ "$i" -ge "$n" ]; then
			echo "install: $a needs an argument" >&2
			exit 1
		fi
		v=$1
		shift
		i=$((i + 1))
		if [ "$a" != -f ] && [ "$root" = yes ]; then
			if [ "$a" = -o ]; then
				v=$(lookup "$etc/master.passwd" "$v" owner) ||
				    exit 1
			else
				v=$(lookup "$etc/group" "$v" group) || exit 1
			fi
			set -- "$@" "$a" "$v"
		fi
		;;
	-m)
		if [ "$i" -lt "$n" ]; then
			set -- "$@" "$a" "$1"
			shift
			i=$((i + 1))
		else
			set -- "$@" "$a"
		fi
		;;
	-f?*)
		# -fschg, written as one argument, as INSTALLFLAGS gives it
		;;
	*)
		set -- "$@" "$a"
		;;
	esac
done

exec "$real" "$@"
