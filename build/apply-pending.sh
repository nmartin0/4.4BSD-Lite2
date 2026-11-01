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
# pending bundle (build/pending-bundle.sh) to the maintainer's
# repository, branch dev3, and pushes. Usage, from any directory:
#
#	sh /ABSOLUTE/PATH/apply-pending.sh /ABSOLUTE/PATH/pending-NNN.bundle [/ABSOLUTE/PATH/TO/REPO]
#
# The repository defaults to $HOME/BSD-Lite2. Both paths must be
# absolute (they are checked), so it does not matter where it is run
# from or where the script itself lives: it needs no copy of itself in
# the tree, and it is sent beside each bundle, since a repository that
# predates patch 245 has no build/apply-pending.sh at all.
#
# Patches already in dev3 (a commit whose subject has "(patch N)") are
# skipped; the rest go in numerical order, each checked against the tree
# it must start from and the tree it must make. Any mismatch stops
# before the push, and `git reset --hard origin/dev3` undoes it.
#
# Changed (2026-11-01, patch 263), after the maintainer reported it did
# not work: the command given with each bundle ran it as
# build/apply-pending.sh, a relative path that works only from the top
# of a tree that already has the file -- and dev3 on GitHub stood at
# patch 243, before the file existed. It also took the bundle's path as
# given and then cd'd to the repository, so a relative bundle path
# broke there. Now both paths are absolute and checked, the repository
# can be named (a worktree too), and `shasum -a 256' is used where there
# is no sha256sum (macOS). Found by review: uncommitted changes are
# looked for before switching to dev3, not after, since the switch
# carried them over; and tar is told to read its standard input, which
# FreeBSD's tar does not assume. (The bundles sent for patches 258-262 also each held one
# patch, where pending-bundle.sh says to start from the oldest patch
# not yet applied; from 263 on they start from dev3 as GitHub has it.)
#
# The whole body is in a subshell, so sh reads all of it before running
# any: git am may rewrite this very file when it is run from the tree.
(
	set -e
	b=$1
	repo=${2:-$HOME/BSD-Lite2}
	case $b in
	/*) ;;
	*) echo "usage: sh /path/to/apply-pending.sh /absolute/path/to/pending-NNN.bundle [/absolute/path/to/repo]"
	   echo "the bundle's path must be absolute (start with /): got '$b'"; exit 1;;
	esac
	case $repo in
	/*) ;;
	*) echo "the repository's path must be absolute (start with /): got '$repo'"; exit 1;;
	esac
	[ -f "$b" ] || { echo "no such bundle: $b"; exit 1; }
	git -C "$repo" rev-parse --git-dir >/dev/null 2>&1 || { echo "not a git repository: $repo"; exit 1; }
	if command -v sha256sum >/dev/null 2>&1; then sum256() { sha256sum "$1" | cut -d' ' -f1; }
	else sum256() { shasum -a 256 "$1" | cut -d' ' -f1; }
	fi
	echo "bundle:     $b"
	echo "repository: $repo"
	cd "$repo"
	git am --abort 2>/dev/null || true
	# before the checkout, which would carry a change to dev3
	if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
		echo "uncommitted changes in $repo -- stopping, nothing changed"
		exit 1
	fi
	git checkout dev3
	git fetch origin
	[ "$(git rev-parse HEAD)" = "$(git rev-parse origin/dev3)" ] || {
		echo "dev3 here and on GitHub differ -- stopping, nothing changed"; exit 1; }
	git bundle verify "$b" >/dev/null
	git fetch "$b" refs/heads/patches
	w=$(mktemp -d); trap 'rm -rf "$w"' EXIT
	git archive FETCH_HEAD | tar -xf - -C "$w"
	applied=0
	while read n f sum pre post; do
		if git log --format=%s | grep -q "(patch $n)"; then
			echo "patch $n: already in dev3, skipped"; continue
		fi
		[ "$(sum256 "$w/$f")" = "$sum" ] || { echo "$f is damaged -- stopping"; exit 1; }
		t=$(git rev-parse HEAD^{tree})
		[ "$t" = "$pre" ] || { echo "patch $n needs tree $pre, dev3 is at $t -- stopping, nothing pushed"; exit 1; }
		git am --keep-cr "$w/$f"
		t=$(git rev-parse HEAD^{tree})
		[ "$t" = "$post" ] || { echo "patch $n made tree $t, expected $post -- not pushing"; exit 1; }
		applied=$((applied+1))
	done < "$w/MANIFEST"
	if [ $applied -gt 0 ]; then git push origin dev3; fi
	echo "done: $applied patch(es) applied$( [ $applied -gt 0 ] && echo ' and pushed')"
)
echo "apply-pending.sh finished (status $?)"
