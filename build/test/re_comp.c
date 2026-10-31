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
 * test for lib/libcompat/4.3/regex.c's two fixes
 * (docs/provenance/smaller-fixes.md):
 *
 *   1. re_comp() and re_exec() took a plain `char *', not matching
 *      this tree's own usr/src/include/unistd.h prototype, which
 *      already declared both `const char *'.
 *   2. re_comp("") recompiled an empty pattern -- which matches any
 *      string -- instead of leaving the last compiled expression
 *      alone, as re_comp.3 says it must: "If re_comp is passed 0 or
 *      a null string, it returns without changing the currently
 *      compiled regular expression."
 *
 * re_comp()/re_exec() have no side effects reaching the filesystem
 * or the network (just two static globals and a call into
 * lib/libcompat/regexp/regexp.c's regcomp()/regexec()), so this
 * #includes both real source files textually and calls the real
 * functions directly -- not a copy that can drift. The const fix
 * (hole 1) is exercised simply by this file's own prototypes above
 * the #includes matching unistd.h's exactly: a stale plain `char *'
 * definition below would conflict with them and fail to build,
 * which is checked for directly in build/test/re_comp.sh via a
 * grep of unistd.h's own prototypes rather than repeating them here
 * (so the two can't quietly drift apart). Hole 2 is exercised with
 * the actual compiled-regex behaviour change: a regex compiled
 * before the empty re_comp() call must still be the one re_exec()
 * uses afterward.
 *
 * usr/src/include/regexp.h is this tree's own header, unchanged by
 * this patch; it's copied into this test's own build (see the .sh
 * wrapper) rather than built against via -I usr/src/include
 * directly, because the latter also makes the host's own
 * <stddef.h>/<sys/types.h> resolve to this tree's BSD-specific ones
 * (which pull in <machine/ansi.h>, not present on the host) ahead of
 * the system's.
 */
#include <stdio.h>
#include <string.h>

/* AI-ONLY NOTE: matches usr/src/include/unistd.h's own prototypes
 * exactly (checked by build/test/re_comp.sh); if regex.c's own
 * definitions below don't match these, the build fails.
 */
char *re_comp(const char *);
int re_exec(const char *);

#include "../../usr/src/lib/libcompat/4.3/regex.c"
#include "../../usr/src/lib/libcompat/regexp/regexp.c"

static int fails;

static void
check(const char *label, int got, int want)
{
	printf("T %s: got %d, want %d -- %s\n", label, got, want,
	    got == want ? "ok" : "FAIL");
	if (got != want)
		fails = 1;
}

int
main(void)
{
	char *err;

	/*
	 * Hole 2: a regex compiled now must still be the one in effect
	 * after a later re_comp("") or re_comp(NULL) -- not replaced by
	 * an empty pattern, which (post-fix or pre-fix both) matches
	 * any string at all, so a wrongly-replaced pattern would make
	 * re_exec() start reporting a match on input it must not match.
	 */
	err = re_comp("^foo[0-9]+$");
	check("first re_comp succeeds", err == NULL, 1);
	check("matches foo123", re_exec("foo123"), 1);
	check("rejects bar", re_exec("bar"), 0);

	err = re_comp("");
	printf("T re_comp(\"\") returns: %s\n", err ? err : "(null)");
	check("re_comp(\"\") itself reports no error", err == NULL, 1);
	check("still rejects bar after re_comp(\"\")", re_exec("bar"), 0);
	check("still matches foo123 after re_comp(\"\")", re_exec("foo123"), 1);

	err = re_comp(NULL);
	check("re_comp(NULL) also reports no error", err == NULL, 1);
	check("still rejects bar after re_comp(NULL)", re_exec("bar"), 0);

	/* A genuinely new pattern must still take effect normally. */
	err = re_comp("^bar$");
	check("re_comp of a real new pattern succeeds", err == NULL, 1);
	check("now matches bar", re_exec("bar"), 1);
	check("now rejects foo123", re_exec("foo123"), 0);

	if (fails)
		printf("T FAIL\n");
	else
		printf("T all checks passed\n");
	return fails;
}
