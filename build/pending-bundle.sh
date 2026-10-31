#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: it makes
# the "pending" bundle the maintainer asked for on 2026-10-30, so that
# patches can be sent when he cannot apply them and applied later, many
# at once.
#
#   sh build/pending-bundle.sh BASE OUT.bundle
#
# BASE is the last commit the maintainer is known to have applied; every
# commit from BASE (exclusive) to HEAD becomes one patch file. The bundle
# holds a single branch, `patches`, with no history of the tree in it: one
# commit with the patch files NNN-*.patch (N from "(patch N)" in the
# subject) and MANIFEST. Because it carries no part of the tree, it needs
# nothing in the maintainer's repository to be fetched. MANIFEST has a
# line per patch:
#     N file sha256 tree-before tree-after
# apply-pending.sh (same directory) applies, in order, those not applied
# yet (found by "(patch N)" in dev3's log), checking the trees.
# It is cumulative: send the bundle from the oldest unapplied patch.
set -e
base=$1 out=$2
[ -n "$base" ] && [ -n "$out" ] || { echo "usage: sh build/pending-bundle.sh BASE OUT.bundle" >&2; exit 1; }
w=$(mktemp -d); trap 'rm -rf "$w"' EXIT
mkdir $w/p; : > $w/p/MANIFEST
for c in $(git rev-list --reverse $base..HEAD); do
	n=$(git log -1 --format=%s $c | sed -n 's/.*(patch \([0-9][0-9]*\)).*/\1/p')
	[ -n "$n" ] || { echo "commit $c has no (patch N) in its subject" >&2; exit 1; }
	f=$n-$(date -u +%Y-%m-%d).patch
	git format-patch -1 $c --stdout > $w/p/$f
	printf '%s %s %s %s %s\n' $n $f $(sha256sum $w/p/$f | cut -d' ' -f1) \
	    $(git rev-parse $c^^{tree}) $(git rev-parse $c^{tree}) >> $w/p/MANIFEST
done
( cd $w/p && git init -q . && git add -A &&
  git -c user.name=bundle -c user.email=bundle@localhost commit -q -m "pending patches" &&
  git branch -M patches && git bundle create "$out" patches >/dev/null 2>&1 )
echo "wrote $out:"; cut -d' ' -f1,2 $w/p/MANIFEST
