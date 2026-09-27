# Where this tree contradicts itself, and which spelling to follow

4.4BSD-Lite2 was written over fifteen years by many hands, and in
places it holds several spellings of the same thing. They are not
competing conventions to choose between: usually they are snapshots of
one convention at different dates, frozen when each file was written.
This file records the ones met so far, so that a reader who finds an
older form does not take it for a style worth imitating.

## Declaring ttrstrt, and prototypes generally

<sys/tty.h> declares

	void	 ttrstrt __P((void *tp));

It gained that line on 5 September 1993, in a CSRG commit titled
"Cleanups for 4.4BSD-Lite" that touched tty.h and nothing else. Every
driver written before then carried its own declaration, and none was
updated afterwards. So the tree holds four spellings, and their dates
explain them:

  int ttrstrt();			May 1991
	i386/isa/pccons.c (corrected here), vax/uba/qv.c,
	news3400/bm/bmcons.c, tahoe/vba/vx.c, tahoe/vba/mp.c

  extern void ttrstrt();		June 1992
	news3400/iop/rs.c, from the initial port by Kazumasa Utashiro

  extern void ttrstrt(void *);		July 1992
	sparc/rcons/rcons_kern.c

  extern void ttrstrt __P((void *));	November 1992
	pmax/dev/scc.c

  none at all, taken from the header	September 1993 onwards
	the form every other BSD converged on

Each is the right answer for its year, and the last is the right
answer now. The other BSDs settled it: NetBSD 1.0 has no local
declaration of ttrstrt anywhere in its kernel, NetBSD 1.1 and FreeBSD
2.0.5 have one stray each, and their tty.h carries the same line
Berkeley wrote. NetBSD 1.0's and 1.1's pccons.c, the nearest relative
of this tree's file, call timeout(ttrstrt, tp, 1) and declare nothing.

So: when a driver here declares a function the headers already
declare, delete the declaration rather than correcting it. Correcting
it preserves the thing that let the two drift apart.

The five other stale declarations are in drivers for VAX, Tahoe and
the Sony NEWS, none of which is built here. They will want the same
one-line deletion.

## __P, and where not to add it

__P((...)) is how this tree writes a prototype that must also compile
on a K&R compiler, and it is used throughout the headers. It is not
used evenly in the drivers: pmax/dev/scc.c has it, sparc's
rcons_kern.c writes the prototype plainly, and none of the twelve
drivers in i386/isa uses it at all. A correction to an i386 driver
that introduced __P would be importing a style the directory does not
have, which is why the ttrstrt declaration there was deleted rather
than rewritten in scc.c's form.
