# Eight small, unrelated fixes ("the smaller ones")

plan-4.5.md stage 2, item 6. Eight small, unrelated fixes bundled
into one patch because each is a single-issue, single-file (or
single-pair-of-files) donor-fit fix too small to carry its own
patch: `re_comp`, `pstat`, ddb's ELF sanity checks, `wcd`'s TOC
length check, the ELF loader's specified-address rounding, the
`strtoimax`/`strtoumax` "0x" prefix bug, `uucplock`'s `sprintf`, and
two man-page corrections.

## 1. `re_comp`/`re_exec`: non-`const` parameter, and the empty-string case

`usr/src/lib/libcompat/4.3/regex.c`.

**The hole.** `re_comp(s)` took `char *s`, but `usr/src/include/unistd.h`
already prototypes it `char *re_comp __P((const char *));` -- any
caller passing a `const char *` (the normal way to call it) mismatches
the actual definition, and relies on the prototype rather than the
definition to warn about it. Separately, `re_comp()`'s `s == NULL`
check did not also cover `s[0] == '\0'`: `re_comp.3` documents "If
re_comp is passed 0 or a null string, it returns without changing the
currently compiled regular expression," but the code recompiled an
empty pattern instead, which (as any regex library's empty pattern
does) matches everything -- so `re_comp("")` silently discards the
caller's previously compiled pattern and turns the next `re_exec()`
into an always-match.

**The fit.** NetBSD 1.1, pk, regex.c rev 1.5, `$Id: regex.c,v 1.5
1995/06/05 19:42:25 pk Exp $`, changes the parameter to `const char *`.
NetBSD 1.2, jtc, regex.c rev 1.6, `$NetBSD: regex.c,v 1.6 1996/03/21
03:11:13 jtc Exp $`, adds the `*s == '\0'` check. Both are NetBSD,
first in donor order, and both are narrow single-issue fixes with no
overlap.

**The fix.** `re_comp`'s and `re_exec`'s parameters changed to
`const char *s;`. `re_comp()`'s guard changed from `if (s == NULL)`
to `if (s == NULL || *s == '\0')`.

**Test: `build/test/re_comp.c`/`.sh`/`.want`.** `#include`s the real
`regex.c` and `regexp.c` directly (a scratch directory holds just
`regexp.h`/`regmagic.h`, copied out of `usr/src/include` and
`usr/src/lib/libcompat/regexp` so `-I usr/src/include` doesn't also
pull in that tree's own `stddef.h`/`sys/types.h`, which shadow the
host's and fail on `<machine/ansi.h>`). Confirmed pre-fix the file
fails to build at all (parameter-type mismatch against `unistd.h`'s
prototype, `-Werror=incompatible-pointer-types`), and confirmed with
non-`const` prototypes substituted and the empty-string check removed,
`re_comp("")` and `re_comp(NULL)` each wrongly make `re_exec("xyz")`
match. Post-fix, both empty-string and `NULL` calls leave the
previously compiled pattern in effect, and a real `re_comp()` call
still takes effect normally. The `.sh` wrapper greps `unistd.h` for
both `const char *` prototypes and `regex.c` for the `*s == '\0'`
check.

**What is not established.** Only the exact two cited revisions were
checked directly; whether any NetBSD revision between 1.1 and 1.2
also touched this file is not relevant here since both donor fixes
land cleanly regardless.

## 2. `pstat`: a botched duplicate table entry with no name

`usr/src/usr.sbin/pstat/pstat.c`, the `mnt_flags[]` table.

**The hole.** The table had a duplicate, malformed `{ MNT_DELEXPORT
},` entry -- a struct initializer with only `m_flag` set, so C's
zero-initialization left `m_name` as `NULL` -- followed by a second,
exact duplicate of the already-present `{ MNT_UPDATE, "update" },`
line. The print loop, `for (i = 0; mnt_flags[i].m_flag; i++)`, stops
only when `m_flag` is `0`, never checks `m_name`, and the call site
does `printf("%s%s", sep, mnt_flags[i].m_name)` -- a `NULL` `m_name`
reaches `printf("%s", NULL)` directly.

**The fit.** NetBSD, mrg, pstat.c rev 1.33, `$NetBSD: pstat.c,v 1.33
1997/10/17 06:34:22 mrg Exp $` ("merge lite-2" -- the commit that
first gives this table its present, table-driven form), is NetBSD's
own table without the botched duplicate pair. Checked that no earlier
NetBSD revision has this table at all (rev 1.32 and before use a
different, `if`/`flags`-style implementation with no table), and that
no OpenBSD revision adopts the table form before this date (the
latest OpenBSD commit to this file before 1997-10-17, 1997-09-18,
still has no `mnt_flags[]` table), so rev 1.33 is the first-fit
revision across both checked donors.

*Correction after independent review:* an earlier draft of this note
cited NetBSD 1.3, drochner, pstat.c rev 1.38, `$NetBSD: pstat.c,v 1.38
1997/10/20 18:12:56 drochner Exp $`. That citation was wrong: rev
1.38's actual diff ("The previous changes were not sufficient, we
need more kvm reads") touches only `nfs_print()`'s `vattr` handling
and has nothing to do with `mnt_flags[]`. The reviewer traced the
table's real history in `/home/claude/donors/nb12h` (fine-grained CVS
history, unlike the squashed `nbhist.git` import) and found rev 1.33
instead -- confirmed independently as above before this correction
was made.

**The fix.** Removed the null-named `{ MNT_DELEXPORT },` entry and
the duplicate `{ MNT_UPDATE, "update" },` line that followed it.

**Test: `build/test/pstat_mntflags.sh`.** Text-based, no companion
`.c` file: an `awk`+`sed` `extract()` function pulls just the
table's entry lines (stripping comments), and `check_table()` asserts
no entry is missing a quoted name and no `MNT_*` flag token repeats.
Run first against a deliberately re-broken scratch copy (the removed
lines reinserted with `sed`, anchored `^\(\t{ MNT_UPDATE, "update"
},\)$` so the substitution cannot also match the identical text this
patch's own AI-ONLY NOTE quotes for explanation) to confirm the check
catches it, then against the real file.

**What is not established.** Whether this was a hand-edit error or a
merge artifact in this tree's own history before the donor fix; not
needed to apply or verify the fix.

## 3. ddb's ELF loader: no sanity check on a corrupt ELF header

`usr/src/sys/ddb/db_elf.c`, `X_db_sym_init()`.

**The hole.** The function computed `shp = (Elf_Shdr *)(symtab +
elf->e_shoff)` and then walked `shp[i]`/`shp[j]` to find the symbol
and string tables, with no check that `e_shoff` (or either found
section's `sh_offset`) was non-zero. A kernel loaded with a corrupt
or stripped-down ELF header (`e_shoff == 0`, or a `SHT_SYMTAB`/its
linked string-table section with `sh_offset == 0`) walks into
`symtab + 0`, i.e. the start of some unrelated buffer, as if it were
a section-header array.

**The fit.** NetBSD, bjh21, commit
`1d16f72d976293a3bbc0ac7027c6e7d54f604895` ("Extra sanity checking on
ELF headers to avoid being confused by corrupt kernels.",
2001-07-31, db_elf.c rev 1.16). Not on any `netbsd-1-*` release
branch -- found via `/home/claude/donors/nbhist.git`'s full history,
by trying the abbreviated hash recorded in
`docs/provenance/bestfit-sweep.tsv` row #646 directly against each
donor repo until `nbhist.git` recognized it.

**The fix.** `if (elf->e_shoff == 0) goto badheader;` before
computing `shp`, and `if (shp[i].sh_offset == 0) continue;` /
`if (shp[j].sh_offset == 0) continue;` inside the section-finding
loop.

**Test: `build/test/db_elf_sanity.c`/`.sh`/`.want`.** Standalone
`find_symtab()`, reproducing the real loop with a mock `struct shdr`
(the four fields the loop actually touches), since the real function
needs kernel-only types and can't be built on the host. Cases: a
normal table is found; `e_shoff == 0` is rejected; a symtab section
with `sh_offset == 0` is rejected; a linked string-table section with
`sh_offset == 0` is rejected. The `.sh` wrapper greps the real
`db_elf.c` for the `e_shoff == 0` check and exactly two occurrences of
`sh_offset == 0`, so the standalone reproduction can't silently drift
from the real code.

**Regression.** `build/kernel.sh GENERIC.i386`: `db_elf.c` recompiles
with no warnings; kernel links; `vmunix` unchanged at 737738 bytes.

## 4. `wcd`: unbounded TOC-entry count from the drive

`usr/src/sys/i386/isa/wcd.c`, the `CDIOREADTOCENTRYS` case.

**The hole.** `len` (the byte count requested for the TOC-entry
`copyout`) was checked only for `len <= 0`, with no upper bound
against `toc->tab`'s actual size -- a caller, or a miscomputed `len`
driven by a corrupt or hostile `toc->hdr.ending_track` (a `u_char`
read straight from the drive, 0-255, never clamped against the
drive's real track count), could drive `copyout` past the end of
`toc->tab`.

**The fit.** FreeBSD, Andrey Chernov (ache), commit
`e407db3cbc080bdbd6c540a4af3d2d201ddc02f5` ("READTOCENTRYS: protect
against stack overflow when incorrect data stored in CD's TOC",
1996-01-30). Found via `/home/claude/donors/fbh` tag `r220` and plain
`git log --oneline r220 -- sys/i386/isa/wcd.c` (not `-S` pickaxe,
which timed out against full history). FreeBSD is after OpenBSD and
NetBSD in donor order, but neither carries this file's history this
far back with this exact fix; FreeBSD fits first among those that do.

**The fix.** `if (len > sizeof(toc->tab)) return (EINVAL);` added
right after the existing `len <= 0` check.

**What this fix does NOT close (documented, not fixed here).**
Confirmed by direct inspection of `wcd_read_toc()` (lines ~898-925)
that a local `ntracks` is clamped to `MAXTRK` (99) for the actual TOC
read, but `toc->hdr.ending_track` itself is never clamped. This
check bounds only the `copyout` byte count. Two gaps remain,
independent of `len`:
(a) the MSF-format conversion loop just below computes its `e`
    pointer fresh from the same unclamped `ending_track`;
(b) `copyout`'s source offset depends on `te->starting_track`, which
    can itself be as large as the unclamped `ending_track` allows.
Confirmed directly that FreeBSD's own later/current tree still has
this same gap after this exact commit -- so this is not a case of
"the donor's later fix is skipped because it's out of scope for
4.5BSD," it is the literal donor fix in full, and the donor's own fix
was never complete.

**Test: `build/test/wcd_toclen.c`/`.sh`/`.want`.** Standalone
`toclen()` with mock `struct toc_hdr`/`struct toc` (the real
function is kernel-only). Cases: a normal TOC proceeds; a corrupted
`ending_track = 250` with a large requested `data_len` is rejected;
a corrupted `ending_track = 250` with a small `data_len = 16` is
explicitly asserted as "NOT caught by this check (documented gap)"
-- the test's expected value for that case is "not rejected," not a
false claim of full protection. The `.sh` wrapper greps the real
`wcd.c` for the exact added line.

**Regression.** `build/kernel.sh GENERIC.i386`: `wcd.c` recompiles
with no new warnings; kernel links; `vmunix` unchanged at 737738
bytes (confirms the common/correct-data case is behaviorally
unaffected).

## 5. ELF loader: `*addr` rounding rounds an already-aligned address up anyway

`usr/src/sys/kern/exec_elf.c`, `ELF_ALIGN` / `elf_load_psection()`.

**The hole.** The single macro `ELF_ALIGN(a, b) ((a) & ~((b) - 1))`
was used at three sites in `elf_load_psection()`: two that want to
round a value *up* to the next alignment boundary (computing the
caller-specified `*addr`), and one that wants to *truncate* a value
down (finding a segment's own aligned base from `ph->p_vaddr`).
`ELF_ALIGN` alone only truncates; the two "round up" call sites
instead wrote `ELF_ALIGN(*addr + ph->p_align, ph->p_align)`, which
unconditionally adds a full `p_align` before truncating -- so an
`*addr` that is already aligned lands one full alignment unit past
where it started, instead of being left unchanged, which is what
"round up" means for an already-aligned value.

**The fit.** NetBSD, cgd, `sys/kern/exec_elf32.c` rev 1.18, `$NetBSD:
exec_elf32.c,v 1.18 1996/10/11 19:44:02 cgd Exp $` -- the commit that
introduces `ELF_ROUND`/`ELF_TRUNC` in place of the single `ELF_ALIGN`,
with the commit message itself identifying exactly this bug: "This
fixes a bug found by Robert Baron <rvb+@cs.cmu.edu> where
elf_load_psection, if given a properly aligned address at which to
load the section, would [...] actually load it at the next highest
alignment boundary." NetBSD 1.1's own `kern/exec_elf.c` rev 1.3,
`$NetBSD: exec_elf.c,v 1.3 1995/09/16 00:28:08 thorpej Exp $` -- the
revision this tree's `elf_load_psection` was itself adapted from (see
that file's own AI-ONLY NOTE) -- still has the single, buggy
`ELF_ALIGN`, confirming rev 1.18 as the fix and rev 1.3 as the
pre-fix state this tree inherited.

*Correction after independent review:* an earlier draft of this note
cited NetBSD 1.3, mycroft, `exec_elf32.c` rev 1.26.8.1, `$NetBSD:
exec_elf32.c,v 1.26.8.1 1998/05/05 09:56:31 mycroft Exp $`. That
revision is real (found in `/home/claude/donors/nb12h`, whose commit
message is "Pull up 1.27, per request of scottb") and does carry the
fix, but it is a release-branch pullup of a fix already nearly a year
and a half old by then, not the fix's origin -- rev 1.18 is the
earliest revision on NetBSD's own trunk that introduces it, and is
byte-for-byte identical in its macro definitions and both call-site
changes to what this tree ports. "First canonical fit" means the
earliest fix, and rev 1.26.8.1 was not that; corrected to rev 1.18.

**The fix.** `ELF_ALIGN` replaced with `ELF_ROUND(a, b) (((a) + (b) -
1) & ~((b) - 1))` and `ELF_TRUNC(a, b) ((a) & ~((b) - 1))`. In
`elf_load_psection()`: the `*addr != ELF32_NO_ADDR` branch's `*addr =
ELF_ALIGN(*addr + ph->p_align, ph->p_align)` becomes `*addr =
ELF_ROUND(*addr, ph->p_align)`, and that branch's `uaddr =
ELF_ALIGN(ph->p_vaddr, ph->p_align)` becomes `uaddr =
ELF_TRUNC(ph->p_vaddr, ph->p_align)`; the `else` branch's `*addr =
ELF_ALIGN(uaddr, ph->p_align)` becomes `*addr = ELF_TRUNC(uaddr,
ph->p_align)` -- a purely cosmetic rename there, since `ELF_TRUNC` is
bit-for-bit the old `ELF_ALIGN`.

**Reachability, stated plainly.** `kern_exec.c`'s only caller of
`elf_load_psection` always passes `addr = ELF32_NO_ADDR`, so the
`*addr != ELF32_NO_ADDR` branch -- where the actual bug lives --
never executes in this tree today. Fixed anyway: latent-but-wrong
code is still wrong code, and this is not a fix skipped for being out
of scope, it is applied in full even though its live effect today is
nil.

**Test: `build/test/elf_round.c`/`.sh`/`.want`.** The `.sh` wrapper
greps the real `exec_elf.c` for the `ELF_ROUND`/`ELF_TRUNC` `#define`
lines and extracts them verbatim into a scratch header (the rest of
that file needs kernel-only types and can't be built on the host),
which `elf_round.c` `#include`s and tests directly on plain integers
-- including an explicit demonstration that the pre-fix expression
`(a + b) & ~(b - 1)` overshoots an already-aligned address by a full
`b`, while `ELF_ROUND` leaves it unchanged. The wrapper also confirms
`ELF_ALIGN` is gone and that `elf_load_psection()` uses `ELF_ROUND`
for `*addr`.

**Regression.** `build/kernel.sh GENERIC.i386`: `exec_elf.c`
recompiles with no new warnings (one pre-existing, unrelated "vref
prototype follows" warning from `vnode.h`); kernel links; `vmunix`
unchanged at 737738 bytes -- confirms the live path's rename is
behaviorally inert, as expected since it never runs the changed
arithmetic.

## 6. `strtoimax`/`strtoumax`: "0x" with no hex digit after it fails entirely

`usr/src/lib/libc/stdlib/strtoimax.c`, `strtoumax.c`.

**The hole.** Both functions unconditionally consumed a leading
`"0x"`/`"0X"` and committed to base 16 whenever `base` was 0 or 16 and
the next two characters looked like a hex prefix -- with no check
that anything after the "x" was actually a hex digit. An input like
`"0xg"` (base 0) consumed "0x", then failed outright on the non-hex
`'g'`: `EINVAL`, `endptr` reset all the way back to the start,
discarding the valid leading `"0"`. Standard `strtol`-family
semantics (confirmed against POSIX and glibc's own behavior) are that
such input should fall back to parsing just the leading `"0"` as
octal, leaving `endptr` at the `'x'`.

**The fit.** FreeBSD. The exact commit is not recoverable: `$FreeBSD$`
is never expanded (left as the literal four-character string) in any
donor mirror checked (`freebsd`, `fbh`), consistent with this
project's documented mirror-graph corruption elsewhere. Direct
inspection narrows it to absent at `release/5.3.0` and present at
`release/5.5.0` in `/home/claude/donors/freebsd` -- bisecting further
was not possible with the tools available, so this imprecision is
stated plainly rather than guessed past.

**The fix.** Both functions: before committing to the hex-prefix
branch, check that `s[1]` (the byte after the "x") is `'0'`-`'9'`,
`'A'`-`'F'`, or `'a'`-`'f'`; only then consume "0x" and set `base =
16`. Otherwise `c` (still `'0'`) and `base` fall through unchanged, so
the existing octal/decimal path takes over.

**Test: `build/test/strtoimax_0x.c`/`.sh`/`.want`.** `#define
strtoimax my_strtoimax` / `#define strtoumax my_strtoumax` before
`#include`ing both real `.c` files directly (pure string/arithmetic
code, no OS dependency, so this doesn't risk drifting from a copy).
Cases: `"0xg"` at base 0 and base 16 (both signed and unsigned);
`"0x"` alone; real hex (`"0x1A"`, `"0xFF"`); plain octal (`"017"`) and
decimal (`"17"`); a valid hex prefix with trailing garbage
(`"0x2Fxyz"`), which must still parse the hex part in full. Confirmed
pre-fix (substituting the last-committed, pre-this-patch source via
`git show HEAD:...`) that all three "0x with no following hex digit"
cases genuinely fail -- `errno == EINVAL`, zero bytes consumed --
while every other case behaves identically before and after the fix.
The `.sh` wrapper greps both real files for the `s[1] >= '0' && s[1]
<= '9'` check.

**What is not established.** The exact FreeBSD commit and its date/
author are not recoverable from any mirror checked; only the
release-tag bracket (absent at 5.3.0, present at 5.5.0) is confirmed.

## 7. `uucplock`: unbounded `sprintf` into a fixed-size lock-path buffer

`usr/src/usr.bin/tip/uucplock.c`, `uu_lock()` and `uu_unlock()`.

**The hole.** `tbuf`, sized `sizeof(_PATH_LOCKDIRNAME) + MAXNAMLEN`
(280 bytes: the 25-byte format string `"/var/spool/lock/LCK..%s"`
plus `MAXNAMLEN` (255)), was filled with `sprintf(tbuf,
_PATH_LOCKDIRNAME, ttyname)` -- unbounded. `ttyname` is a plain `char
*` parameter with no length check of its own in either function; a
caller passing a name longer than `MAXNAMLEN` overflows `tbuf`.

**The fit.** OpenBSD, deraadt, commit
`bc4b3bff0fca454be16f4ea92d04680f66fcba50` ("avoid oflow",
1997-08-24), carried into NetBSD's own uucplock.c the same year (rev
1.7/1.8, per that commit's own `$NetBSD:` header line). First in
donor order; single-issue (only these two `sprintf` calls change in
that commit -- a third `sprintf`, building `text_pid` from a `pid_t`
into an 81-byte buffer, is not touched by the donor commit and is not
touched here, since `"%10d\n"` of any `pid_t` cannot exceed about 12
bytes and is not the overflow this commit addresses).

**The fix.** Both `sprintf(tbuf, _PATH_LOCKDIRNAME, ttyname)` calls
(in `uu_lock()` and `uu_unlock()`) changed to `snprintf(tbuf,
sizeof(tbuf), _PATH_LOCKDIRNAME, ttyname)`.

**Reachability, stated plainly.** This tree's only caller
(`hunt.c`'s `hunt()`) passes the basename of a device path found
under `_PATH_DEV`, which is short in practice, so the overflow is not
reachable through this tree's own code today. Fixed anyway:
`uu_lock`/`uu_unlock` are general-purpose, file-scope routines, not
private to one caller, and latent-but-wrong code is still wrong code.

**Test: `build/test/uucplock_snprintf.c`/`.sh`/`.want`.** `uu_lock()`
itself can't be called on the host (it does real `open()`/`kill()`),
so this reproduces just the one line that changed: the same `tbuf`
declaration and format string, in a `static` (not stack-local)
`struct probe { tbuf; canary[512]; }` -- static so an overrun lands in
ordinary `.bss` storage that this test can inspect, rather than
corrupting the stack frame or (as a first attempt demonstrated) a
different static variable entirely. Cases: a short name leaves the
canary intact with either `sprintf` or `snprintf`; a 400-byte name
smashes the canary with the unbounded `sprintf` and leaves it intact
with the bounded `snprintf`. The `.sh` wrapper greps the real file
for exactly two `snprintf(tbuf, sizeof(tbuf), ...)` calls and checks
`_PATH_LOCKDIRNAME`/`MAXNAMLEN` against `pathnames.h`/`sys/dirent.h`
so the reproduction can't drift from them. Confirmed against a
scratch copy of the pre-fix file that the `.sh` wrapper's own grep
guard refuses to run (rather than silently passing) when the fix is
absent.

## 8. Two man-page corrections

### 8a. `m4.1`: "loosing" for "losing"

`usr/src/usr.bin/m4/m4.1`, bestfit-sweep.tsv row #370.

**The hole.** "They may be redefined, loosing their original
meaning." -- "loosing" (releasing, setting free) for "losing"
(ceasing to have).

**The fit.** NetBSD, jtc, m4.1 rev 1.4, `$Id: m4.1,v 1.4 1995/10/19
07:52:46 jtc Exp $`, on the `netbsd-1-2` branch. First in donor
order; a plain spelling fix.

**The fix.** "loosing" -> "losing".

### 8b. `deroff.1`: `-m m` and `-m s` both say they recognize `me(7)`

`usr/src/usr.bin/deroff/deroff.1`, bestfit-sweep.tsv row #567.

**The hole.** The `-m` option's argument table said, for both `m`
and `s`: "recognize me(7) macros." -- each names the wrong macro
package (`me(7)` instead of its own). The `l` entry said "recognize
mm(7) macros and delete mm(7) lists," which is also wrong: confirmed
directly in `deroff.c`'s `getopt` switch (lines ~334-356) that `-m l`
only sets `disp = YES` and does not set `mac = MM`, and that `disp` by
itself (checked at line 1469) just suppresses list-construct display,
independent of which macro package (if any) `-m m`/`-m s`/etc. also
selected.

**The fit.** OpenBSD, jmc, deroff.1 rev 1.6, `$OpenBSD: deroff.1,v 1.6
2005/07/05 15:02:20 jmc Exp $` ("sort description of -m / correct
typos in -m description / formatting improvements", from
wiz@netbsd) -- corrects `-m m` to `mm(7)` and `-m s` to `ms(7)`.
OpenBSD, jmc, deroff.1 rev 1.7, `$OpenBSD: deroff.1,v 1.7 2005/07/05
15:40:04 jmc Exp $` ("little bit better description of -ml; agreed
w/ wiz@netbsd") -- replaces the `l` entry's wording with "remove list
constructs." Both are OpenBSD, first in donor order. Only the
substantive corrections from these two commits are taken; their
combined, unrelated reordering and `.Dq`/`.Po`/`.Pc` formatting
rewrite of the rest of the page is not.

**The fix.** `-m m`'s entry: "recognize me(7) macros." -> "recognize
mm(7) macros." `-m s`'s entry: "recognize me(7) macros." -> "recognize
ms(7) macros." `-m l`'s entry: "recognize mm(7) macros and delete
mm(7) lists." -> "remove list constructs."

**Test: `build/test/manpage_fixes.sh`.** A man page has no behavior
to execute, so this checks (a) the wrong wording is gone and the
corrected wording is present in both pages, via targeted `grep`, and
(b) both pages still parse as `mdoc` (`groff -mdoc -Tascii -ww`,
warnings treated as failures). Confirmed against a scratch copy
holding the pre-fix text of each page in turn that the check
correctly reports the fix as missing (not a false pass) for each one
independently.

## Independent review

An independent reviewer (fresh context, no access to the reasoning
above) checked all eight fixes against their cited donor commits
directly in the mirrors, traced the `wcd` "incomplete donor fix" and
`exec_elf.c` "unreachable path" claims by hand, and ran and tried to
break all eight tests. Findings: two of the eight donor citations
(item 2, `pstat.c`; item 5, `exec_elf.c`) were wrong -- in both cases
a real, earlier, better-fitting donor commit existed and was missed,
which is itself a "first canonical fit" violation. Both sections
above have been corrected and the citations re-verified independently
(by re-deriving each from the donor mirrors a second time, not by
taking the reviewer's word for it) before this patch was committed;
see each section's "Correction after independent review" note. The
remaining six fixes, the `wcd` and `exec_elf.c` scope/reachability
claims, and all eight tests were confirmed accurate and solid, with
no changes needed.

## What is not established, project-wide for this patch

Beyond what each section above states on its own: no attempt was made
to find whether any of these eight holes has a second, unrelated fix
on some other donor branch that might be a better (not just first)
fit -- that comparison is 4.6BSD's job, not 4.5BSD's.
