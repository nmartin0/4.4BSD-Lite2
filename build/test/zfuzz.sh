#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: a host
# (not kernel) fuzz test for compress(1)'s decode-stack bound
# (build/test/zfuzz.c, docs/provenance/compress-stack.md). Builds
# usr/src/usr.bin/compress/zopen.c as a 32-bit host program (funopen()
# shimmed with fopencookie()) under AddressSanitizer and mutation-
# fuzzes it. Host-only: this is a user program, not something ktest.sh
# boots under QEMU.
#
# This is a fuzzer, not a fixed-output test: trial count and seed
# vary by invocation, so there is deliberately no NAME.want here for
# ktest.sh's diffing convention to compare against. Passing (no ASan
# abort, exit 0) is the only pass/fail signal; see
# docs/provenance/compress-stack.md for what this fuzzer did and did
# not find, and why.
#
#   sh build/test/zfuzz.sh [trials] [seed]
set -e
cd "$(dirname "$0")/../.."
t=${1:-20000}
s=${2:-1}
w=$(mktemp -d)
trap 'rm -rf "$w"' EXIT
gcc -m32 -g -O1 -fsanitize=address -std=gnu89 -w -DEFTYPE=79 '-D__P(x)=x' \
    -D_GNU_SOURCE -Dcalloc=zd_calloc \
    usr/src/usr.bin/compress/zopen.c build/test/zfuzz.c -o "$w/zfuzz"
ASAN_OPTIONS=detect_leaks=0 "$w/zfuzz" "$t" "$s"
