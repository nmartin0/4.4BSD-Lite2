#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. It runs, in order, the
# recipes docs/provenance/install.md gives one at a time, and makes
# the installable CD from a clean tree -- what FreeBSD's `make
# release' and NetBSD's `build.sh release' are for in theirs:
#
#   the target root     build/sysroot.sh
#   the base set        install.md, *Staging*: the programs, termcap,
#                       the boot blocks, the kernel, etc/'s files,
#                       archived as ustar
#   the install kernel  *A root inside the kernel*, with MFS_ROOT=1600
#   the install root    *The install root*, written into that kernel
#   the boot floppy     *A root inside the kernel*: 2.88 MB, boot1,
#                       boot2, the kernel
#   the CD              *Installing from the CD*: release(7)'s tree
#                       and mkisofs
#
# Run from the top of the tree, as root or, as an ordinary user, under
# fakeroot(1), for the tree's owners and groups (install.md,
# *Staging*): `fakeroot sh build/release.sh'. Host tools beyond
# what build/sysroot.sh needs: makefs and mkisofs (Debian's makefs and
# genisoimage). Output: ${L2_BUILD}/release/, the CD as i386-cd.iso.
#
# Some installs stop at a manual page, after their programs are in
# place: there is no nroff (install.md, *Staging*). Inside sbin the
# make goes on past them; usr.bin/id's stops the directory. So a
# directory that fails here is reported and the build goes on;
# anything else that fails stops it.

set -e

# The tree's own modes, whatever the caller's umask: 002, the default
# for users on hosts with per-user groups, leaves directories made here
# group-writable, and mtree -u, building the install root, then
# corrects one and stops the build.
umask 022

[ -f build/release.sh ] || { echo "run from the top of the tree" >&2; exit 1; }
# makefs is a system administration tool, so Debian installs it in
# /usr/sbin, which an ordinary user's PATH need not have; the install
# root's Makefile runs it by name too.
PATH=$PATH:/usr/sbin:/sbin
export PATH
for t in makefs mkisofs; do
	command -v $t >/dev/null || { echo "not found: $t" >&2; exit 1; }
done

export L2_BUILD=${L2_BUILD:-$HOME/.cache/lite2}
r=$(pwd)
R=$L2_BUILD/root
T=$L2_BUILD/tools/bin
O=$L2_BUILD/release

# From clean, as a release is: a second run over the objects of a
# first stops in libedit, whose .depend -- written by the first run's
# mkdep -- bmake will not read back. The target root goes too, so the
# base set holds only what this run installs.
rm -rf "$O" "$L2_BUILD/obj" "$R"
mkdir -p "$O"

# the target root, and the host tools
sh build/sysroot.sh

# the base set: install.md, *Staging*
for d in bin sbin libexec/getty usr.bin/login usr.bin/passwd \
    usr.sbin/pwd_mkdb usr.sbin/dev_mkdb usr.sbin/kvm_mkdb \
    usr.sbin/syslogd usr.sbin/chown usr.sbin/update \
    usr.bin/logger usr.bin/find usr.bin/true usr.bin/false \
    usr.bin/tty usr.bin/id usr.bin/who usr.bin/more usr.bin/wc \
    usr.bin/head usr.bin/tail usr.bin/env usr.bin/uname usr.bin/w \
    usr.bin/xinstall usr.bin/sed usr.bin/tset usr.sbin/sysctl \
    usr.sbin/quotaon usr.sbin/accton usr.sbin/timed usr.sbin/lpr \
    usr.sbin/inetd usr.sbin/portmap usr.sbin/amd usr.bin/cap_mkdb \
    usr.bin/script usr.sbin/pstat usr.bin/awk usr.bin/man
do
	(cd usr/src/$d && sh "$r/build/make.sh" \
	    depend all install) ||
	    echo "release.sh: $d: install stopped (see the note above)" >&2
done
(cd usr/src/share/termcap && sh "$r/build/make.sh" termcap)
"$T/install" -o bin -g bin -m 444 \
    "$L2_BUILD/obj$r/usr/src/share/termcap/termcap" \
    "$R/usr/share/misc/termcap"
(cd usr/src/sys/boot/i386 && sh "$r/build/make.sh" all install)
sh build/kernel.sh
install -m 755 usr/src/sys/compile/GENERIC.i386/vmunix "$R/vmunix"
sh build/distribution.sh "$R"
tar -C "$R" --numeric-owner --format=ustar -cf "$O/base.tar" .

# the install kernel: install.md, *A root inside the kernel*
c=usr/src/sys/i386/conf
sed -e 's/^ident[[:space:]]*GENERIC/ident		INSTALL/' \
    -e '/^options[[:space:]]*GENERIC/d' \
    -e 's/^config[[:space:]].*/config		vmunix	root on wd0 swap on wd0/' \
    $c/GENERIC.i386 > $c/INSTALL
printf 'options\t\t"MFS_ROOT=1600"\n' >> $c/INSTALL
sh build/kernel.sh INSTALL
rm -f $c/INSTALL

# the install root, into the install kernel: *The install root*
f=usr/src/distrib/i386/floppies/inst
(cd $f && sh "$r/build/make.sh")
cp usr/src/sys/compile/INSTALL/vmunix "$O/vmunix"
"$T/write_mfs_in_kernel" "$O/vmunix" "$L2_BUILD/obj$r/$f/inst-11.fs"

# the boot floppy: *A root inside the kernel*
cc -I usr/src/sys/sys -o "$O/mklabel" build/mklabel.c
mkdir "$O/fl"
cp "$O/vmunix" "$O/fl/vmunix"
makefs -t ffs -B le -o version=1,bsize=8192,fsize=1024 \
    -o minfree=0,optimization=space -s $(( (5760-256-32)*512 )) \
    "$O/fl.img" "$O/fl"
dd if=/dev/zero of="$O/boot.fs" bs=512 count=5760 2>/dev/null
"$O/mklabel" "$O/boot.fs" 5760
dd if="$O/fl.img" of="$O/boot.fs" bs=512 seek=256 conv=notrunc 2>/dev/null
dd if="$R/usr/mdec/boot1" of="$O/boot.fs" conv=notrunc 2>/dev/null
dd if="$R/usr/mdec/boot2" of="$O/boot.fs" bs=512 skip=1 seek=2 \
    conv=notrunc 2>/dev/null

# the CD: *Installing from the CD*
mkdir -p "$O/cd/i386/binary/sets" "$O/cd/i386/installation/floppy"
cp "$O/base.tar" "$O/cd/i386/binary/sets/"
cp "$O/boot.fs" "$O/cd/i386/installation/floppy/boot.fs"
mkisofs -quiet -A "4.4BSD-Lite2 i386" -V LITE2_I386 -r \
    -b i386/installation/floppy/boot.fs \
    -c i386/installation/floppy/boot.catalog \
    -o "$O/i386-cd.iso" "$O/cd"

echo "release.sh: $O/i386-cd.iso"
