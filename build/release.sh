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
#   the base set        the whole tree, by Lite2's own top-level
#                       `make build'; the boot blocks, the kernel,
#                       etc/'s files; archived as ustar
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
# Anything that fails stops it.

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

# The sources, as 4.4BSD's distribution carried them beside the
# system (share/doc/smm/01.setup, section 2): three tar images to be
# extracted into /usr/src -- the kernel's, sys; the utilities', the
# rest but sys and contrib; and the contributed software's, contrib
# -- made here, before the build, from the tree as it stands, but for
# the kernels built in sys/compile: that directory goes empty but for
# its .keep_me, as FreeBSD 2.0, NetBSD 1.0 and OpenBSD ship it, since
# config makes a kernel's directory in it and cannot make it. On the
# CD in source/sets, as release(7) puts a release's source sets, named
# as NetBSD names its: syssrc, src, and contribsrc beside them. They are extracted by hand, as the setup guide has it
# (install.md, *The sources*).
S="tar -C usr/src --numeric-owner --owner=0 --group=0 --format=ustar"
mkdir -p "$O/src"
$S -cf "$O/src/syssrc.tar" --exclude=./sys/compile ./sys
$S -rf "$O/src/syssrc.tar" --no-recursion ./sys/compile ./sys/compile/.keep_me
$S -cf "$O/src/src.tar" --exclude=./sys --exclude=./contrib .
$S -cf "$O/src/contribsrc.tar" ./contrib

# the target root, and the host tools
sh build/sysroot.sh

# the base set: the whole tree, as Lite2's top-level Makefile builds
# and installs it -- `make build', the way 4.4BSD was rebuilt from its
# own sources (share/doc/smm/01.setup, *Recompiling and reinstalling
# system software*) -- into the target root. SHARED=copies for the
# headers, as sysroot.sh installs them (it explains why). Its input
# is /dev/null: games/phantasia's setup asks for a message of the day,
# and a release has none to give.
(cd usr/src && sh "$r/build/make.sh" build SHARED=copies) </dev/null
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
mkdir -p "$O/cd/i386/binary/sets" "$O/cd/i386/installation/floppy" \
    "$O/cd/source/sets"
cp "$O/base.tar" "$O/cd/i386/binary/sets/"
cp "$O/src/syssrc.tar" "$O/src/src.tar" "$O/src/contribsrc.tar" \
    "$O/cd/source/sets/"
cp "$O/boot.fs" "$O/cd/i386/installation/floppy/boot.fs"
mkisofs -quiet -A "4.4BSD-Lite2 i386" -V LITE2_I386 -r \
    -b i386/installation/floppy/boot.fs \
    -c i386/installation/floppy/boot.catalog \
    -o "$O/i386-cd.iso" "$O/cd"

echo "release.sh: $O/i386-cd.iso"
