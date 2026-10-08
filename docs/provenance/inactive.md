# Lite2's inactive subsystems, judged by what the descendants did

Twice now this project has found a subsystem complete in the tree and
adopted by only some ports: `sys/device.h` with `kern/subr_autoconf.c`,
which only sparc used, and `miscfs/procfs`, which only hp300 and pmax
have the machine-dependent half for. Both were found by accident, when
something the i386 lacked turned out to exist already.

This surveys them deliberately, and judges each not by reading it but
by what NetBSD and FreeBSD did with it **after** they had integrated
4.4BSD-Lite. If both descendants kept and grew a thing, it was the
direction; if both dropped it, it was not; if they split, that is
worth knowing before building on it.

The releases used are NetBSD 1.3 and FreeBSD 2.2, two to three years
after Lite2, by which time each had merged and lived with the Lite
code.

## The stackable filesystem layer

`miscfs` is **13,066 lines across ten filesystems with no stubs at
all**, and most are configured by no port in this tree:

| filesystem | lines | ports configuring it here |
|---|---|---|
| `union` | 3,238 | 6 |
| `procfs` | 2,444 | 3 |
| `umapfs` | 1,320 | 0 |
| `nullfs` | 1,287 | 0 |
| `fdesc` | 1,201 | 0 |
| `kernfs` | 1,017 | 0 |
| `portal` | 1,002 | 6 |
| `specfs` | 686 | 0 |
| `fifofs` | 515 | 0 |
| `deadfs` | 356 | 0 |

**All ten survive in both NetBSD 1.3 and FreeBSD 2.2.** Not one was
dropped. And they grew modestly rather than being rewritten --
`fdesc_vfsops.c` goes 250 to 316 and 314, `kernfs_vfsops.c` 257 to 348
and 381, `null_vfsops.c` 367 to 383 and 390, `procfs_vfsops.c` 193 to
261 and 280, `union_vfsops.c` 495 to 551 and 578 -- which is two or
three years of maintenance on a design both kept, not replacement.

So the stackable vnode layer is the direction 4.4BSD was choosing and
both descendants confirmed it. The copies here are mature enough to
configure: this tree is not holding an early sketch of something that
got rebuilt later.

## The autoconfiguration framework

`kern/subr_autoconf.c` is 345 lines here and three ports reference
`cfdriver`. Afterwards:

| tree | lines | `cfdriver`/`config_found` references |
|---|---|---|
| Lite2 | 345 | 2 |
| NetBSD 1.3 | 609 | 4 |
| FreeBSD 2.2 | 342 | 2 |

**The descendants split.** NetBSD adopted it and built on it,
nearly doubling the file. FreeBSD carried it at essentially Lite2's
size and did not -- and later replaced it with newbus entirely.

That is a weaker signal than the filesystems, and an honest reading is
that `cfdriver` was NetBSD's bet rather than everyone's. It does not
argue against converting the i386 to it, which `deferred.md` already
records as wanted, but it does mean the conversion is following
NetBSD's line rather than a settled consensus.

## How to use this

The test is cheap and worth repeating before building on anything in
this tree that looks unused: fetch the same file from NetBSD 1.3 and
FreeBSD 2.2 and compare. Three outcomes, three meanings.

- **Both kept and grew it.** The direction was real. Build on it.
- **Both dropped it.** 4.4BSD was alone, and the thing is a dead end
  whatever it looks like from inside.
- **They split.** Whichever is followed, the commit should say which,
  because it is a choice rather than a restoration.

What this survey does not do is tell you whether a subsystem *works*
here. `procfs` is complete, both descendants kept it, and it still
cannot trace a process on this port, because the machine-dependent
half is unwritten and `trace_req` is a stub --
`docs/provenance/procfs.md` has that audit. Adoption by the
descendants is evidence about the design, not about this tree.
