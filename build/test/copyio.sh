#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: the
# copyin/copyout test (build/test/copyio.c, copyio.want, provenance/
# copyout.md), run by build/test/ktest.sh.
#
#   sh build/test/copyio.sh [kernel]
exec sh build/test/ktest.sh copyio "$@"
