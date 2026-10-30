#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. It tests the kernel's
# copyin and copyout: build/test/copyio.c, run as /sbin/init on a bare
# disk, asks for copies to and from kernel addresses, addresses that
# wrap, addresses that run past the top of user space, a read-only
# page, fresh pages and an execve with arguments and environment.
# The kernel must refuse the first and do the rest.
#
#   sh build/test/copyio.sh [kernel]
#
# Run from the top of the tree after build/kernel.sh. The kernel is
# usr/src/sys/compile/GENERIC.i386/vmunix unless named. It takes about
# four minutes, most of it the IDE probe waiting for drives that are
# not there. Needs the cross tools of build/sysroot.sh in $L2_BUILD,
# a host cc that can make -m32 programs, and qemu-system-i386.
# The result is PASS or FAIL and the exit status.
set -e
[ -f build/test/copyio.sh ] || { echo "run from the top of the tree" >&2; exit 1; }
: "${L2_BUILD:=${HOME}/.cache/lite2}"
top=$(pwd)
K=${1:-$top/usr/src/sys/compile/GENERIC.i386/vmunix}
[ -f "$K" ] || { echo "no kernel at $K -- run build/kernel.sh first" >&2; exit 1; }
CC=$L2_BUILD/tools/bin/i386-unknown-bsd4.4elf-gcc
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT

# the host tools that make the disk, as addfile-host.c says to build them
mkdir -p $w/inc/sys
ln -s $top/usr/src/sys/ufs $w/inc/ufs
cp usr/src/sys/sys/disklabel.h $w/inc/sys/
: > $w/inc/a.out.h
for t in mkfs-host addfile-host; do
	cc -m32 -std=gnu89 -w -include build/fs-host.h -I usr/src/sbin/newfs \
	    -I $w/inc -o $w/$t build/$t.c usr/src/sbin/newfs/mkfs.c
done
cc -I usr/src/sys/sys -o $w/mklabel build/mklabel.c

# the test, as init
$CC -static -o $w/init build/test/copyio.c

# 20 MB disk, labelled to fit what the IDE driver reads from it
dd if=/dev/zero of=$w/disk.img bs=1M count=20 2>/dev/null
$w/mklabel $w/disk.img 40000 >/dev/null
$w/mkfs-host $w/disk.img >/dev/null 2>&1
$w/addfile-host $w/disk.img sbin $w/init dev console:c:0:0 >/dev/null

# the shim loads the kernel; the disk is a snapshot, so it is not changed
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -O1 \
    -c build/shim/shim.c -o $w/shim_c.o
gcc -m32 -ffreestanding -fno-pic -c build/shim/shim.S -o $w/shim_s.o
ld -m elf_i386 -T build/shim/shim.ld -o $w/shim.elf $w/shim_s.o $w/shim_c.o 2>/dev/null
timeout ${COPYIO_TIMEOUT:-300} qemu-system-i386 -kernel $w/shim.elf -initrd "$K" \
    -drive file=$w/disk.img,format=raw,snapshot=on -cpu 486 \
    -nographic -display none -no-reboot > $w/out 2>&1 || true
tr -d '\r' < $w/out | grep -a '^T ' > $w/got || true

cat > $w/want <<'W'
T begin
T A copyout->kernel: ret=-1 errno=14
T B copyin<-kernel: ret=-1 errno=14
T C pipe copy: ret=5 errno=0
T C: data ok
T C2 file copy: ret=4 errno=0
T C2: data ok
T E copyout wrap: ret=-1 errno=14
T F copyin wrap: ret=-1 errno=14
T G copyin straddle: ret=-1 errno=14
T J zero-length: ret=0 errno=0
T I copyout across 2 fresh pages: ret=8 errno=0
T I: data ok
T H copyout->ro page: ret=-1 errno=14
T H: page unchanged
T K pipe copyout ending at USRSTACK: ret=8 errno=0
T M copyin ending at USRSTACK: ret=8 errno=0
T N copyin 1 byte past USRSTACK: ret=-1 errno=14
T D copyout 1 byte past USRSTACK: ret=-1 errno=14
T child: argc=3 argv1=first argv2=second-argument env0=ENVTEST=yes
T L child exit status: ret=0 errno=0
T end
W
if diff $w/want $w/got > $w/diff; then
	echo "copyio: PASS ($(wc -l < $w/got) lines as expected)"
else
	echo "copyio: FAIL -- expected (<) against got (>):"
	cat $w/diff
	tail -c 600 $w/out | tr -d '\r' | tail -8
	exit 1
fi
