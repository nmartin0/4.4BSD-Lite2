# The ELF loader accepts a short read

Done 2026-10-30 (patch 246), plan-4.5.md stage 2, item 2; the sweep's
#207 (`audit-bestfit.md`).

## The defect

`elf_read_from` (`kern/exec_elf.c`), which `execve` uses to read the
program headers, read with `vn_rdwr` and, when the read came back short
(`resid != 0`), returned `error`, which is 0 at that point. A file
whose program headers lie past its end was therefore taken as good and
`execve` used whatever the kernel's buffer held as headers.

## The fit (first canonical fit) and the alternatives

Lite2 has no ELF loader, so there is nothing earlier in source order.
The donors, from what was read in the donor trees (dates are commit
dates in the donor repositories; **release dates are from memory and
unverified**):

| candidate | what it has |
|---|---|
| NetBSD 1.0 | no `exec_elf.c` at all (`git show origin/netbsd-1-0:sys/kern/exec_elf.c` fails) |
| NetBSD, branch `netbsd-1-1` (tip 1996-04-23, a post-release snapshot) | `elf_read_from` with the bug: `if (resid != 0) return error;` |
| OpenBSD, commit `b16105e3033` (1996-01-29, "from netbsd: ... Incorporate some fixes") | `if (resid != 0) return ENOEXEC;`, the same line, imported from NetBSD; OpenBSD's first release (2.0) is, from memory, after NetBSD 1.2 |
| NetBSD 1.2 (branch `netbsd-1-2`, a single 1997-03-19 squash, so the fix's own date is not recoverable) | `if (resid != 0) return ENOEXEC;` |
| FreeBSD `imgact_elf.c` (earliest 1996-10-24, in 2.2.0) | a different loader with no `elf_read_from` and no such check |

The text of the fix is identical in OpenBSD's 1996 commit and NetBSD
1.2, and the result is the same one line either way, so the choice
between them changes the credit and nothing in the tree. It is credited
to NetBSD (OpenBSD's own commit says it came from NetBSD) and read from
`netbsd-1-2`. NetBSD 1.2 also changed the read's flags from
`IO_NODELOCKED` to 0; that is NetBSD's locking (this tree's `execve`
holds the vnode lock, and `vn_rdwr` would lock it again) and is not
taken. The smallest change is the one line. FreeBSD's loader is a
different design and carries the fix nowhere to take.

## The test

`build/test/elfshort.c`, run by `build/test/ktest.sh elfshort` (the
general harness, which `copyio.sh` now calls too): as `/sbin/init` it
makes a whole copy of init, a copy cut to its 52-byte header with the
program headers placed at the end of file, and one cut so the headers
start inside the file and run past its end, and execs each.

| case | kernel at patch 243 (measured 2026-10-31) | with the fix |
|---|---|---|
| intact copy | runs | runs |
| headers past end of file | exec succeeds (errno 0) | ENOEXEC |
| headers cut by end of file | exec succeeds (errno 0) | ENOEXEC |

So the test fails without the fix and passes with it. The kernel at
243 let the child exec garbage headers and carry on; what it would have
done with different buffer contents is not known (it could fault or load
something), only that it did not refuse.

Regression: `copyio.sh` passes on the fixed kernel (22 lines); the
installed system (from the earlier CD install) boots multi-user on it,
which execs a few hundred programs; no panic.

## What is not established

- Behaviour with buffer contents that happen to look like valid headers
  (the headers are uninitialised kernel memory; not constructed).
- Whether OpenBSD's first import (1995-10-18) had the bug: not diffed.
- The second read in the loader, `vmcmd_map_readvn`, passes `IO_UNIT` and
  no `resid`, so a short read is already an error there; not tested.

## Correction to the test (patch 247)

The first form of `elfshort.c` made the root file system writable with a
remount that named no device (`fspec` 0). That worked for the few writes
the test makes, but a later test (`bufwrite.c`) showed that the state it
leaves hangs the kernel under any real write load: the guest sat idle
with a process asleep on a buffer nobody would release. The reviewer had
asked whether the trick was sound; it was not. Both tests now remount
through a block device node, `/d/wd0a`, which `ktest.sh` puts on the
disk (a second directory because `addfile-host` makes a new directory
entry for each pair it is given). `elfshort` was re-run on that: the
patch-243 kernel fails on the same two lines as before, the fixed kernel
passes. The hang itself, on a half-remounted root, is not investigated
(`deferred.md`).
