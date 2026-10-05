#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# Build the shim and start the kernel under QEMU.  Scratch tooling;
# see shim.c for why it exists and what replaces it.
#
#	sh build/shim/run.sh            run it
#	sh build/shim/run.sh -d int     and log exceptions to /tmp/qint.log
set -e
d=$(cd "$(dirname "$0")" && pwd)
SRC=$(cd "$d/../.." && pwd)
K=${K:-$SRC/usr/src/sys/compile/LINK.i386/vmunix}
[ -f "$K" ] || { echo "no kernel at $K -- run build/kernel.sh first" >&2; exit 1; }
for t in gcc ld qemu-system-i386; do
	command -v $t >/dev/null || { echo "not found: $t" >&2; exit 1; }
done
cd "$d"
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -O1 -c shim.c -o shim_c.o
gcc -m32 -ffreestanding -fno-pic -c shim.S -o shim_s.o
ld -m elf_i386 -T shim.ld -o shim.elf shim_s.o shim_c.o 2>/dev/null
exec qemu-system-i386 -kernel shim.elf -initrd "$K" \
    -nographic -display none -no-reboot "$@"
