# pwd: error() in a forked child, and the descendants' answer

`docs/provenance/stdarg.md` ends by noting a bug it uncovered and did
not chase: after `pwd`, the shell's standard output went silent for the
rest of the session while standard error kept working. This is that
bug. It is Berkeley's, not this port's, and both descendants had
already removed the code it lives in.

## The symptom

	# echo A
	A
	# pwd
	pwd: Cannot exec /bin/pwd
	# echo B
	#                              <- nothing, here and ever after
	# echo a | /bin/sh -c "cat"
	cat: not found                 <- standard error still fine

Reproducible from a clean boot. A failed exec of something *else* --
`nosuchcmd` -- does not do it, because `evalcommand` prints `not found`
for an unknown command in the parent and never forks. `pwd` is the
reachable case on this root image, and `pwd` is a builtin.

## Narrowing it, one boot per question

The whole diagnosis is four runs of the shell, each asking one thing.

**Is the terminal broken, or is it fd 1?**

	# pwd
	pwd: Cannot exec /bin/pwd
	# echo B
	# echo C >/dev/console
	C
	# exec 1>/dev/console
	# echo D
	D

The terminal is fine and reopening fd 1 fixes it outright, so the
tty, its process group and the `clist` are all innocent: the shell's
own file descriptor 1 is wrong.

**Closed, or pointing somewhere else?**

	# pwd
	pwd: Cannot exec /bin/pwd
	# exec 9>&1
	# echo probe-survived 1>&2
	probe-survived
	# echo Y 1>&9
	#

`exec 9>&1` does not fail, so fd 1 is open. Writing to the duplicate
produces nothing and does not kill the shell, so it is neither closed
(EBADF) nor a pipe whose reader has gone (EPIPE, and SIGPIPE). It is a
pipe with a reader that never reads.

**Are there two shells?**

	# echo A$$ 1>&2
	A3
	# pwd
	pwd: Cannot exec /bin/pwd
	# echo B$$ 1>&2
	B3

This measured nothing, and is recorded because it looked like it had.
ash sets `$$` once in `main` and never again, so a forked child reports
its parent's pid. The same answer is what a single shell and two shells
both print.

## The cause

`bin/sh/cd.c`'s `getpwd`, as Berkeley wrote it:

	INTOFF;
	if (pipe(pip) < 0)
		error("Pipe call failed");
	jp = makejob((union node *)NULL, 1);
	if (forkshell(jp, (union node *)NULL, FORK_NOJOB) == 0) {
		close(pip[0]);
		if (pip[1] != 1) {
			close(1);
			copyfd(pip[1], 1);
			close(pip[1]);
		}
		execl("/bin/pwd", "pwd", (char *)0);
		error("Cannot exec /bin/pwd");
	}
	close(pip[1]);
	pip[1] = -1;
	p = buf;
	while ((i = read(pip[0], p, buf + MAXPWD - p)) > 0
	     || (i == -1 && errno == EINTR)) {
		if (i > 0)
			p += i;
	}

The last line of the child is the bug. `error()` does not exit:

	exception = e;
	longjmp(handler->loc, 1);

and for a builtin, `handler` is `evalcommand`'s own `jmploc`, whose
`setjmp` the child inherited at the same stack address. So the child
longjmps into `evalcommand`'s `cmddone`, restores the handler, falls
through `FORCEINTON`, returns, and carries on as a second shell -- with
`pip[1]` still installed as its standard output.

That leaves the two processes deadlocked in a way neither reports:

- The parent waits in the `read` loop for end of file on `pip[0]`,
  which needs every write end closed. The child holds one as fd 1, so
  the parent never leaves `getpwd`.
- The child reads the terminal and is the shell the user is talking to.
  Its standard error is still the console, so prompts (`out2`) and
  error messages appear. Everything it prints to standard output goes
  into the pipe, where the wedged parent reads it into a 256-byte
  buffer nobody ever looks at. No error, no signal, no sign.

Every observation above follows: fd 1 open but not the terminal, writes
silently swallowed, stderr working, `exec 1>/dev/console` curing it.

`error()` in a forked child is only safe where the child's path reaches
`exitshell`, which is how `evalcommand`'s fork path for an external
command is written. `cd.c` is the one place in this shell that forks
from inside a builtin -- `forkshell`'s other callers are `eval.c` and
`redir.c`, all on paths that exit.

## The fix is the descendants', and the file is the same file

NetBSD's `bin/sh/cd.c` carries this file's own SCCS identifier,
`@(#)cd.c 8.2 (Berkeley) 5/4/95`, so it is the continuation of what is
here. Its `find_curdir` keeps the fork above only under `#else`, for
systems whose `getcwd` is itself a popen of `/bin/pwd`, and on NetBSD
takes a `getcwd` loop instead. It left the `error("Cannot exec
/bin/pwd")` line untouched in the dead arm. FreeBSD went further: its
`getpwd2` is nothing but that loop.

The condition the descendants test for holds here. This tree's own
`lib/libc/gen/getcwd.c` walks up with `opendir` and `readdir`, execs
nothing and forks nothing, so asking the C library costs no child.

Adopted, shrink-wrapped to this `getpwd`'s `void` shape -- it sets
`curdir` and errors rather than returning a pointer:

	STATIC void
	getpwd() {
		char *pwd;
		int i;

		if (curdir)
			return;
		INTOFF;
		for (i = MAXPWD ; ; i *= 2) {
			pwd = stalloc(i);
			if (getcwd(pwd, i) != NULL) {
				curdir = savestr(pwd);
				stunalloc(pwd);
				INTON;
				return;
			}
			stunalloc(pwd);
			if (errno != ERANGE)
				break;
		}
		INTON;
		error2("getcwd", errmsg(errno, E_OPEN));
	}

Measured against both donors, by line, ignoring comments and
conditionals: 9 of 20 lines identical to NetBSD's, longest identical
run 4 -- the `stalloc`, `getcwd`, `savestr`, `stunalloc` core, which is
the import. 3 of 20 identical to FreeBSD's, longest run 2. This is a
kind D transplant in `imports.md`'s terms, from a file with this file's
SCCS identifier, and it is said here rather than presented as new work.

The error line is this tree's idiom, not NetBSD's: NetBSD writes
`error("getcwd() failed: %s", strerror(errno))`, and nothing in this
shell uses `strerror` -- it has `errmsg(errno, action)`, and `E_OPEN`
is the action a directory read belongs to.

`nodes.h` and `jobs.h` go from `cd.c`'s includes with the fork; they
were there for `struct job`, `makejob`, `forkshell` and `waitforjob`,
and `getpwd` was their only user in this file. Checked by compiling.

## What this buys, besides the bug

One fork, one pipe, one job entry and the dependency on `/bin/pwd` all
leave the shell. `pwd` now works on a root filesystem that holds
nothing but `/sbin/init`, `/bin/sh` and `/dev/console`, which is what
this port actually boots.

	# pwd
	/
	# cd /bin
	# pwd
	/bin
	# cd /dev
	# pwd
	/dev
	# cd ..
	# pwd
	/
	# cd /sbin && pwd
	/sbin
	# cd /nosuch
	cd: can't cd to /nosuch
	# pwd
	/sbin
	# echo still-alive
	still-alive

and the mixed run from `stdarg.md` that first showed the fault now
passes end to end, `pwd` included.

## Reviewed, and what the review found

The change was reviewed against the code by a reader that had not seen
it written. The `getpwd` body came back sound on every point asked:
`INTOFF`/`INTON` balance on all three paths; `savestr` before
`stunalloc` correct, because `savestr` uses `ckmalloc` and not the
stack allocator, so the copy outlives the `stunalloc`; `stunalloc`'s
argument the most recent `stalloc` every time round the loop; `errno`
still the one `getcwd` set by the time `error2` reads it, since
`stunalloc` and `INTON` make no system call and `getcwd`'s error path
only calls `free`; `error2` and `errmsg` signatures matching
`error.h`; `getpwd` called only from `pwdcmd`, never with a string
under construction on the stack allocator, so the new `stalloc` cannot
shift `stackblock()` under anyone. `i *= 2` cannot run away in
practice, since `ERANGE` is bounded by the length of a path; the cost
of a retry is one `stack_block` left for `popstackmark`.

One behaviour change that is not a bug, but is a change. This tree's
`getcwd` begins by trusting `$PWD` when it stats equal to `.`
(`getcwd.c:69`), so `pwd` can now report a logical path where
`/bin/pwd` always reported the physical one. Nothing in this system
sets `PWD` -- not the shell, which never names it, and not `init` --
so it takes an inherited value to see a difference, and preferring
`$PWD` is what POSIX asks of `pwd` without `-P` anyway.

## Noted, not chased

	# trap "echo trapped" USR1
	trap: Illegal number: USR1

This shell's `trap` takes signal numbers only. POSIX requires names,
so it is a real conformance gap rather than a bug, and it belongs with
the other POSIX work rather than here.
