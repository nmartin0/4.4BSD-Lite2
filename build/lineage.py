#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# AI-ONLY NOTE: a host tool, not part of the tree's own sources. It asks
# where the lines of a candidate file come from, so a clearance-ledger
# row (docs/provenance/clearance.md) can say what a donor file carries.
#
#   lineage.py CANDIDATE... --lite2 FILE --early FILE [--mid FILE]
#
#   --lite2  the same file in 4.4BSD-Lite2 (the cleared text)
#   --early  the earliest copy we may read (386BSD 0.0's, the Net/2-era
#            text): the lines of it that Lite2 lacks are the ones
#            Berkeley changed or dropped between the two
#   --mid    a later clear-in-principle copy (386BSD 0.1's), for the
#            lines that first appear there
#
# For each candidate it prints the distinct substantive lines (twelve
# letters or digits or more, whitespace folded) and how many are in
# Lite2's file; in the early file but not Lite2's; first in the mid
# file; and none of these. It is a LOWER BOUND on copying: a line
# renamed or reflowed is counted as new. It reads only files it is
# given; it never reads the CSRG history.
import re, sys

def lines(path):
    out = set()
    for l in open(path, errors='replace'):
        n = re.sub(r'\s+', ' ', l).strip()
        if len(re.sub(r'[^A-Za-z0-9]', '', n)) >= 12:
            out.add(n)
    return out

def main(argv):
    cands, opts, i = [], {}, 0
    while i < len(argv):
        if argv[i] in ('--lite2', '--early', '--mid'):
            opts[argv[i][2:]] = argv[i + 1]; i += 2
        else:
            cands.append(argv[i]); i += 1
    if not cands or 'lite2' not in opts or 'early' not in opts:
        sys.exit(__doc__)
    L, E = lines(opts['lite2']), lines(opts['early'])
    M = lines(opts['mid']) if 'mid' in opts else set()
    early_only = E - L
    mid_new = M - E - L
    for c in cands:
        t = lines(c); n = len(t) or 1
        a, b, m = t & L, t & early_only, t & mid_new
        o = len(t) - len(a | b | m)
        print("%-28s lines %5d  in Lite2 %4d (%d%%)  early-not-Lite2 %3d  first-in-mid %3d (%d%%)  none %d"
              % (c.rsplit('/', 1)[-1], len(t), len(a), 100 * len(a) // n,
                 len(b), len(m), 100 * len(m) // n, o))
        for x in sorted(b):
            print("    early-not-Lite2: " + x[:100])
    print("(early lines not in Lite2: %d; first in mid: %d)" % (len(early_only), len(mid_new)))

main(sys.argv[1:])
