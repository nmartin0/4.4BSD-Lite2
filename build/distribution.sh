#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# distribution.sh -- etc/Makefile's `distribution' target, run on a
# Linux host into a staging root.
#
#	sh build/distribution.sh DESTDIR
#
# WHY NOT `make distribution'
#
# That target was written to run as root on a 4.4BSD machine. Two of
# its steps cannot run here, and two must run differently:
#
#   pwd_mkdb	is the target's own program; the host has none, and its
#		database format is the target's. Run it on the system:
#		pwd_mkdb -p /etc/master.passwd
#   MAKEDEV std	mknod needs root, and the nodes belong on the target's
#		filesystem, not the host's. Run it on the system:
#		cd /dev; sh MAKEDEV std
#   mtree	resolves wheel, bin and the rest against the host's
#		group and passwd files. It is run, with -N pointing it at
#		the tree's own, as build/sysroot.sh already does.
#   make etc-sendmail.cf
#		runs the target's make in usr.sbin/sendmail/cf/cf; here
#		its rule is run directly, with the build machine's m4,
#		which sendmail's configuration files ask for (GNU m4
#		works, cf/cf/Makefile says).
#
# Everything else is the target's, step for step and in its order. The
# file lists are read out of etc/Makefile with bmake -V, so a change
# there reaches this script without an edit here.
#
# OWNERS. install -o and -g are passed only when this runs as uid 0 --
# root, or under fakeroot(1), which is how an unprivileged user gets
# them into an archive:
#
#	fakeroot sh -c 'sh build/distribution.sh /some/stage &&
#	    cd /some/stage && tar -cf ../dist.tar --numeric-owner .'
#
# The names are turned into numbers from the tree's own etc/group and
# etc/master.passwd, since the host's root:wheel may not exist and its
# operator, staff and utmp are other numbers.
#
# Nothing here is copied from another system. NetBSD's unprivileged
# build answers the same problem with a METALOG; Debian's packaging
# answers it with fakeroot, which this uses.
set -e

die() { printf '%s\n' "distribution.sh: $*" >&2; exit 1; }

[ $# -eq 1 ] || die "usage: sh build/distribution.sh DESTDIR"
case "$1" in
/*) D=$1 ;;
*)  D=$(pwd)/$1 ;;
esac
case "$D" in
*[[:space:]]*) die "DESTDIR contains whitespace" ;;
esac

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
E="$REPO_ROOT/usr/src/etc"
MACHINE=i386

for t in bmake mtree install m4; do
	command -v "$t" >/dev/null 2>&1 || die "not found: $t"
done

# The lists, from the Makefile itself. MACHINE is passed because BIN1
# names etc.${MACHINE}/disktab.
mvar() { bmake -C "$E" -f Makefile MACHINE=$MACHINE -V "\${$1}" 2>/dev/null; }
BIN1=$(mvar BIN1)
BIN2=$(mvar BIN2)
NAMEDB=$(mvar NAMEDB)
KRB=$(mvar KRB)
[ -n "$BIN1" ] || die "could not read BIN1 from $E/Makefile"

# Names to numbers, from the tree.
gid() { awk -F: -v n="$1" '$1 == n { print $3 }' "$E/group"; }
uid() { awk -F: -v n="$1" '$1 == n { print $3 }' "$E/master.passwd"; }

# inst OWNER GROUP MODE file... target
inst() {
	o=$1 g=$2 m=$3
	shift 3
	if [ "$(id -u)" -eq 0 ]; then
		install -c -o "$(uid "$o")" -g "$(gid "$g")" -m "$m" "$@"
	else
		install -c -m "$m" "$@"
	fi
}

cd "$E"

mkdir -p "$D/etc" "$D/dev" "$D/tmp"
# pwd_mkdb: on the system, as above.
cp group "$D/etc/group"
# `-mtree' in the Makefile: a failure is not fatal there either. mtree
# exits 2 when it has had to change anything -- here, /tmp's mode, from
# the mkdir above -- which is the normal case on a fresh root.
mtree -N "$E" -def mtree/4.4BSD.dist -p "$D/" -u >/dev/null || :
inst root wheel 555 MAKEDEV.local etc.$MACHINE/MAKEDEV "$D/dev"
# MAKEDEV std: on the system, as above.
inst root wheel 444 COPYRIGHT "$D/"
inst root wheel 600 master.passwd "$D/etc"
inst root wheel 600 hosts.equiv "$D/etc"
inst root wheel 744 netstart "$D/etc"
# shellcheck disable=SC2086	# lists of names, as make passes them
inst root wheel 644 $BIN1 "$D/etc"
# shellcheck disable=SC2086
inst root wheel 666 $BIN2 "$D/etc"
(
	cd root
	inst root wheel 644 dot.cshrc "$D/root/.cshrc"
	inst root wheel 600 dot.klogin "$D/root/.klogin"
	inst root wheel 644 dot.login "$D/root/.login"
	inst root wheel 644 dot.profile "$D/root/.profile"
	inst root wheel 600 dot.rhosts "$D/root/.rhosts"
	rm -f "$D/.cshrc" "$D/.profile"
	ln "$D/root/.cshrc" "$D/.cshrc"
	ln "$D/root/.profile" "$D/.profile"
)
(
	cd mtree
	inst root wheel 600 special "$D/etc/mtree"
	inst root wheel 444 4.4BSD.dist "$D/etc/mtree"
)
# shellcheck disable=SC2086
(cd namedb && inst root wheel 644 $NAMEDB "$D/etc/namedb")
# shellcheck disable=SC2086
(cd kerberosIV && inst root wheel 444 $KRB "$D/etc/kerberosIV")
inst root operator 664 /dev/null "$D/etc/dumpdates"
inst root staff 644 /dev/null "$D/var/log/lastlog"
inst root wheel 644 /dev/null "$D/var/log/messages"
inst root wheel 644 /dev/null "$D/var/log/maillog"
inst root wheel 644 /dev/null "$D/var/log/lpd-errs"
inst root utmp 664 /dev/null "$D/var/run/utmp"
(cd etc.$MACHINE && inst root wheel 444 fstab.* "$D/etc")
# The links, as the Makefile makes them, the second X11 replacing the
# first as it does there.
(cd "$D/" && rm -f sys && ln -s usr/src/sys sys)
(cd "$D/etc" && rm -f rmt && ln -s /usr/sbin/rmt rmt)
(cd "$D/etc" && rm -f termcap && ln -s /usr/share/misc/termcap termcap)
(cd "$D/usr" && rm -f X11 && ln -s X11R5 X11)
(cd "$D/usr" && rm -f X11 && ln -s X11R6 X11)
(cd "$D/usr/bin" && rm -f X11 && ln -s /usr/X11/bin X11)
(cd "$D/usr/lib" && rm -f X && ln -s /usr/hpux/lib/X X)
(cd "$D/usr/lib" && rm -f X11 && ln -s /usr/X11/lib X11)
(cd "$D/usr/lib" && rm -f grmd && ln -s /usr/hpux/lib/grmd grmd)
(cd "$D/usr/lib" && rm -f tmac && ln -s /usr/share/tmac tmac)
(cd "$D/usr/ucb" && rm -f rdist && ln -s /usr/bin/rdist rdist)
# The Makefile's `cd ../usr.sbin/sendmail/cf/cf; make etc-sendmail.cf':
# generic-bsd4.4.mc through the build machine's m4, by the rule there.
(
	cd ../usr.sbin/sendmail/cf/cf
	m4 ../m4/cf.m4 generic-bsd4.4.mc > "$D/etc/sendmail.cf.new"
	inst root wheel 644 "$D/etc/sendmail.cf.new" "$D/etc/sendmail.cf"
	rm -f "$D/etc/sendmail.cf.new"
)

printf '%s\n' "distribution.sh: done; on the system, still:" \
    "  pwd_mkdb -p /etc/master.passwd" "  cd /dev && sh MAKEDEV std"
