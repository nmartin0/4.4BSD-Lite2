#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. It runs build/test/copyio.sh
# on a kernel whose copyout does the 386's page-by-page write check on
# every CPU (locore.s, -DFORCE_386_COPYOUT), so that the check is run
# under a CPU model that has write protection. It makes a temporary
# configuration, COPYIO386, of GENERIC.i386 plus that option, builds it
# with build/kernel.sh, runs copyio.sh on it and removes the
# configuration; the build directory usr/src/sys/compile/COPYIO386 is
# left (it is ignored by git).
#
#   sh build/test/copyio386.sh
#
# This is not a real 386: it shows the walk's logic, not the 386's
# behaviour (docs/provenance/copyout.md).
set -e
[ -f build/test/copyio386.sh ] || { echo "run from the top of the tree" >&2; exit 1; }
c=usr/src/sys/i386/conf
trap 'rm -f $c/COPYIO386' EXIT
sed '0,/^options/s//options\t\t"FORCE_386_COPYOUT"\n&/' $c/GENERIC.i386 > $c/COPYIO386
rm -rf usr/src/sys/compile/COPYIO386
sh build/kernel.sh COPYIO386 >/dev/null 2>&1 || { echo "copyio386: the kernel build failed" >&2; exit 1; }
# the option must have reached the build, or the run proves nothing
grep -q 'DFORCE_386_COPYOUT' usr/src/sys/compile/COPYIO386/Makefile || {
	echo "copyio386: the option did not reach the Makefile" >&2; exit 1; }
sh build/test/copyio.sh usr/src/sys/compile/COPYIO386/vmunix
