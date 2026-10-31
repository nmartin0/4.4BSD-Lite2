#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: the one
# way to run a kernel test (plan-4.5.md stage 5). A test is two files,
# build/test/NAME.c, which runs as /sbin/init on a bare disk and prints
# lines beginning `T ', and build/test/NAME.want, the lines it must
# print. The kernel runs under QEMU (-cpu ${KTEST_CPU:-486}) on a snapshot of a 20 MB
# disk and the lines are compared; the result is PASS or FAIL and the
# exit status.
#
#   sh build/test/ktest.sh TEST [kernel]
#
# Run from the top of the tree after build/kernel.sh. The kernel is
# usr/src/sys/compile/GENERIC.i386/vmunix unless named. A run takes
# about four minutes, most of it the IDE probe waiting for drives that
# are not there (KTEST_TIMEOUT seconds at most, 300 by default). A test
# that does much disk I/O may need KTEST_SLOW=1 (on, whatever its value, if
# set): QEMU 8.2.2 can lose disk
# interrupts otherwise (the guest then sits idle, as in the CD install),
# and the one-instruction-per-block mode it asks for is slower.
# KTEST_LOG=file keeps the whole console output (the `T' lines are all that
# is compared; a kernel that prints its own diagnostics shows them there). Needs the
# cross tools of build/sysroot.sh in $L2_BUILD, a host cc that can make
# -m32 programs, and qemu-system-i386.
set -e
[ -f build/test/ktest.sh ] || { echo "run from the top of the tree" >&2; exit 1; }
: "${L2_BUILD:=${HOME}/.cache/lite2}"
top=$(pwd)
T=$1
[ -f build/test/$T.c ] && [ -f build/test/$T.want ] || { echo "usage: sh build/test/ktest.sh TEST [kernel]  (build/test/TEST.c and TEST.want)" >&2; exit 1; }
K=${2:-$top/usr/src/sys/compile/GENERIC.i386/vmunix}
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
$CC -static -o $w/init build/test/$T.c

# 20 MB disk, labelled to fit what the IDE driver reads from it
dd if=/dev/zero of=$w/disk.img bs=1M count=20 2>/dev/null
$w/mklabel $w/disk.img 40000 >/dev/null
$w/mkfs-host $w/disk.img >/dev/null 2>&1
$w/addfile-host $w/disk.img sbin $w/init dev console:c:0:0 d wd0a:b:0:0 >/dev/null

# the shim loads the kernel; the disk is a snapshot, so it is not changed
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -O1 \
    -c build/shim/shim.c -o $w/shim_c.o
gcc -m32 -ffreestanding -fno-pic -c build/shim/shim.S -o $w/shim_s.o
ld -m elf_i386 -T build/shim/shim.ld -o $w/shim.elf $w/shim_s.o $w/shim_c.o 2>/dev/null
timeout ${KTEST_TIMEOUT:-${COPYIO_TIMEOUT:-300}} qemu-system-i386 -kernel $w/shim.elf -initrd "$K" \
    -drive file=$w/disk.img,format=raw,snapshot=on -cpu ${KTEST_CPU:-486} \
    ${KTEST_SLOW:+-accel tcg,one-insn-per-tb=on} \
    ${KTEST_MONITOR:+-monitor tcp:127.0.0.1:$KTEST_MONITOR,server,nowait} \
    -nographic -display none -no-reboot > $w/out 2>&1 || true
[ -z "$KTEST_LOG" ] || cp $w/out "$KTEST_LOG"
tr -d '\r' < $w/out | grep -a '^T ' > $w/got || true

cp build/test/$T.want $w/want
if diff $w/want $w/got > $w/diff; then
	echo "$T: PASS ($(wc -l < $w/got) lines as expected)"
else
	echo "$T: FAIL -- expected (<) against got (>):"
	cat $w/diff
	tail -c 600 $w/out | tr -d '\r' | tail -8
	exit 1
fi
