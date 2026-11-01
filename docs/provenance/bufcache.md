<!-- SPDX-FileCopyrightText: 2026 Nicholas Martin -->

# The buffer cache: NetBSD 1.1's corrections

Done 2026-10-31 (patch 248; reviewed twice, see the commit message), plan-4.5.md stage 2, item 3; the sweep's
#252 and #254 (`audit-bestfit.md`).

## The defects

`kern/vfs_bio.c` here is NetBSD 1.0's, byte for byte in the functions
that matter (checked with `diff` against the donor tree). NetBSD 1.1
corrected five of them:

- **`bwrite`** set `B_AGE` on the buffer after `VOP_STRATEGY` of an
  asynchronous write. The write's completion (`biodone`, at interrupt
  level) may already have released the buffer, so the flag went onto a
  buffer on the free list or in someone else's hands. 1.1 returns
  without touching it.
- **`getblk`** looked up a buffer with `incore()`, which skips invalid
  ones, so NFS, which sets `B_INVAL` on data it is still writing,
  could get a second buffer for a block whose write was in flight. 1.1
  walks the hash chain itself and tests busy before invalid.
- **`getnewbuf`** left a reused buffer on its old hash chain (`getblk`
  moved it afterwards); 1.1 takes it off at once. **`allocbuf`**
  rehashed a buffer it had stripped only when nothing was left of it;
  1.1 always puts it on the invalid chain. **`geteblk`** called
  `bremhash` itself, which with `getnewbuf` now doing it removes the
  buffer twice (harmless, since `LIST_REMOVE` repeats the same stores,
  but unsound); 1.1 drops that call and the three lines that zeroed
  `b_bcount`, `b_error` and `b_resid` (`getnewbuf` zeroes them). One
  consequence: a `geteblk` buffer now has `b_bcount` equal to the size
  asked for, where it had 0. Every caller on this machine (the disk
  label, slice and bad-sector readers in `diskslice_machdep.c`,
  `ufs_disksubr.c`, `subr_dkbad.c`) sets `b_bcount` itself; the other
  ports' callers (hp300 `ct.c`, luna68k `sc.c`, vax `crl.c`) were not
  checked.

## The fit (first canonical fit)

Lite2's own `vfs_bio.c` is encumbered and not read (`precedent.md`);
the file here is NetBSD 1.0's, so the first continuation of the line is
NetBSD 1.1, which is the first release with these corrections (the
sweep's finding, #252; NetBSD 1.2 differs from 1.1 in these functions
only by `static`, `curproc` fallbacks and `splbio` around the
accounting). The five functions are taken as 1.1 has them, because 1.1's `getblk`,
`getnewbuf`, `allocbuf` and `geteblk` hunks depend on one another
(`getblk` no longer rehashes; `getnewbuf` does). Left out, as features
and not fixes:

| in 1.1 | why not taken |
|---|---|
| `bwrite` turns a synchronous write on an `MNT_ASYNC` mount into a delayed write | a feature |
| `bio_doread`, the shared read helper | a refactor of `bread`/`breadn` |
| `bdwrite` tests `bdevsw[].d_type == D_TAPE` for tape | not a hole; this tree's `B_TAPE` form is kept |

The alternatives, measured in the donor trees:

| donor | what it has |
|---|---|
| NetBSD 1.0 | the code as it is here: both holes |
| **NetBSD 1.1** (vfs_bio.c 1.39) | the corrections above; taken |
| NetBSD 1.2 | `getblk`, `getnewbuf` the same as 1.1; `allocbuf` one blank line; `bwrite` adds a `curproc` fallback to `proc0`, and wraps `reassignbuf`, the accounting and `v_numoutput++` in `splbio` (see below); `bdwrite` the same wrapping and `CLR(B_NEEDCOMMIT)` |
| NetBSD trunk 1.32 to 1.38 (between 1.0 and 1.1) | **not checked**: the donor histories are shallow or need the network. A fix there would be earlier |
| OpenBSD, first import (1995-10-18, rev 1.39) up to 1996-05-02 | byte-identical to NetBSD 1.1 in these functions; no `splbio` in `bwrite`. Not earlier |
| FreeBSD 2.0 (1994-11-22, Dyson) | a re-shaped file: its `bwrite` does not touch `B_AGE` after `VOP_STRATEGY`, so it has no such hole, but `incore` still skips `B_INVAL`, so the NFS ordering is not fixed there; not a continuation of this code |
| FreeBSD 1.1.5.1 (`vfs__bio.c`) | a synchronous-only `bwrite` |

FreeBSD is left out because its buffer cache is a different design, not
a correction of this one (the reviewer's reading; not an exhaustive
comparison).

**Not taken: NetBSD 1.2's `splbio` around `reassignbuf`.** In this tree
`reassignbuf` (vfs_subr.c) has no spl, and in `bwrite`'s asynchronous
delayed-write branch it runs at spl0; a completion's `brelse` calling
`brelvp` on the same vnode could interleave with it and corrupt the
vnode's buffer lists. That is a reading of the code, plausible and
narrow, not shown, and it is NetBSD 1.2's fix, not 1.1's. It is recorded
in `deferred.md` rather than taken untested.

## The test, and what it can show

`build/test/bufwrite.c` (run by `ktest.sh bufwrite`): creates 120 files
on a writable root (remounted through `/d/wd0a`), calling `sync()`
every ten so that delayed writes are started asynchronously, then reads
every file back and compares it, removes half and checks the rest. A
smoke test of the boot-time paths: its working set (about 0.4 MB) is far
below the cache (about 6.7 MB at boot: 827 buffers), so buffers are not
reclaimed and `getnewbuf`'s delayed-write branch and `allocbuf`'s growth
are barely run; the read-back is served from the cache.

| kernel | result |
|---|---|
| patch 243 (before) | PASS |
| patch 243 + 1.1 functions (the first form of this patch) | PASS |
| the final tree (the five functions, with `geteblk`) | PASS |

**The race, by instrument** (`KTEST_LOG` keeps the console). A kernel
built from the patch-243 `bwrite` with a `printf` in the asynchronous
delayed-write branch, after `VOP_STRATEGY` and before `SET(B_AGE)`,
that says `RACE` if the buffer is no longer `B_BUSY` (a released
buffer: `brelse` clears it; the kernel is uniprocessor and
non-preemptive, so only interrupt-level code runs in the window, and it
cannot take the buffer back):

| window | checks run | released before the check |
|---|---|---|
| as in the code | 191 | 0 |
| widened by a busy loop after `VOP_STRATEGY` | 191 | 191 |

So the check can fire, the race is real once the window is wide enough,
and in the unwidened code on QEMU's IDE it did not occur in 191 tries.
The test above does not fail without the fix; the fix is the donor's.

## What is not established

- The race and the NFS ordering fix are untested: no NFS exercise, no
  reproduction of the `bwrite` race.
- That 1.1's `getblk` is better than 1.0's under load beyond the one
  workload above.
- The effect of dropping `B_AGE` from asynchronous delayed writes
  (they now age as ordinary buffers); 1.1 does the same.

Also run on the final kernel: `copyio.sh` (22 lines), `elfshort.sh`
(6 lines) and the installed system boots multi-user (no panic).
