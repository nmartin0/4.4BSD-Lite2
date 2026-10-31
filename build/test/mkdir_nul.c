/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * This file deliberately carries no SPDX licence identifier: the
 * licence for this tree's own tests is not settled yet, and the tag
 * is kept out rather than written into a sentence, where a scanner
 * would read it as a declaration. 4.4BSD-Lite2's terms (./COPYRIGHT)
 * cover the Berkeley material, not this file.
 *
 * Nothing here is copied from another system. AI-ONLY NOTE: a host
 * test for bin/mkdir/mkdir.c's build() writing past the path's
 * terminating NUL on its last component (docs/provenance/mkdir-nul.md).
 * mkdir(1) is a plain host program, so this builds build() itself
 * (#define main to keep mkdir.c's own main out of the way, and stub
 * setmode()/getmode(), which this test's call to build() never
 * reaches) with the host's own cc, not the cross toolchain.
 *
 * Maintenance note (added after independent review): this #includes
 * the real mkdir.c textually, so it is the exact function under
 * test, not a copy that can drift -- but it has no defensiveness
 * beyond that. If a future change to mkdir.c needs more than
 * setmode()/getmode() from outside build() itself, this file will
 * fail to compile or link, loudly, and will need a matching update
 * here rather than silently testing something stale.
 */
#include <sys/types.h>
#include <sys/stat.h>

/*
 * mkdir.c calls these only from the -m path, never reached here;
 * declared and defined first so mkdir.c's own calls (which have no
 * prototype in scope there either, the same as the real build) bind
 * to these rather than colliding with an implicit declaration.
 */
void *
setmode(const char *p)
{
	return (void *)0;
}

mode_t
getmode(const void *set, mode_t base)
{
	return (base);
}

#define main unused_main		/* keep mkdir.c's own main() out of our way */
#include "../../usr/src/bin/mkdir/mkdir.c"
#undef main

#include <stdio.h>
#include <string.h>

static int fails;

/*
 * Run build() on a path held in a buffer with a known sentinel byte
 * right after the string's NUL, inside a scratch directory, and
 * report whether that sentinel survived (the real safety property:
 * build() must never write past the string's own NUL) and what
 * length the string ends up at. want_len is the length build() is
 * expected to leave the string at: for a path with no trailing
 * slash, that is the original length (the terminator must stay
 * exactly where it was); for a path the user gave a trailing slash,
 * the fix's same guard also skips restoring that slash once `last'
 * is set through it, so the string is legitimately one byte shorter
 * afterward -- harmless, since build() has already used the path by
 * then, and the caller never reads it again. Either way the sentinel
 * one byte further out must never move, which is what -- the only
 * thing -- the pre-fix bug actually broke.
 */
static void
probe(const char *dir, const char *rel, size_t want_len)
{
	char buf[256];
	size_t n;
	int r;

	snprintf(buf, sizeof buf - 1, "%s/%s", dir, rel);
	n = strlen(buf);
	buf[n + 1] = (char)0xAA;	/* the sentinel, one byte past the NUL */

	r = build(buf, 0700);

	printf("T build(\"%s\"): returned %d, sentinel %s, length %zu "
	    "(wanted %zu)\n",
	    rel, r,
	    (unsigned char)buf[n + 1] == 0xAA ? "intact" : "OVERWRITTEN",
	    strlen(buf), want_len);
	if ((unsigned char)buf[n + 1] != 0xAA || strlen(buf) != want_len)
		fails++;
}

int
main(void)
{
	char tmpl[] = "/tmp/mkdir_nul_test.XXXXXX";
	char *dir = mkdtemp(tmpl);
	size_t dirlen;

	printf("T begin\n");
	if (dir == NULL) {
		printf("T mkdtemp failed, skipping\n");
		printf("T end\n");
		return (0);
	}
	dirlen = strlen(dir);

	/* last component has no trailing slash: the case that corrupts. */
	probe(dir, "a/b/c", dirlen + 1 + 5);
	/* a second, independent one, to show it's not a one-off. */
	probe(dir, "x/y", dirlen + 1 + 3);
	/* a trailing-slash case: p ends up pointing at a real '/' that
	 * was turned into a NUL; with the fix, that slash is left as a
	 * NUL rather than restored (see the comment on probe() above),
	 * so this one is expected to come back one byte shorter. */
	probe(dir, "p/q/", dirlen + 1 + 3);

	printf("T %s\n", fails ? "FAIL" : "all checks passed");
	printf("T end\n");

	/*
	 * Clean up the directories build() actually created (not just
	 * the mkdtemp() top, which is empty on its own); noted by an
	 * independent reviewer that earlier runs left these behind.
	 * system() is fine here: this is a test program, not mkdir.c.
	 */
	{
		char cmd[320];

		snprintf(cmd, sizeof cmd, "rm -rf -- '%s'", dir);
		(void)system(cmd);
	}

	return (fails != 0);
}
