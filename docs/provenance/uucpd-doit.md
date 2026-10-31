# `uucpd` changes directory as root and never sets the login name

plan-4.5.md stage 2, item 5; the sweep's #586 (`audit-bestfit.md`
item 7). Two unrelated bugs in `doit()`, folded into one patch
because the sweep folded them into one item.

## Bug 1: `chdir()` runs before `setuid()`, so it runs as root

`usr/src/libexec/uucpd/uucpd.c`, `doit()`, after a successful
login:

```c
initgroups(pw->pw_name, pw->pw_gid);
chdir(pw->pw_dir);
setuid(pw->pw_uid);
```

`chdir()`'s directory-permission check is bypassed entirely for
root (the same as any file-access check): root can `chdir()` into
any directory regardless of its mode, while an ordinary user cannot
enter one they lack search permission on. Doing `chdir(pw->pw_dir)`
*before* `setuid(pw->pw_uid)` means it always runs as root, so it
always succeeds -- even into a `pw_dir` the uucp account itself has
no permission to enter at all (moved, mode changed, anything). The
check `chdir()` is supposed to make on the uucp user's behalf is
silently skipped.

## Bug 2: the session's login name is never recorded

Nothing in `doit()` ever calls `setlogin(2)`. The login name a
session is attributed under (what `getlogin(2)` returns, read by
programs like `who(1)` and `w(1)`) stayed whatever `uucpd`'s own
parent process happened to have -- not the uucp user who had just
authenticated over the wire.

## The fit (first canonical fit)

Both checked directly against the donor trees. OpenBSD has both,
each in its own small, clean, non-synthetic commit:

| candidate | date | what it has |
|---|---|---|
| OpenBSD, `d336ec13856` (deraadt, "do not chdir as root") | 1996-07-22 | swaps the order: `setuid()` before `chdir()` |
| OpenBSD, `9ff5160c91a` (deraadt, "setlogin") | 1996-09-02 | adds `setlogin(user);` right after `dologin()` |
| NetBSD 1.2 (`origin/netbsd-1-2`) | -- | has both bugs: no `setlogin()`, `chdir()` before `setuid()` |
| NetBSD 1.3 (`origin/netbsd-1-3`), per its own `$NetBSD: uucpd.c,v 1.7 1997/10/13 09:28:11 lukem Exp $` | 1997-10-13 | adds `setlogin()`, but **still** calls `chdir()` before `setuid()` -- bug 1 is not fixed here |
| NetBSD 1.4 (`origin/netbsd-1-4`), per its own `$NetBSD: uucpd.c,v 1.14 1998/07/03 18:09:49 mrg Exp $` | 1998-07-03 | a larger rewrite (syslog, a single compound permission check) that **still** orders `chdir()` before `setuid()` in that compound check -- bug 1 is not fixed in any of this tree's NetBSD snapshots |

OpenBSD is earliest for both halves, and -- unlike `at`'s Y2K fix in
the previous patch -- NetBSD never independently fixes the `chdir`
ordering at all in the snapshots checked here, so there is no
competing earlier fit for that half to weigh. `git log`/`git blame`
on these OpenBSD commits is clean and non-synthetic (unlike the
recurring mirror-graph irregularity seen on several other files in
this tree), so both dates and authors are taken directly, not
reconstructed from an embedded keyword.

## The fix

`setlogin(user);` added right after `dologin(pw, sinp);` (before
`setgid()`), and `setuid(pw->pw_uid);` moved to run before
`chdir(pw->pw_dir);` (previously the reverse) -- matching OpenBSD's
two commits exactly, each with its own AI-ONLY NOTE. `setlogin()` is
already declared in this tree's own `usr/src/include/unistd.h`;
`uucpd.c` already includes `<unistd.h>`, so nothing else changes.

## The test

`doit()` itself can't be called in a test: it reads a login and
password from fd 0, and on success really calls `setuid()` and
`execl()` against the real `uucico` binary -- not something to run
inside a test program. Instead, `build/test/uucpd_doit.c`
reproduces the actual security property the fix establishes, using
real `chdir()`/`setuid()` calls against a real, restrictive
directory -- not a mock of the two calls, which could only ever
assert on the order they were written in, not on what either call
actually does:

- A scratch directory is created (owned by root, since the test
  itself must run as root) with mode `0700` -- readable, writable,
  and searchable by no uid but its owner.
- Two forked children each try to `chdir()` into it as uid 1
  ("daemon" on this system; not 0, not the test's own uid), one in
  the bug's order (`chdir()` while still root, then drop to uid 1),
  one in the fixed order (drop to uid 1, then `chdir()`).
- The bug order's `chdir()` always succeeds (root bypasses the
  mode-0700 check); the fixed order's `chdir()` gets `EACCES`, the
  real kernel enforcing the real permission bit, for the real
  reason `chdir(pw->pw_dir)` is supposed to be checked at all.

`build/test/uucpd_doit.sh` first greps the real `uucpd.c` to confirm
`setuid()` still runs before `chdir()` (by line number) and
`setlogin(user)` is still present, so the test can't drift from
source; then, because the whole point only shows up under real
root's permission bypass, it checks `id -u` and skips cleanly
(exit 0, with a message, not a failure) when not run as root, rather
than reporting a false pass or a confusing failure. This container
runs as root, so the check actually runs here.

Ran it three times in a row with identical output (no forking
races of the kind that bit `build/test/sa_argv.c` earlier in this
project -- each order here runs to completion in its own forked
child before the parent reads anything).

Shown to fail before the fix, on both halves, without touching the
real source for longer than the check itself: reordered `setuid()`
and `chdir()` back to the pre-fix sequence in a scratch copy and
reran `build/test/uucpd_doit.sh` -- the drift-check caught it and
stopped before building ("`uucpd.c` calls `chdir()` ... before
`setuid()` ... -- the pre-fix order"). Separately removed the
`setlogin(user);` line and reran -- caught the same way ("`setlogin
(user)` not found"). Restored `uucpd.c` from a saved copy both
times and diffed it back to byte-identical with the saved copy
before continuing.

Regression: cross build verified clean,
`cd usr/src/libexec/uucpd && sh ../../../../build/make.sh`.

## What is not established

- Real behavior of a complete `uucpd` session (a real login,
  password check, and `uucico` handoff over a real socket): not
  staged; the `chdir`/`setuid` ordering is tested against the real
  kernel's real permission enforcement, but not through a full
  `doit()` call.
- Whether NetBSD ever independently fixes the `chdir`-before-`setuid`
  ordering in a later release than the two snapshots checked here
  (1.3, 1.4): not traced further, since OpenBSD's fix is already
  established as earliest regardless.
- FreeBSD's and other donors' history for this file: not checked,
  since OpenBSD already has both fixes earliest and first in donor
  order, so a later or equal fit elsewhere would not change the
  outcome.
