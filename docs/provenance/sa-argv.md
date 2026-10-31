# `sa` passes `char **` to `%s`

plan-4.5.md stage 2, item 5; the sweep's #512 (`audit-bestfit.md`
item 11).

## The defect

`usr/src/usr.sbin/sa/main.c`, `main()`'s per-file loop (`for (; argc
> 0; argc--, argv++)`), where a failing `ftruncate()` or `close()`
on the current accounting file is reported:

```c
if (ftruncate(fd, 0) == -1) {
	warn("couldn't truncate %s", argv);
	error = 1;
}
...
if (close(fd) == -1) {
	warn("close %s", argv);
	error = 1;
}
```

`argv` here is `char **`, the loop's own cursor into `main`'s argv
array as it advances (`argc--, argv++`) -- not the file name. `%s`
against a `char **` prints whatever bytes happen to sit at the
pointer's own value, read as if they were a C string: garbage, not
the path that actually failed to truncate or close. The file name
`sa(8)` means to report is `argv[0]`, i.e. `*argv`.

## The fit (first canonical fit)

Lite2's import has the bug, unchanged from 4.4BSD-Lite2. Checked
directly against the donor trees in `/home/claude/donors/netbsd`:

| candidate | what it has |
|---|---|
| NetBSD 1.2 (branch `origin/netbsd-1-2`) | has the bug, both calls: `warn("couldn't truncate %s", argv);` / `warn("close %s", argv);` |
| NetBSD 1.3 (branch `origin/netbsd-1-3`) | has the fix, both calls: `warn("couldn't truncate %s", *argv);` / `warn("close %s", *argv);` |

OpenBSD is first in donor order, but at the point this fix must have
landed (1996-1997) OpenBSD's own `sa` tree descends from this same
NetBSD code with no independent history to check ahead of NetBSD
here; 386BSD does not carry `usr.sbin/sa` at all. NetBSD 1.3 is the
earliest fit found, and donor order does not change that, since no
earlier donor has it.

**The exact fixing commit is not recoverable from this mirror.**
`git blame` on NetBSD 1.3's `main.c` attributes both fixed lines to a
single synthesized commit, `8b8d5a4d4`, dated 2001-01-18, whose own
message ("oops, undo previous change, which was committed
accidentaly here") plainly has nothing to do with this fix -- the
same cvs2svn mirror-graph irregularity already recorded for NetBSD
1.2 in `elf-short-read.md` and for FreeBSD in `if_ed-assign.md`:
branch and tag creation gets squashed into commits that carry an
unrelated stated purpose, so the graph cannot be used to date the
real change. What is established directly from branch content, not
from blame or log: NetBSD 1.2 has the bug, NetBSD 1.3 has the fix.
The exact date and author are left unconfirmed rather than guessed.

The fix itself is one character, twice (`argv` to `*argv`): there is
no design choice to weigh.

## The fix

Both `warn()` calls changed from `argv` to `*argv`, with an AI-ONLY
NOTE at the first explaining the pointer mistake and citing this
file.

## The test

Reaching these two calls through a real `sa(8)` run needs a live,
privileged accounting file and a forced `ftruncate()`/`close()`
failure on it, which is not practical to stage here, so
`build/test/sa_argv.c` reproduces the exact mistake directly: it
builds the same local shape (an `argv` array holding one file name,
`"/var/account/acct"`) and two small functions, `bug_form()` (`%s`
against `argv`) and `fixed_form()` (`%s` against `*argv`), each
calling libc's own `warn()` exactly as `main.c`'s calls do. Each
form is run in a forked child with its stderr captured through a
pipe read to EOF in the parent, and the captured text is checked for
whether the real path, `/var/account/acct`, appears in it.

`build/test/sa_argv.sh` first greps the real `main.c` to confirm
both calls still read `*argv` and the pre-fix `argv`-only form is
gone, so the test can't quietly drift from what `main.c` actually
says, then builds and runs the harness with the host's own `cc`.

| form | stderr shows the real path |
|---|---|
| `bug_form` (`argv`) | no -- prints whatever bytes are at the pointer's value instead |
| `fixed_form` (`*argv`) | yes, every time |

So the test fails to show the path on the pre-fix statement and
passes on the fixed one, run back to back in the same program.

### A test-harness bug found and fixed before this was shown to a reviewer

The first version of `capture()` read the child's stderr with a
single `read()` call. That is wrong for a pipe: a single `read()` is
not guaranteed to return everything the writer wrote, only what
happens to be available at that moment, and running the test
repeatedly showed exactly that -- the deterministic `fixed_form`
check (which should always find the path, since `*argv` is always
exactly `"/var/account/acct"`) flipped between "yes" and "no" across
runs, because a short first `read()` could return before the
child's full `warn()` output had landed in the pipe, truncating the
captured buffer right where the path would have matched. This was
caught here, before any reviewer saw the test, by running it
repeatedly and noticing the flicker; fixed by looping `read()` until
the child closes its write end (EOF) instead of trusting one call.
Re-run 30 times in a row afterward with no flicker (`fixed_form`
"yes" on all 30; `bug_form` "no" on all 30, as expected since its
output is pointer-garbage that essentially never happens to contain
the literal path text).

Confirmed the test actually exercises the real source, not just its
own copy of the mistake: temporarily reverted the committed fix in
`main.c` (restored `argv` in place of `*argv` in the truncate call)
and reran `build/test/sa_argv.sh` -- the script's own drift-check
grep caught it and stopped before even building the harness
("`main.c`'s truncate `warn()` no longer uses `*argv`"), then the
real fix was restored and the file was confirmed to match the
intended patch diff before continuing.

Regression: cross build verified clean,
`cd usr/src/usr.sbin/sa && sh ../../../../build/make.sh`, with
`argv`/`*argv` the only functional change against the Lite2
original.

### Correction (same patch, before delivery)

This doc's first version described the per-file loop as `for (;
*argv; argc--, argv++)`. The real loop (`main.c` line 216) tests
`argc > 0`, not `*argv`; an independent reviewer checked the loop
line directly and caught the mismatch. It doesn't change which
pointer `argv` is or why the bug is a bug -- `argv` is still the
cursor the loop advances with `argv++`, just under a different
continuation test than first written here -- but the line as stated
was wrong against the actual source, and is corrected above.

## What is not established

- The exact commit and author that introduced the fix on NetBSD's
  side: only that NetBSD 1.2 has the bug and NetBSD 1.3 has the fix,
  per the mirror-graph irregularity above.
- Whether OpenBSD's own `usr.sbin/sa` ever carried this bug
  independently of NetBSD's tree, or merely inherited NetBSD's
  history at this point: not traced further, since it does not
  change which donor is earliest here.
- Real behavior of a live, privileged `sa(8)` run hitting a genuine
  `ftruncate()`/`close()` failure on an accounting file: not staged;
  the test reproduces the mistake's mechanism in isolation, not a
  full `sa(8)` invocation.
