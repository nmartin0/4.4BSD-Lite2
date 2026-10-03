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

### Programs that still do not build

| program | why | commit |
|---|---|---|
| `libexec/kpasswdd` | is Kerberos | `ece8f3da` |
| `usr.bin/vacation` | `${LIBDBM}` names a library this tree does not build; its dbm is inside libc as `ndbm.o`. NetBSD 1.0 and FreeBSD 2.0.5 carry the same stale dependency | `7dc30e7d` |
| `usr.bin/pascal` | stops on `libcpats.c`, a source absent here and from 4.4BSD-Lite | `02feb631` |
| `usr.sbin/eeprom` | includes `<machine/openpromio.h>`, sparc only | `b1e714d4` |
| `usr.bin/vmstat.sparc` | a sparc program | `b1e714d4` |
| 14 of 44 games | see below | `74b0fa86`, `1e9f5155` |

The fourteen games, each reason read from that program's own build
log: two build a generator and run it, **three pass
`-fwritable-strings`** (no modern GCC has it), two call `exit` with no
argument, two pass the wrong type to `log`, one declares a static
function inside another, one wants `gtty` from libcompat's absent 4.1
sources, two want X11 headers, one wants nroff. `games/ching` has no
`ching.6` here or in 4.4BSD-Lite.

### Sources and directories named but absent

`dd3de94e`, `3a30baa0`, `02feb631`, `b8596dbf`

Twenty-five programs, thirty-six documents and several sources are
named by build lists and are not here. `b8596dbf` established that
this is a **lookup, not an inference**: the Lite cut was mechanical
and kept `%sccs.include.redist.c%` while dropping
`%sccs.include.proprietary.c%`. The seventeen no BSD has are the
AT&T-derived tools (`bc`, `dc`, `spell`, `learn`, `struct`,
`diction`, `ptx`); the eight every BSD has are their own later
rewrites (`ed`, `expr`, `at`, `cron`, `sa`), plus `compress`, dropped
for the LZW patent rather than the licence.

`lib/libcompat` lost thirteen of twenty-two sources. `4.1` holds five
manual pages and no code at all; `4.3` is missing `ecvt.c`, `gcvt.c`,
`sibuf.c`, `sobuf.c`, `strout.c`. **`ecvt` and `gcvt` are POSIX** —
this is a conformance hole, not just a missing file. `regex.c` was
restored from NetBSD 1.0 (`fd425062`), and `b8596dbf` found that it is
*not* Berkeley's 407-line engine (marked proprietary) but a 93-line
shim over the `regexp` engine already here.

`usr.bin/grep` lost `old.bin.grep`, `old.egrep`, `old.fgrep`,
`old.ucb.grep`; `diff` lost `diffh`; `pascal` lost `eyacc`. All absent
from 4.4BSD-Lite too.

### The one inversion, stated by the project itself

`7cb61884`

> 741 lines were imported so savecore could compress a crash dump,
> while ten sites of adaptation that would give the kernel a disk
> driver were declined as too large. By functional importance that is
> backwards, and the reason given at the time — reaching a link sooner
> — was about momentum rather than authenticity.

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

### `DELAY()` rounds anything under a millisecond to nothing

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

### `cpu_model` is empty; nothing detects the processor

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

### The kernel has no local disk

`adb8a806`, `4326a65b`

`wd`, `fd` and `wt` are out of `LINK.i386`. `wd.c` and `fd.c` use the
4.3BSD queue names `av_forw`, `b_forw`, `b_actl` against a `struct
buf` that has had `b_actf`/`b_actb` since before 4.4BSD-Lite — nine
sites, seven in `wd.c` and two in `fd.c`. `wt.c` is out for a
different reason: a static declaration of `cmds` following a
non-static one.

**Donor checked, not assumed: FreeBSD 2.0.5 alone.** NetBSD 1.0
rewrote `wd.c` around a softc and TAILQ; OpenBSD 1996 ships none.

### `GENERIC.i386` is left broken on purpose

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

### The DIAGNOSTIC build does not compile

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

### The specs file stands in for a cross-compiler

`9e6bb751`

> No BSD uses a specs file; they build a compiler for their target,
> and then `-B` suffices. This is a stand-in for a cross-compiler and
> goes away when there is one.

Chosen over teaching Lite2's rules to cross-link as **NetBSD 10's
`bsd.prog.mk`** does, "because it leaves the tree untouched and needs
no unpicking later".

### `YACC` still names the packaged byacc

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

### Other build items

- **No `LIBCRT0` dependency** — changing `crt0.o` does not relink
  programs; NetBSD has it (`ad7131e0`).
- **468 dangling `obj` symlinks** shipped in the tarball; `make obj`
  is not used, object directories are made outside instead
  (`1da402c0`).
- **`-I-` is obsolete** in GCC and warns once per compile; used for
  NetBSD's include order in the csu Makefile (`c5fd6177`).
- **`tsort` reports loops** among libc's profiling objects. OpenBSD
  silences them with `tsort -q`; **GNU tsort rejects `-q` and prints
  nothing at all**, which would install an empty `libc.a` silently.
  Measured both ways; the notices stay (`1da1369b`).

## 5. Known defects carried knowingly

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
  `a_magic` 0407/0410/0413. Nothing in this tree can load the ELF
  kernel it now builds.

## 6. Things established that change how the rest reads

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
