<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# `mkdir -p` writes '/' over the path's terminating NUL

plan-4.5.md stage 2, item 5; the sweep's #452 (`audit-bestfit.md`
item 10).

## The defect

`usr/src/bin/mkdir/mkdir.c`, `build()` (the `-p` implementation,
already carried in this tree from FreeBSD 2.2.5's rewrite -- see the
existing AI-ONLY NOTE above this patch's own, at the call site in
`main()`):

```c
for (first = 1, last = 0; !last ; ++p) {
	if (p[0] == '\0')
		last = 1;
	else if (p[0] != '/')
		continue;
	*p = '\0';
	if (p[1] == '\0')
		last = 1;
	...
	*p = '/';
}
```

`last` is set in two different circumstances that the final line
treats identically but should not: when `p` is sitting on a real `/`
between two components (the `p[1] == '\0'` case, meaning that `/` is
the path's own trailing slash), restoring `*p = '/'` is correct -- it
undoes the temporary `*p = '\0'` a few lines up, which only existed to
isolate the component for `stat()`/`mkdir()`. But when `p[0] == '\0'`
set `last` directly, `p` is pointing at the path string's own,
original terminating NUL -- there was no `/` there to restore. The
unconditional `*p = '/'` at the bottom of the loop runs in both cases
alike, so on the ordinary case of a path with no trailing slash (`mkdir
-p a/b/c`), it overwrites that terminator with `/`, leaving the
caller's `argv` entry without a NUL where the caller expects one.

Confirmed directly (see "The test" below): with the terminator
overwritten, the string is no longer properly terminated at its
original length, and reading it back (`strlen()`) runs on past the
overwritten byte into whatever follows in memory.

## The fit (first canonical fit) and the alternatives

This tree's `build()` is itself FreeBSD 2.2.5's (rev 1.5.8.1, 1997),
already noted in the file. The natural continuation is FreeBSD's own
later fix to the same function, not a switch to a different
implementation. FreeBSD's commit `16c0b5195c` (Mike Smith,
1998-10-22, "Honour the spirit rather than the implementation of the
previous changes..."), mixed in with an unrelated `chmod()` change for
`-m`, has:

```diff
-		*p = '/';
+		if (!last)
+		    *p = '/';
```

Checked the intervening FreeBSD commits between the 1997-04-01 commit
that introduced this exact `build()` shape and this one (`06f628852a`
1998-05-15, `2426ecdfa2` and `37a182e2b2`, both 1998-10-20) -- none of
them already has the guard, so `16c0b5195c` is the first.

NetBSD and OpenBSD's own `mkdir -p` (`mkpath()`, checked at OpenBSD's
1995-10-18 import, which came by way of NetBSD) is a structurally
different implementation (`strspn`/`strcspn` rather than a hand-walked
`first`/`last` state machine) -- but it has the *same class* of bug in
its own shape: `done = (*slash == '\0'); *slash = '\0'; ...; *slash =
'/';` restores unconditionally at the bottom of its loop too, for
exactly the analogous reason. This is not a shared defect by descent
(the two implementations don't share code), only a shared mistake, and
there is nothing to transplant from a different function shape into
this one; NetBSD/OpenBSD's own fix to their own `mkpath()`, if and
when it came, is not this tree's concern, since this tree's `build()`
is FreeBSD's lineage, not theirs.

## The fix

The same one line: `if (!last) *p = '/';`, with an AI-ONLY NOTE
explaining which of `last`'s two causes the guard is for.

## The test

`mkdir(1)` is a plain host program with no device dependency.
`build/test/mkdir_nul.c` pulls `build()` out of the real `mkdir.c`
with `#include` (so it's the exact, unmodified function under test,
not a retyped copy), stubbing `setmode()`/`getmode()` (the `-m` path,
never reached by a call to `build()` directly) so it links on the
host. It calls `build()` on a path held in a buffer with a known
sentinel byte placed one byte past the string's NUL, and checks that
byte is never disturbed and that the resulting string length matches
what `build()` should leave it at.

Three cases: two paths with no trailing slash (the bug's case), and
one with a trailing slash (included to show the fix doesn't disturb
that case, beyond the same guard also leaving that trailing slash as a
NUL rather than restoring it -- harmless, since nothing reads the path
again after `build()` returns, and this exact side effect is in
FreeBSD's own shipped fix too, not something introduced here).

| case | pre-fix | fixed |
|---|---|---|
| `a/b/c` (no trailing slash) | sentinel intact, but string reads back longer than it was (terminator overwritten; `strlen()` ran on into whatever followed) | sentinel intact, length unchanged |
| `x/y` (no trailing slash) | same corruption | sentinel intact, length unchanged |
| `p/q/` (trailing slash) | sentinel intact, length unchanged (this case was never buggy) | sentinel intact, length one byte shorter (the trailing slash is now left as a NUL rather than restored -- see above) |

Run against the pre-fix `build()` (`git show
HEAD:usr/src/bin/mkdir/mkdir.c`), the first two cases come back wrong
(exit 1, `FAIL`); against the fixed one, all three match
`build/test/mkdir_nul.want` exactly (exit 0).

Regression: the cross build (`cd usr/src/bin/mkdir && sh
../../../../build/make.sh`, after `touch mkdir.c` to force a rebuild)
compiles with no warnings, same as before this patch, and links a
32-bit i386 ELF binary.

## What is not established

- Whether the corrupted `argv` entry ever causes a visible problem in
  practice: `main()` doesn't read `*argv` again after `build()`
  returns for the `-p` path, so the corruption is latent here, the
  same way `if_ed`'s `sc->type` rewrite was latent -- a real exposure
  would need something past `argv[i]` in memory to matter, which this
  test doesn't attempt to characterize.
- NetBSD/OpenBSD's own fix to their analogous bug in `mkpath()`, if
  any, and when: not traced, since it's a different function this
  tree doesn't use.
- Whether FreeBSD's intervening `1a8d9bc1f7` (1997-04-01) or earlier
  commits for *other* files were checked exhaustively for this
  pattern; only `bin/mkdir/mkdir.c`'s own history was walked.
