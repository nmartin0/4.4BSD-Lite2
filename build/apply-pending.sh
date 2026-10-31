#!/bin/sh
# SPDX-FileCopyrightText: 2026 Nicholas Martin
#
# This file deliberately carries no SPDX licence identifier: the
# licence for this tree's own scripts is not settled yet, and the tag
# is kept out rather than written into a sentence, where a scanner
# would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
# cover the Berkeley material, not this file.
#
# Nothing here is copied from another system. AI-ONLY NOTE: applies a
# pending bundle (build/pending-bundle.sh) to ~/BSD-Lite2's dev3 and
# pushes. Usage:  sh apply-pending.sh ~/Downloads/pending-NNN.bundle
# Patches already in dev3 (a commit whose subject has "(patch N)") are
# skipped; the rest go in numerical order, each checked against the tree
# it must start from and the tree it must make. Any mismatch stops
# before the push, and `git reset --hard origin/dev3` undoes it.
(
	set -e
	b=$1
	[ -f "$b" ] || { echo "usage: sh apply-pending.sh BUNDLE"; exit 1; }
	cd ~/BSD-Lite2
	git am --abort 2>/dev/null || true
	git checkout dev3
	if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
		echo "uncommitted changes in ~/BSD-Lite2 -- stopping, nothing changed"
		exit 1
	fi
	git fetch origin
	[ "$(git rev-parse HEAD)" = "$(git rev-parse origin/dev3)" ] || {
		echo "dev3 here and on GitHub differ -- stopping, nothing changed"; exit 1; }
	git bundle verify "$b" >/dev/null
	git fetch "$b" refs/heads/patches
	w=$(mktemp -d); trap 'rm -rf "$w"' EXIT
	git archive FETCH_HEAD | tar -x -C $w
	applied=0
	while read n f sum pre post; do
		if git log --format=%s | grep -q "(patch $n)"; then
			echo "patch $n: already in dev3, skipped"; continue
		fi
		[ "$(sha256sum $w/$f | cut -d' ' -f1)" = "$sum" ] || { echo "$f is damaged -- stopping"; exit 1; }
		t=$(git rev-parse HEAD^{tree})
		[ "$t" = "$pre" ] || { echo "patch $n needs tree $pre, dev3 is at $t -- stopping, nothing pushed"; exit 1; }
		git am --keep-cr $w/$f
		t=$(git rev-parse HEAD^{tree})
		[ "$t" = "$post" ] || { echo "patch $n made tree $t, expected $post -- not pushing"; exit 1; }
		applied=$((applied+1))
	done < $w/MANIFEST
	if [ $applied -gt 0 ]; then git push origin dev3; fi
	echo "done: $applied patch(es) applied$( [ $applied -gt 0 ] && echo ' and pushed')"
)
echo "apply-pending.sh finished (status $?)"
