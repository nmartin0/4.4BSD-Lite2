# What was deferred, and what the tree says to do instead

Every item here was read out of a commit message or an `AI-ONLY NOTE`
on this branch, not inferred. Where the record names a donor or a
technique, that donor is named here; where it says a thing was not
attempted, that is quoted rather than paraphrased. The commit is given
so the reasoning can be read in full.

The pattern across all 127 commits is worth stating first: **almost
every sacrifice on this branch was made to reach a link, not because
period fidelity required it**, and in most cases the commit already
names the tree that did it properly. This is a list the previous work
wrote for itself.

## 1. Functionality given up

### Kerberos: ten programs authenticate locally that were meant not to

`36c29877`, `54236780`, `6ce0f8c1`, `46e0f065`

4.4BSD-Lite2 turns Kerberos on unconditionally — no conditional
anywhere in Berkeley's tree, and `kerberosIV` shipped to support it.
Eleven Makefiles here are now `.if defined(KERBEROS)`, so `login`,
`su`, `rlogin`, `rsh`, `passwd`, `telnet`, `rlogind`, `rshd`,
`telnetd` and `rcp` do local passwords only. `libexec/kpasswdd` is
Kerberos itself and does not build at all.

`46e0f065` calls this "the sharpest reduction against Berkeley's
intent on this branch", and corrects the earlier claim that the tree
*could not* build kerberosIV: four of the seven DES generators
(`make_p_table`, `make_ip`, `make_fp`, `make_odd`) compile and run as
they stand. What blocks the other three is a byte-order define
`kerberosIV/include/conf.h` asks for by name, one undeclared variable
in `make_p`, and two link failures. "A day's work of the kind this
branch is made of, not a wall."

**Recommendation as recorded:** leave KRB4 conditional. Kerberos IV
was superseded in 1993, its DES has been breakable for decades,
OpenBSD ships none today, FreeBSD 13's login has none, NetBSD 10 uses
Heimdal.

### ~~Programs that still do not build~~ FIXED

**Done.** Every program directory the Makefiles name for the i386
builds: 341 of them, in one pass, with the release's compiler. What
this entry listed went one of two ways.

| program | now | commit |
|---|---|---|
| `usr.bin/vacation` | links against libc's dbm, not the `${LIBDBM}` no tree built | `7ea2cfc4` |
| 13 games | gtty and stty from NetBSD 1.0's libcompat; generators built with `${HOSTCC}`; the compiler's own fixes | `3f74e1ef`, `4adeaafe` and before |
| `libexec/kpasswdd` | named only with `KERBEROS`, as the rest of Kerberos is | `16d88945` |
| `games/ching` | not named: the cut took the program and left its helpers | `16d88945` |
| `games/xneko`, `games/xroach` | named only when X11's headers are in the root, where Lite2 installs contrib/X11R5 | `16d88945` |
| `usr.bin/pascal`, `usr.sbin/eeprom`, `usr.bin/vmstat.sparc` | not named for the i386: VAX and Tahoe, and sparc | `b1e714d4` |

Twenty-two named directories are absent, the Lite cut's (the next
entry). X11R5 is to come with contrib/X11R5.

### Sources and directories named but absent

`dd3de94e`, `3a30baa0`, `02feb631`, `b8596dbf`

Twenty-five programs, thirty-six documents and several sources are
named by build lists and are not here. `b8596dbf` established that
this is a **lookup, not an inference**: the Lite cut was mechanical
and kept `%sccs.include.redist.c%` while dropping
`%sccs.include.proprietary.c%`. Some no BSD has (`learn`, `struct`,
`diction`, `ptx`); some every BSD has as its own later rewrite
(`ed`, `expr`, `at`, `cron`, `sa`, and `bc` and `dc`, rewritten for
OpenBSD in 2003); some OpenBSD and NetBSD restored from Berkeley's
own 4.4 sources under Caldera's licence (`deroff`, `spell`); and
`compress`, which Lite carried and Lite2 left out, most likely for
the LZW patent rather than the licence (back, patch 189).
Corrected 2026-10-16: this paragraph had `bc`, `dc` and `spell` among
those no BSD has. See *Caldera's licence, and what the BSDs took
under it*, below.

`lib/libcompat` lost thirteen of twenty-two sources. `4.1` held five
manual pages and no code at all -- `gtty`, `stty` and `ftime` are
NetBSD 1.0's now (`3f74e1ef`), and `getpw`, `vlimit` and `vtimes`
keep their pages and have no code, as there; `4.3` is missing `ecvt.c`, `gcvt.c`,
`sibuf.c`, `sobuf.c`, `strout.c`. **`ecvt` and `gcvt` are POSIX** —
this is a conformance hole, not just a missing file. `regex.c` was
restored from NetBSD 1.0 (`fd425062`), and `b8596dbf` found that it is
*not* Berkeley's 407-line engine (marked proprietary) but a 93-line
shim over the `regexp` engine already here.

`usr.bin/grep` lost `old.bin.grep`, `old.egrep`, `old.fgrep`,
`old.ucb.grep`; `diff` lost `diffh`; `pascal` lost `eyacc`. All absent
from 4.4BSD-Lite too. What is left of grep, James Woods' egrep, hands
-v, -b, -w, -x and -y to the first three, so they failed ("can't exec
old 'egrep'"); usr.bin/grep is no longer named, and the system's grep,
egrep and fgrep are GNU grep 2.0 as NetBSD 1.1 had it, with 4.4BSD's
-o, from contrib/grep-2.0, as in NetBSD and FreeBSD 2.0 (patch 160).

Since patch 153, which builds the whole tree with `sh -ec` as Lite2's
own make ran commands, each of these is **no longer named** in its
Makefile: a name with no directory stops the build, as it stopped
Lite2's own `make build`. Sixty entries, in bin, lib, sbin, usr.bin,
usr.sbin, share/man and share/doc/{psd,smm,usd}, each Makefile saying
which. They come back by name if their programs do.

**icheck, ncheck and mkproto stay out** (decided 2026-10-23). All
three were Berkeley's, AT&T-derived (`new copyright; att/bsd/shared',
April 1991), in the 4.4 lists, and working with 4.4's FFS. No BSD of
the period shipped any of them: NetBSD's sbin/Makefile kept `# Missing:
icheck ncheck' from 1994 on, FreeBSD's `# XXX MISSING: fsdb icheck
ncheck' and `# XXX MISSING: mkproto' (Grimes, 28 May 1994), and fsck
had taken icheck's place long before, as its own page said. fsdb,
named with them, has a successor every BSD carried, John Kohl's of
1995, and is taken up separately.

**ncheck_ffs is deferred.** OpenBSD alone wrote an ncheck: Thomas
Lockert's ncheck_ffs (SigmaSoft, 1995-96, a four-clause licence with
an advertising clause until 2003), installed with an `ncheck' link.
It is a separate program written from the manual page, not Berkeley's,
which Berkeley never saw, and not fully compatible with it:

- a file no directory reaches is not listed; Berkeley's printed it
  under `???', or `...' for a loop -- the case fsck has just named by
  inode number;
- one file system per run, where Berkeley's took several;
- a seven-place number column where Berkeley's had a tab;
- -i reads numbers until one is not, where Berkeley's took a fixed
  count; and it also takes a mount point and finds the raw device.

-i, -a, -s and -m are otherwise Berkeley's. It builds here with its
includes reordered (dinode.h before fs.h). Meanwhile `find -xdev
-inum` names an inode on a mounted file system, `find -perm` does
-s's audit, and fsdb inspects an inode on an unmounted one.

### Programs the Lite cut emptied

Patch 153. Ten more the cut did not delete but emptied, leaving a
manual page, a README or a few subdirectories, and no Makefile:
`usr.bin/diff` (diff and diff3), `usr.bin/ex`, `usr.bin/file`,
`usr.bin/sort`, `usr.bin/uucp`, `usr.sbin/ac` (back: NetBSD 1.0's,
patch 187), `usr.sbin/quot`,
`usr.sbin/sendmail` (back: sendmail 8.8.8, patch 182), and `old`,
the top-level directory of eight
retired programs. None is named now, and their manual pages are not
installed. Lite2's syslog.conf still sends uucp.info to
/var/spool/uucp/ERRORS, and syslogd says at boot that it is missing;
the line is kept as shipped, and the file comes with uucp, when uucp
is taken up (roadmap.md, item 2; patch 186). contrib/ holds programs that could stand for some --
nvi for ex and vi, file-3.12, sort -- outside contrib/Makefile's
list; they are to be weighed with contrib/ (roadmap.md, *Next:
contrib/*).

**diff is back**: GNU diff 2.6, NetBSD 1.0's, from contrib/diff-2.6,
is the system's diff, diff3 and sdiff in /usr/bin, as in NetBSD 1.0
and FreeBSD 2.0. Patch 155 had Berkeley's own 4.3-Reno diff there, as
OpenBSD took it under Caldera's licence in 2003; it was removed by the
rule below (*Caldera's licence, and what the BSDs took under it*).
usr.bin/diff is again as Lite2 left it. **sort is back**: contrib/sort,
Berkeley's own new sort, Peter McIlroy's of 1993, which Lite2 ships
off contrib/Makefile's list, as /usr/bin/sort (patch 158) -- the sort
NetBSD's usr.bin/sort descends from; bib's invert runs it. And **ed
is back**, though it was deleted rather than
emptied: contrib/ed, Berkeley's own replacement, Rodney Ruddock's, as
/bin/ed.

### Caldera's licence, and what the BSDs took under it

Researched 2026-10-16. **The rule, by the maintainer's decision
(2026-10-17)**, for code this tree adds -- not for what came with
4.4BSD-Lite2, which is kept as Berkeley shipped it, whatever its
notice:

- Caldera-licensed code is taken only where OpenBSD, NetBSD or FreeBSD
  took it, and never what none of them did -- Berkeley's own kernel
  bodies among it (below).
- Even then, an alternative not derived from Caldera-licensed code is
  preferred. So diff is GNU diff 2.6, not Berkeley's (patch 157).
- Code under the old Berkeley notice ("The Berkeley software License
  Agreement specifies the terms and conditions for redistribution")
  is not added either.

None of what follows is needed to bootstrap the system; the list at
the end is to be looped back to under that rule.

**The licence.** Caldera's of 23 January 2002 frees the 16-bit UNIX
V1 to V7 and the 32-bit 32V, and excludes System III, System V and
their successors. It does not name BSD. But what it frees is AT&T's
copyright, and Berkeley's proprietary-marked files are proprietary
only for what they took from 32V: each carries Berkeley's licence for
Berkeley's part and, now, Caldera's for AT&T's. Greg Lehey read it so
(TUHS, 2020: "it was my understanding that this would free all BSD
versions"), and -- the stronger evidence -- all three BSDs acted on
it, in their base systems, keeping both notices:

| BSD | what, from Berkeley's 4.4 sources, under Caldera's licence |
|---|---|
| OpenBSD | deroff (`25a69cdca46`, 2002, millert: "the 4.4BSD deroff, now freed by Caldera"); spell (`9175dedbf31`, 2002); diff and diff3 (2003, deraadt: "free because of the Caldera license"); the documents for ed (09.edtut, 10.edadv), awk, sed, lex, yacc, lint, bc, dc, 01.begin, 17.msmacros and smm/17.password (2002-2003, mickey, otto); bc.library and the bc and dc pages |
| NetBSD | deroff, spell (spell.sh in plain sh, 2005), diff, diff3, dc's document and page, bin/sh's USD document, m4's PSD document; and **games/ching's ching.sh, hexagrams and macros** (2005, perry) -- the three files this tree's games/ching lacks |
| FreeBSD | diff and diff3 |

**What none took.** No BSD took a V7 or 32V source directly, nor
Heirloom's tools, nor Sun's beyond ZFS and DTrace (FreeBSD's cddl/).
OpenBSD imported pcc, 32V's compiler's descendant, in 2007 and removed
it in 2012. For ed, expr, at, cron, sa, ac, units, file and sort, every
BSD kept its own rewrite rather than the archive's. So the GNU tools
here -- flex as lex, gawk, GNU diff, gzip, and groff to come
-- have no Caldera or Heirloom replacement with precedent, and the
rewrites accepted (expr, libcompat's regex.c, Kernighan's awk) stand.

**The kernel.** No BSD took Caldera-licensed kernel code: no file
under sys/ in the three current trees carries the licence, and no
commit in their histories applies it there. Berkeley's history holds
complete versions of all eight files whose 35 bodies the settlement
removed (kern_exec.c 8.10, kern_acct.c 8.8, vfs_bio.c 8.11 and the
rest), 32V's lineage throughout; outside the three, 2.11BSD and
DiscoBSD ship that lineage's kernel, and McKusick credited the licence
with making the whole CSRG archive publishable. By the rule above
they are not used: the bodies stay as imported, from NetBSD and
FreeBSD, and precedent.md's rule for the CSRG history stands.

**AT&T's AST toolkit** (github.com/att/ast; Korn and Fowler, AT&T
Research, 1985-2012; Eclipse Public License 1.0; not derived from
Caldera-licensed code) was weighed the same day: no BSD has it in its
base system -- FreeBSD took ksh93 off its release CDs in 2001 "until
the license issues are resolved", and it is in ports and pkgsrc only
-- and it needs its own runtime, libast, sfio and vmalloc, and its own
nmake. A candidate for ports, not the base system.

**To loop back to**, under the rule, each with its precedent and its
catch:

	deroff     OpenBSD's 2002 import; NetBSD -- Caldera's, taken
	           by two BSDs; no non-Caldera alternative in any
	spell      NetBSD's 2005, whose front end is sh; OpenBSD's is
	           ksh, which Lite2 has not. Uses share/dict/words.
	           As deroff
	ching      NetBSD's ching.sh, hexagrams and macros, completing
	           games/ching. As deroff
	compress   Berkeley's own, `redist`, not Caldera's; the patent
	           has expired; every BSD ships it. Back: Lite's own,
	           with Berkeley's 1995 fix (patch 189)
	bc, dc     Otto Moerbeek's 2003 rewrites, ISC-licensed, all
	           three BSDs -- preferred, not being Caldera's; dc
	           needs libcrypto's bn(3), which this tree has not.
	           Alternative: GNU bc, as FreeBSD 2.0 and NetBSD 1.0
	documents  the Caldera-licensed 4.4 documents above, into
	           share/doc, those OpenBSD, NetBSD or FreeBSD took

### The one inversion, stated by the project itself

`7cb61884`

> 741 lines were imported so savecore could compress a crash dump,
> while ten sites of adaptation that would give the kernel a disk
> driver were declined as too large. By functional importance that is
> backwards, and the reason given at the time — reaching a link sooner
> — was about momentum rather than authenticity.

### binutils reads no core files

**Superseded, and to be measured.** The stand-in this recorded --
libbfd built without a core-file reader, because NetBSD's name for
the system gave NetBSD's reader and GNU had no name for a 4.4BSD on
ELF -- went with contrib/binutils, NetBSD 5's 2.16.1. The first way
of doing it properly that this entry named is what replaced it: the
system has a name of its own, `i386-unknown-bsd4.4elf`, in
`config.bfd` and the configure scripts of contrib/binutils-2.11.2,
and for a host of that name bfd's configure keeps choosing what it
chooses for any i386 `*-bsd*` host, `trad-core.c` with
`hosts/i386bsd.h`, written for 386BSD and BSD/386, whose `struct
user` is 4.4BSD's.

The build host's cross binutils is not such a host and has no reader,
as no cross binutils does. Whether trad-core reads this kernel's core
files is to be measured when binutils is built on the system itself.

### `chrtoblktbl`'s bound is wrong in hp300 and luna68k

Found while writing i386's. `chrtoblk` indexes
`chrtoblktbl[major(dev)]` and refuses anything at or above `MAXDEV`.
Measured across the four ports that have the table:

	news3400   MAXDEV 43   cdevsw 43   ok
	pmax       MAXDEV 19   cdevsw 19   ok
	hp300      MAXDEV 21   cdevsw 23   two majors unreachable
	luna68k    MAXDEV 21   cdevsw 23   two majors unreachable

On hp300 and luna68k, character majors 21 and 22 return `NODEV`
whether or not they have a block device. The number looks like one
chosen when the table was written and left behind as devices were
added; luna68k derives from hp300 and inherited it.

Not fixed: neither port has ever been compiled here (`roadmap.md`
Tier 0), and the two that agree establish the invariant as
`MAXDEV == nchrdev`, which is what i386's new table uses.

## 2. Performance and correctness sacrificed

### ~~`DELAY()` rounds anything under a millisecond to nothing~~ FIXED

`64dc319e`, note in `i386/include/param.h`

**The description below was wrong and is corrected here.** The note
said sub-millisecond requests round to nothing; the multiplication
precedes the division, so small delays were the ones that worked.
Measured on a QEMU harness running this port's own calibration,
`delaycount` comes out 271473 and `delaycount * n` overflows a signed
int above about 7900 microseconds, where the count goes negative and
the loop exits at once. Six of the eight call sites asked for more
than that.

Worse, nothing called `startrtclock()`, so `delaycount` was never
written at all and every `DELAY` was a no-op for every argument. Both
are fixed: `i386: call startrtclock, so the 8254 is programmed at all`
and `i386: delay() reads the 8254, where DELAY counted a loop`.

What the original note said, for the record: the busy loop uses
`delaycount`, which `findcpuspeed()` calibrates to a millisecond, so
microseconds are divided by a thousand.
`isa/pccons.c` asks for `DELAY(4000000)`, `isa/wt.c` for
`DELAY(1000)`. The commit names the answer outright:

> every descendant wrote a real `delay()` reading the 8254, computing
> `n * TIMER_FREQ / 1e6` with a 64-bit intermediate

About thirty lines, adapted rather than copied. Four trees agree.
An earlier draft scaled by `cpuspeed` as hp300 does; this port has no
`cpuspeed`, only `isa/if_ec.c` declaring one nothing defines.

### `CLKF_INTR` is hardcoded zero

`64dc319e`, note in `i386/include/cpu.h`

The i386 `clockframe` is the generic `intrframe` and carries no
interrupt-level flag. pmax and news3400 also answer zero; **hp300
wrote the real test against `PSL_M`, put it behind `#if 0`, and left
the zero live with a note**; only luna68k and sparc answer it
properly. Profiling cannot distinguish interrupt-level ticks.

### No statistics clock

`64dc319e`

`stathz` stays zero, so `kern_clock.c` does the statistics from
`hardclock`, and `setstatclockrate` is empty in consequence (as
NetBSD 1.0's i386 one is). **FreeBSD 2.0.5 and OpenBSD 1996 both have
a real statistics clock on IRQ 8** — "work this does not attempt".

### Interrupts are wired in at build time

`3201007b`, `64dc319e`, `5a0ade80`

This is the structural one. `usr.sbin/config/mkglue.c` generates
`vector.s`; the handler is wired in at config time, so there is no
`intr_establish`, `register_intr` or `isa_intr_establish`. That is
why none of the three donors' `cpu_initclocks` could be taken.

**Every other tree had already moved away from it before ELF was a
factor.** NetBSD 1.1 has a static `vector.s` dispatching through a
table filled at run time; OpenBSD 1996 has no `mkglue.c` at all;
FreeBSD 2.0.5 has a 298-line static file. FreeBSD 2.0.5's `isa.c`
carries `register_intr()`/`unregister_intr()` in **1056 lines against
this tree's 254**, and every driver's attach path changes with it.

`5a0ade80` adds the fact that reframes it: **`mkglue.c` was never
Berkeley's own work** — it arrived from 386BSD in January 1991 as
"386BSD additions to config". That is part of why every later tree
dropped it rather than maintaining it. Nothing genetic is lost by
replacing it.

**This is the single biggest blocker to modern hardware.** Nothing
probed, hot-pluggable or PCI can work under a build-time vector table.

### ~~`cpu_model` is empty; nothing detects the processor~~ FIXED

**Done.** `i386: detect the processor, and enable write protection on
a 486' enabled the detection sequence that had sat under `#ifdef
cgd_notdef` since the June 1993 merge, and added `identifycpu()`.
A running kernel now prints `CPU: Pentium (586-class CPU)`.


`c12b5995`

`hw.model` reads empty. "This port detects nothing: `initcpu()` is an
empty function and there is no `cpu_class`, `cputype` or
`identifycpu` anywhere in i386." hp300 fills its buffer at boot; all
three donors fill theirs from an `identifycpu()` that reads the
processor. "An empty `hw.model` is better than an invented one."

### `vmstat` shows `??0`, `??1` for drives

`6698fb08`, note in `usr.bin/vmstat/names.c`

NetBSD 1.0's empty i386 stub was taken. **FreeBSD 2.0.5 has a real
implementation by Rodney W. Grimes** reading
`namelist[X_DK_NAMES]` from the kernel to give drives their true
names. Declined only because the kernel symbol could not be verified
without a running kernel — a condition that lifts the moment one
boots.

### ~~The kernel has no local disk~~ FIXED

**Done**, all three: `wd` is FreeBSD 2.1.0's (imports.md, *The
disks*), and `fd` and `wt` are Berkeley's, in `GENERIC.i386`. The
floppy reads and writes after three corrections from FreeBSD 2.0,
whose fd.c and isa.c are these files' descendants (`4928f76c`); `wt` drops a `delay()` of its own that the
kernel's now duplicates (`8b16f38c`), and probes nothing on QEMU,
which has no QIC-02 tape. What follows is the record as it stood.

`adb8a806`, `4326a65b`

`wd`, `fd` and `wt` are out of `LINK.i386`. `wd.c` and `fd.c` use the
4.3BSD queue names `av_forw`, `b_forw`, `b_actl` against a `struct
buf` that has had `b_actf`/`b_actb` since before 4.4BSD-Lite — nine
sites, seven in `wd.c` and two in `fd.c`. `wt.c` is out for a
different reason: a static declaration of `cmds` following a
non-static one.

**That description is right about the symptom and wrong about the
remedy.** Traced across six trees, the whole picture is:

	386BSD 0.1   b_actf and b_actl are MACROS over av_forw and
	             av_back, and b_forw/b_back exist as the hash
	             chain. wd.c keeps both queue levels in struct buf
	             and every field it needs is there.
	4.4BSD       struct buf becomes b_actf, **b_actb as real
	             fields; av_forw, av_back, b_forw and b_back are
	             deleted. wd.c arrives from 386BSD unchanged --
	             seven old-name uses, already broken.
	Lite1        identical, seven.
	Lite2        identical, seven.

So nothing regressed between the Lite releases and there is no
earlier permissively licensed version that does it differently: the
driver was broken on arrival in the encumbered release, when the
structure changed underneath it. It is also not an i386 oddity --
`vax/bi/kdb.c`, `vax/uba/uda.c`, `vax/uba/np.c`, `vax/uba/tmscp.c`,
`vax/vax/mscp.c`, `tahoe/vba/ik.c` and `tahoe/vba/cy.c` have the same
break, and all of them are out of their configurations too.

**The fix is not a rename.** Seven of the nine sites are, but
`wd.c:313-318` keeps a second queue -- the controller's list of
*drives*, chained through `b_forw`, while `dp->b_actf` is already the
head of that drive's *buffer* queue. One field cannot be both.

**And the answer is in this tree, not a donor.** Berkeley's own
working disk drivers already separate the two: hp300's `sd.c` uses
`struct buf`'s `b_actf` for the per-drive buffer queue via
`disksort()`, and keeps the controller's queue of devices in its own
`struct devqueue sc_dq` (`hp300/dev/scsivar.h:41`), chained by
`dq_forw`/`dq_back` inside the softc. Two levels, two structures,
neither borrowing the other's fields.

That is exactly what NetBSD 1.0 later did with a `TAILQ` of
`wd_softc`. FreeBSD 2.0 and 2.0.5 instead flatten to one level,
chaining buffers directly in `wdtab[ctrlr]`, which discards the
controller-level queue `wd.c` was written around and hp300 still
uses.

So: **per-drive buffer queue stays in `struct buf` through
`disksort()` and `b_actf`; the controller's drive list moves into
`wd_softc`**, as hp300 does, using the `sys/queue.h` macros `struct
buf` itself already uses for `b_hash` and `b_freelist`. NetBSD is
confirmation; hp300 is the source.

**Superseded.** Lite2's `wd.c` has since been replaced by FreeBSD
2.1.0's, for its ATAPI CD driver (imports.md, *The disks*). That one
queues FreeBSD's flattened way described above, moving each drive's
buffers onto the controller's queue in `wdtab` through `b_actf`; the
queue question above no longer arises.

### ~~`GENERIC.i386` is left broken on purpose~~ FIXED

**Done** (`f2975f92`): `GENERIC.i386` is corrected, each line from a
configuration Berkeley, FreeBSD or NetBSD wrote, and builds and boots;
`LINK.i386` is gone. It has `PROCFS` since ptrace (`a1b09daf`). The
record as it stood:

`983934d4`, `131c579b`, `adb8a806`

It names a `cn0` console no driver provides — the CSRG history shows
the 1992 reorganisation moved the driver to `pccons.c` as
`optional pc` and **GENERIC.i386 was never followed through**;
Berkeley's own ARGO and BLITZ configs have the corrected
`device pc0 ... vector pcrint`. It also selects no filesystem and no
pager. `LINK.i386` carries the corrections; `GENERIC.i386` is left
"exactly as Berkeley shipped it, with the same fault it has had since
1992". **NFS, DEBUG and DIAGNOSTIC were left out of `LINK.i386`**
deliberately.

### ~~The DIAGNOSTIC build does not compile~~ FIXED

**Done** (`99f245e0`): `struct ctldebug` and `syncprt` under the
option that uses them, as FreeBSD 2.0 has them; a DIAGNOSTIC kernel
boots multi-user. The record as it stood:

`6a81e5fe`

"Built with `-DDIAGNOSTIC` the file has the same two errors it had
before this change… `debug1` has an incomplete type at what is now
line 974."

## 3. Interfaces frozen at the old spelling

| what | current state | the named answer | commit |
|---|---|---|---|
| `sys/mman.h` | `mmap`/`mprotect`/`munmap`/`msync`/`mlock`/`munlock` take `caddr_t`; Berkeley's own comment reads "Some of these int's should probably be size_t's" | **OpenBSD changed them to POSIX `void *` in November 1997, `mmap` returning `void *` from January 1998.** "belongs with POSIX work" | `1e3ffe96` |
| `sys/vnode.h` | `vref`/`vhold`/`holdrele` as `static __inline`, NetBSD 1.4/1.6 spelling | NetBSD 1.4's `ilstatic` macro not taken | `a05ade5b`, `6a81e5fe` |
| `sys_nerr` | `const` dropped from the definition, header returned to Berkeley's line | ten files here redeclare it non-const; OpenBSD still defines `int _sys_nerr` | `17271a25` |
| `reset_cpu` | this port's name | **every other BSD and 386BSD 0.1 call it `cpu_reset`**; touches `pccons.c`, `machdep.c`, and `i386/stand`'s `kbd.c` and `srt0.c` | `338c7c8c` |
| `bin/sh` arithmetic | lex-based, needs `libl` | NetBSD 10 and FreeBSD 13 replaced `arith_lex.l` with hand-written C | `4fc8837e` |
| `fpr`'s `gettext` | declared to avoid the builtin | **NetBSD 10 renames it `get_text`** — "the surer fix and a larger one" | `86455ce0` |

## 4. Build-system compromises with a named exit

### ~~The specs file stands in for a cross-compiler~~ FIXED

`9e6bb751`

> No BSD uses a specs file; they build a compiler for their target,
> and then `-B` suffices. This is a stand-in for a cross-compiler and
> goes away when there is one.

There is one: GCC 2.95.3 and binutils 2.11.2, built by
build/sysroot.sh for i386-unknown-bsd4.4elf, which find the target's
headers, startup files and libraries in the root by themselves,
through their sys-include and lib. The specs file, and the host
GCC's flags around it, are gone from build/make.sh.

### ~~`YACC` still names the packaged byacc~~ FIXED

**Done** (`acb899a0`): `YACC` is the tree's own yacc and `lex` the
tree's flex 2.5.2, both built into the tools directory; every grammar
and scanner outside contrib/ was regenerated and its program built,
and `ftpcmd.y`'s `yylex` is static again. The record as it stood:

`fbd2592b`, `74b0fa86`, `dc51dc50`

`libexec/ftpd/ftpcmd.y` lost `static` on `yylex` **because byacc
declares the function and this tree's own yacc does not**. `dc51dc50`
then built `usr.bin/yacc` as a host tool — but `YACC` still points at
the packaged byacc. Pointing it at the tree's own would let
`ftpcmd.y` stand as Berkeley wrote it. "The tree would generate itself
with its own tools", which `74b0fa86` calls "the end this is working
towards".

Not a one-line switch, and `74b0fa86` says why: it "wants every yacc
file here rebuilt and checked against the result". Fifteen `.y` files
outside `contrib/`, each regenerated under a different yacc and
verified. `fbd2592b` settles the only question that would otherwise
block it — `ftpcmd.y`'s non-static `yylex` "is right under either"
yacc — so nothing has to be reverted first.

### The kernel relies on common symbols -- to be fixed with the GCC upgrade

`build/kernel.sh`; roadmap.md, Tier 1, item 5

101 globals are tentatively defined in more than one kernel object,
from headers that say `int nbuf;` without `extern`. GCC 2.95.3
merges them, as every compiler of the period did. GCC 10 and later
default to `-fno-common` and refuse the link. **Before any newer GCC
builds the kernel, each header gets an `extern` and each symbol one
definition**, as the BSDs did when GCC 10 changed the default.
Meanwhile `build/kernel.sh` stops a kernel in which a common symbol
became code, the `mmap` fault's class.

### The kernel is linked without a linker script

`a50b8e5a`

> what the later releases do instead is a linker script, which is what
> `-T` means now and what every modern BSD kernel is linked with. That
> would place the sections explicitly rather than leaning on ld's
> defaults… It is not written here: no BSD of this period has one to
> copy, and one written from nothing would be this project's own
> invention.

Related, from `docs/status.md`: the link passes no `-e`, and works
only because `locore.o` is first and `start` is its first symbol.

### The whole tree by `make build` -- what it took

Patch 153; roadmap.md, *The whole tree on the CD*. `build/release.sh`
runs Berkeley's own top-level `make build` into the target root. What
a build on a machine of another kind needs that Lite2's did not, and
what was settled with the maintainer for each:

- **Errors stop the build.** bmake runs a command with `sh -c`;
  Lite2's make ran `sh -ec`, so a directory failing inside
  bsd.subdir.mk's loop stopped it. build/make.sh gives bmake
  Lite2's shell, through a sys.mk of one `.SHELL` line ahead of the
  tree's own.
- **Object directories** where Lite2's `make obj` made them -- its
  obj links are the record -- and none for a document (bsd.doc.mk's
  obj target does nothing) or below contrib/ (contrib/Makefile's
  does nothing). A directory that only recurses gets one as its
  subdirectories' parent, so share/doc/{psd,smm,usd} and share/man
  install their own files from `${.CURDIR}`, as games/fortune/datfiles
  does.
- **Tools the build runs** -- zic, cap_mkdb, strfile, col -- are the
  tree's own built for the build machine by build/sysroot.sh and
  called by name: FreeBSD 2.2's build-tools, strfile and the rot13 as
  FreeBSD 4.0's Makefile has them. cap_mkdb's db(3) is built by
  Lite2's own porting kit, lib/libc/db/PORT/linux, with hash.h's
  `errno` field renamed `err` (NetBSD 1.3, OpenBSD 2004);
  build/getcap-host.c gives getcap.c 4.4BSD's fgetln and a harmless
  second fclose. Generators the build runs -- boggle's mkdict and
  mkindex, phantasia's setup -- are built with `${HOSTCC}`, as
  adventure's are; phantasia's setup writes its files where it runs
  and they are installed under `${DESTDIR}` (FreeBSD 2.0; Berkeley's
  wrote the build machine's /var/games/phantasia). The whatis
  database is made from the pages under `${DESTDIR}`, not the build
  machine's.
- **Data the cut took**: dict's words is web2, connectives gone, and
  tmac.r, dump.5, plot.5, eqnchar.7, man.7, ms.7 and term.7 dropped
  from their lists, as FreeBSD 2.0, NetBSD 1.0 and OpenBSD all have
  it. The mtree spec, of July 1992, gains the four document
  directories the Makefiles install into: usd/11.vitut, 12.exref,
  13.viref and smm/19.perl.
- **The hidden games' links are not chowned** (NetBSD 1.0, cgd,
  1994): on 4.4BSD that chown followed the link and stripped dm's
  set-user-ID bit.
- **chpass is made immutable by the installer**, since a file's flags
  do not travel in the set; its Makefile does it only when installing
  onto the running system.
- **a.out tools.** Lite2's ld, ar, nm, ranlib, size, strip, strings,
  gprof and gcore read or write a.out only; for the i386, whose
  programs are ELF, they are not built, and binutils 2.11.2 is built
  to run on the system in their place -- as, ld, ar, ranlib, nm, size,
  strip, strings, objcopy, objdump, readelf, addr2line, c++filt,
  gprof, NetBSD 1.6's set -- by contrib/binutils-2.11.2's makefile,
  in the manner of Lite2's own gdb and gcc makefiles. NetBSD 1.4
  replaced the same programs with binutils'; OpenBSD built its own
  only where there was no ELF toolchain. gcore is not replaced:
  NetBSD and OpenBSD dropped it. GNU's install targets are not used:
  they copy as, ld, nm and strip into `${prefix}` without
  `${DESTDIR}`, onto the build machine -- **measured, on this build
  machine, and undone**.
- **contrib/** for the i386 names binutils-2.11.2 and, one at a
  time, Lite2's own list as each program is taken up (roadmap.md,
  *Next: contrib/*).

### contrib/ in object directories

Patch 154. Lite2 builds contrib/'s programs in their own source
directories: Bostic made contrib/Makefile's obj target do nothing on 9
June 1993 ("add an obj target"), for programs whose GNU Makefiles knew
no object directory. Every descendant built them in object
directories: FreeBSD 2.0, NetBSD 1.0 and OpenBSD 1995 by rewriting the
ones they kept as BSD Makefiles under gnu/, OpenBSD from 1996 by
running GNU's own configure and Makefiles in ${.OBJDIR} through a
Makefile.bsd-wrapper, FreeBSD 3 and NetBSD 1.3 by reachover Makefiles.
Decided (2026-10-16): OpenBSD's way. build/make.sh makes object
directories below contrib/ as everywhere else; a program with a BSD
Makefile builds there unchanged, and one with GNU's Makefile has a
`makefile` beside it -- the name Lite2's own gcc-2.3.3, gdb-4.7.LBL,
kermit and perl wrappers have -- that runs GNU's Makefile there with
srcdir pointed back, and installs by name under DESTDIR, which those
Makefiles do not know.

### Other build items

- **No `LIBCRT0` dependency** — changing `crt0.o` does not relink
  programs; NetBSD has it (`ad7131e0`).
- **468 dangling `obj` symlinks** shipped in the tarball; `make obj`
  is not used, object directories are made outside instead
  (`1da402c0`), where those links say Lite2's were (patch 153).
- **`-I-` is obsolete** in GCC and warns once per compile; used for
  NetBSD's include order in the csu Makefile (`c5fd6177`).
- **`tsort` reports loops** among libc's profiling objects. OpenBSD
  silences them with `tsort -q`; **GNU tsort rejects `-q` and prints
  nothing at all**, which would install an empty `libc.a` silently.
  Measured both ways; the notices stay (`1da1369b`).

## 5. Known defects carried knowingly

- ~~**`make build` deletes three files the tree ships**~~ **FIXED**
  (patch 163): contrib/nvi.1.43/USD.doc/vi.ref's paper.ps and
  vi.ref.txt, Berkeley's, made from its sources and shipped made, and
  paper.pdf, the 2014 import's. Lite2's own top-level build reaches
  the directory through share/doc/usd/13.viref, bsd.doc.mk's cleandir
  removes `paper.*`, and the Makefile's CLEANFILES vi.ref.txt; with
  the source sets on the CD (patch 162), a second release from one
  checkout shipped the sources without them. All three BSDs took the
  made copies out and left the clean rules alone, decided 2026-10-17:
  NetBSD in 1994 and again in 1996 ("we do not want generated
  files"), FreeBSD by 2.0.5, and again leaving out the "postscript
  files" when it imported nvi 1.79 in 1996, OpenBSD in July 1996,
  before 2.0. Nothing is lost: bsd.doc.mk installs the roff sources,
  not the made copies, and groff makes them from those sources.
  contrib/nvi.1.43/docs/README, Lite2's, still names the two.

- ~~**A process starts with 6 MB for its data, Lite2's default.**~~
  **FIXED** (patch 165). sys/i386/include/vmparam.h had DFLDSIZ 6 MB
  and MAXDSIZ 32 MB, and GCC 2.95.3's cc1plus needs more than 6 MB
  for the STL's strings: g++ said "virtual memory exhausted" until
  `limit datasize 32m'. Every descendant raised both: NetBSD 1.0 and
  1.2 to 16 MB and 256 MB, FreeBSD 2.0 and 2.1 to 64 MB and 128 MB.
  Decided 2026-10-18: 64 MB to begin with, FreeBSD 2.0's, and its 128
  MB at most. The text and stack limits stay Lite2's.

- **sort applies a leading -n, -r or the like to no key given by
  position.** `sort -n +1` and `sort -n -k2` compare the second field
  as text; `sort +1n` and `sort -k2n` compare it as a number, as
  POSIX has the first form do too. Berkeley's 1993 sort as Lite2
  ships it, the same built on the build machine; kept as it is
  (patch 158).
- **bib's shipped bibinc.fullnames and bibinc.shortnames are not what
  its bibinc makes from the bibinc.names beside them**: 260 lines
  against 862 on the system itself. Berkeley's, as shipped, are what
  is installed; macs/Makefile would remake them only when
  bibinc.names were newer (patch 157).

- **NetBSD 1.6's `__ctors` reads past its one-element
  `__CTOR_LIST__[1]`**, which GCC compiles into an infinite loop on
  the path taken when the list has entries. Safe only because GNU ld
  puts constructors in `.init_array`, never `.ctors`. "Known and left
  as found" (`c5fd6177`).
- **`common_elf/crtend.c` carries no copyright notice at all.** 26
  lines, first committed by Christopher G. Demetriou in 1996. NetBSD
  shipped it without one until NetBSD 6; OpenBSD ships its copies
  without one today. Imported unchanged by the maintainer's decision
  (`dac5a892`).
- **`PANIC` and `PRINTF` in `locore.s`** (lines 1573, 1575) still emit
  `_waittime` and `_printf` rather than going through `_C_LABEL`. Both
  are invoked only from commented-out lines.
- **`i386/stand/boot.c:122`** reads `struct exec` and accepts only
  `a_magic` 0407/0410/0413. The ELF kernel is loaded by FreeBSD 4.1's
  boot1 and boot2 instead (`sys/boot/i386`); i386/stand is Berkeley's
  and stays as it was.
- **The other ports cannot link ptrace yet**, and are left as
  Berkeley shipped them while they are deferred. With `ptrace`
  (`a56c2f9b`), `procfs_mem.c`, `procfs_regs.c` and `procfs_fpregs.c`
  are standard, so every port needs the procfs register hooks and
  FreeBSD's `ptrace_set_pc` and `ptrace_write_u`. When each is taken
  up: hp300 and pmax have the hooks, but hp300's read `md_regs`, an
  address in `kstack`, which for a process not running is the reader's
  own frame -- NetBSD 1.0's m68k code finds it at the same offset in
  the target's `p_addr` -- and hp300's `procfs_sstep` sets `PSL_T`
  whether asked to set it or clear it, and its `procfs_write_regs`
  checks no status bits; luna68k and news3400 have no hooks, and share
  hp300's and pmax's frames; sparc's `files.sparc` names
  `sparc/sparc/sys_process.c`, which the Lite cut removed.
- **The FPU registers are not given out.** `PT_GETFPREGS` and
  /proc's fpregs answer `EOPNOTSUPP`. Lite2's `npx.c` does not save
  the FPU at context switch -- `locore.s` calls an `npxsave` under an
  option nothing defines, and there is no such function -- so a
  stopped process's FPU state can still be in the chip. When `npx`
  can save on demand (FreeBSD 2.0's `npx.c` has `npxsave`), the hooks
  are FreeBSD 2.1.0's `procfs_read_fpregs` and `procfs_write_fpregs`,
  with NetBSD 1.1's and OpenBSD's flush of a live FPU first -- done
  with the save area's size, where theirs copies `sizeof` a pointer.
  `struct fpreg` is already FreeBSD 2.1.0's.
- **Changes made for GCC 14 are kept** for the newer compilers to
  come, though GCC 2.95.3 does not need all of them. Each says so in
  its `AI-ONLY NOTE`; `grep -rl 'GCC 14' usr/src` lists the files. fpr's
  `gettext` is one: Berkeley's original compiles under 2.95.3.
- **`fdopen` prints a line every time the floppy is opened**, a
  debugging printf Berkeley left in `i386/isa/fd.c`.
- **The serial ports have no dial-out devices.** MAKEDEV makes tty00
  and up, as NetBSD 1.0 does; OpenBSD's later cua00, at minor 128 and
  up, needs callout support Lite2's com.c has not got.
- ~~**Lite2's `mkdir -p` fails on a directory that exists**~~
  **FIXED** (patch 169), by FreeBSD 2.2.5's build(), which POSIX 1003.2
  asks for and NetBSD 1.1 and OpenBSD also fixed.
- ~~**share/termcap runs `ex`, which this system has not got**~~
  **FIXED** (patch 169): nvi is also vi, ex and view in /usr/bin, as
  FreeBSD 2.0 linked it, and root's .profile gives TERM a value in
  single user, as FreeBSD 2.2's does.
- ~~**The CD's termcap is ordered by the build machine's ex.**~~
  **CLOSED, checked** (2026-10-22). A cross build runs share/termcap's
  `ex - termcap.src < reorder' with the build machine's ex, on Linux
  vim's, where the system's own build uses nvi. The two make the same
  file: Berkeley's reorder run by nvi on the system gave 172,285 bytes
  with the CD's termcap's checksum. FreeBSD's buildworld did the same,
  running the build machine's ex for this step -- its bootstrap tools
  were strfile, yacc and config, never ex -- and ex is a POSIX
  utility, assumed as sed and awk are, so build/sysroot.sh does not
  check for it. Nothing else in the cross build runs ex: the Pascal
  compilers' grammar step is VAX and Tahoe only, and the kernel's is
  for profiling kernels. NetBSD dropped the reorder in 1995 (jtc)
  because its termcap had become terminfo's, already in order; Lite2's
  termcap.src is not.
- ~~**Lite2's sh exits under -e when the first command of an && list
  fails**~~ **FIXED** (patch 171). `set -e; false && true' ended the
  shell, where POSIX has -e ignore it, and make runs every command
  with sh -e. eval.c now has the three lines FreeBSD's 1.6 (6 October
  1996) and NetBSD's 1.31 (16 October 1996, PR 2808) both added,
  comment and all, shipped in FreeBSD 2.2 and NetBSD 1.2.1 -- not
  1.2, as this note said; OpenBSD took NetBSD's four days later and
  dropped this shell for pdksh in 1997. The i386 kernel's Makefile,
  the one it stopped, keeps Berkeley's hp300 form. Patch 178 took
  the three lines out again: they covered a list run alone, but a
  failed list after a `;', as in `set -e; false && true; exit 7', still
  ended the shell at the `;' node, and patch 178's narrower test covers
  both, as FreeBSD 5.3 and NetBSD 4.0 dropped them.
- ~~**Lite2's sh exits under -e on `! true'.**~~ **FIXED** (patch 178).
  The negated command was run as tested, but evaltree's check at `out'
  then tested every node, the `!' among them. FreeBSD's eval.c 1.44
  (stefanf, 10 September 2005; FreeBSD 7.0) tests -e only after a
  simple command, a pipeline or a subshell, through a do_etest flag;
  NetBSD took the same lines for 4.0. Earlier fixes were narrower:
  NetBSD 1.5 (Husemann, January 2000) moved the test into evalcommand,
  losing subshells and pipelines; FreeBSD 5.3 (cracauer, December 2003
  and January 2004) tested commands and subshells only, losing
  pipelines. Found testing patch 171. And, fixed in patch 180, the
  two cases kept out of patch 178 as a separate bug: `if false; false;
  then' and a failing command early in a function called as `f ||
  true' ended the shell. FreeBSD's eval.c 1.45 and 1.46 (stefanf, the
  same day, PRs 77067 and 85267) pass EV_TESTED on to an NSEMI's left
  side and to loop bodies, and its 1.14 (cracauer, 4 May 1998, PR 6047;
  FreeBSD 3.0) into a function called where its status is tested,
  which the function case needs as well.
- ~~**Lite2's sh rejects a command of redirections only before &&
  or ||**~~ **FIXED** (patch 172). `<file || echo missing' was a
  syntax error; rcs's rcstest uses the form. NetBSD's parser.c 1.30's
  two case labels, FreeBSD's since 1995. A command of redirections only
  whose file cannot be opened still ends a shell that is not
  interactive -- `sh -c "</nonexistent || echo no"' prints `cannot
  open' and exits 2 -- where POSIX has only a special built-in's
  redirection error do that. Found testing patch 172. Compared
  (2026-10-22), and left as it is for 4.5BSD, for 4.6BSD's POSIX work
  (roadmap-4.6.md): the cause is wider, a redirection error on any
  builtin ending the shell, since evalcommand redirects before it sets
  its handler -- `echo x >/no/dir/f || echo failed' ends a script too.
  FreeBSD 2.0 to 6.x and NetBSD 1.0 to 1.6 are the same. NetBSD 2.0
  fixed the builtins (David Laight's changes, committed by christos 24
  November 2002) and FreeBSD 7.0 (stefanf, 2 and 9 April 2006, with
  special built-ins marked -s in builtins.def); both kept a command of
  redirections only fatal, as a special built-in, until FreeBSD 9.0
  (jilles, 13 March 2010) and NetBSD after 8. Nothing in the tree is
  stopped by it: rcstest's own file always exists, and make runs
  commands with -e, so a failure stops it either way.
- ~~**The clock counts leap seconds**~~ **FIXED** (patch 172).
  Berkeley's zoneinfo REDO=right_only put Lite2's 17 leap seconds into
  every zone, so date -u differed from POSIX time by 17 seconds and
  rcstest failed on it; posix_only, NetBSD's since 1993, decided
  2026-10-19. A system installed before 172 keeps the old zones until
  share/zoneinfo is installed again.
- **rcsmerge shows the common ancestor in an overlap.** RCS 5.6 runs
  `diff3 -am', and GNU diff 2.6's diff3 takes -m as -A, so an overlap
  carries a `|||||||' section and a temporary file's name as its last
  label, where rcstest expects neither; the merge is otherwise right.
  NetBSD 1.0 and FreeBSD 2.0 shipped the same, with this merger.c;
  NetBSD changed it by moving to RCS 5.7. Every other check in
  rcstest passes on the system.
- **su passes nothing after the user name to the shell.** `su tester
  -c id' starts an interactive shell for tester, as Berkeley's su did:
  its usage is `su [-flm] [login]'. NetBSD 1.0's su passes the rest
  on (`np = *argv ? argv : argv-1'); FreeBSD 2.0's is Berkeley's.
  Found testing patch 181; not yet decided.
- **Lite2's /etc/exports is an example.** It exports /usr to
  `local_work_group' and `others', and mountd logs at every boot that
  guest.far.away.domain does not resolve and that `others' is no
  netgroup. Found testing patch 185; not yet compared with the BSDs'.
- **A second `make install` needs single user.** init, login, su,
  passwd, rlogin, rsh, rdist, sliplogin, mail.local and chpass are
  installed immutable, which securelevel 1 will not undo; Berkeley's
  setup guide runs at 0 only while single user. The installer marks
  chpass so on a new system, so even the first install from the
  system's own sources is done single user.

## 6. Things established that change how the rest reads


### 4.4BSD shipped a new driver framework and converted one port

`sys/device.h` declares `struct cfdriver` and `kern/subr_autoconf.c`
implements `config_search`, `config_found` and `config_attach` -- 488
lines of machine-independent autoconfiguration. **Exactly one port
uses it:** sparc, in thirteen files. The other six are each on
something older and local:

	sys/device.h + subr_autoconf.c   sparc
	struct driver                    hp300, luna68k, news3400, pmax
	struct uba_driver                vax
	struct vba_driver                tahoe
	struct isa_driver                i386 -- 386BSD's, outside all of it

So Berkeley wrote a new framework, moved one port to it, and stopped.
The same shape as `setconf`, the i386 pmap and `crt0.c`: begun, not
finished.

**The contemporaries split on it, and the split still holds today.**

	NetBSD 1.0     ships it at 359 lines, and their i386 wd.c uses
	               cfdriver -- they adopted it and converted the ports
	FreeBSD 2.0.5  ships it at 342 lines and their i386 wd.c uses it
	               zero times -- they kept isa_driver, as this tree does

	NetBSD today   sys/device.h, struct cfdriver -- still there
	OpenBSD today  sys/device.h, struct cfdriver -- still there
	FreeBSD today  no sys/device.h; sys/bus.h with device_t and
	               driver_t, the "newbus" replacement

Two of the three surviving BSDs still run on 4.4BSD's framework in
recognisably the same form, thirty-two years on. FreeBSD discarded it
around 3.0.

**What this changes about a decision already made.** `i386: wd and fd
queue buffers the way this struct buf does` declined NetBSD 1.0's
`TAILQ_ENTRY(wd_softc) sc_drivechain` on the ground that it belonged
to "NetBSD's framework". It does not: it is 4.4BSD's framework, in
this tree, unused by six ports. Declining it was still right, and for
FreeBSD's reason rather than the one given -- this port is
`isa_driver`-based and converting is a separate piece of work, not a
queue fix.

**Not deferred lightly.** Converting the i386 to `cfdriver` would be
adopting this tree's own newer framework rather than importing
anything, and it is the design two surviving BSDs chose. It is
thirteen-odd files per port, and this kernel cannot mount a disk yet.
When it can, this is the first architectural question waiting.

`5a0ade80` is the finding with the widest consequences:

> 4.4BSD-Lite2 is therefore not a pure CSRG artefact. It is Berkeley's
> tree with a last round of its descendants' work merged in.

Berkeley's last months carry 98 commits importing from NetBSD,
FreeBSD and 386BSD. Of the 117 files this branch changes, five were
touched by one: `i386/i386/locore.s` and `i386/i386/machdep.c` (the
June 1993 NetBSD merge), `sbin/routed/trace.c`, `usr.bin/kdump/kdump.c`
and `usr.sbin/config/mkglue.c` (386BSD, 1991).

Two consequences the file draws: **"NetBSD 1.0 writes it this way" is
weaker evidence in `sys/i386` than elsewhere**, since it may be the
same text arriving by another route; and `291dffc7` showed the June
1993 merge actively *damaged* `locore.s` — before it the file had
eight correct `movw %ax,%ds` and no `movl`, and the merge replaced two
of them and the byte store. The corrections on this branch restore
Berkeley's own earlier text.

`b8596dbf`'s marker test (`%sccs.include.redist.c%` versus
`%sccs.include.proprietary.c%`) turns every "why is this missing"
question into a lookup.

`b2b31bae` recorded that `csu` came from NetBSD 1.6 **and should
have**: NetBSD 1.4's is three years closer to this tree's era, but its
`crtbegin.c` predates DWARF2 exception-frame registration, which this
toolchain expects. "ELF startup tracks the compiler and not the tree,
so here later is closer. It is the only place in this project where
that is true."
