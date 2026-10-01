# Roadmap

Three tiers, in dependency order. Tier 0 finishes what `dev`/`dev2`
set out to do. Tier 1 is the first tier where anything runs. Tier 2 is
4.5BSD's stated target and is listed so that nothing in it is
rediscovered later.

Nothing here is a schedule. Items are listed with what is known about
each, and with the donor where the record names one. `docs/deferred.md`
holds the reasoning behind most of them; this file holds the ordering.

## Tier 0 — finish the heritage restoration

The `dev`/`dev2` goal was 4.4BSD-Lite2 as Berkeley shipped it,
compiling under a modern GCC. Measured against the tree as it stands:

| | done | total |
|---|---|---|
| program directories that build | 302 | 320 |
| libraries built by `sysroot.sh` | 11 | 13 |
| kernel sources compiled | 136 | 853 |
| kernel configurations that build | 1 | 2 (i386) |
| ports whose kernel has ever been compiled | 1 | 8 |
| kernels that link | 0 | — |

So the userland restoration is substantially done and the kernel one
is not. What remains:

**The kernel link.** Four undefined symbols — `chrtoblk`, `fuswintr`,
`suswintr`, `wddriver`, plus `memset` on a gcc 14 host. Models for
each in `docs/status.md`. `libkern.a` is built by nothing.

**`GENERIC.i386` does not build.** It names a `cn0` console no driver
provides and selects no filesystem or pager. `LINK.i386` carries the
corrections; Berkeley's own ARGO and BLITZ configs have the corrected
console line, so fixing GENERIC would be kind A — Berkeley's code
corrected to Berkeley's own intent.

**The DIAGNOSTIC build does not compile.** `6a81e5fe`: `debug1` has an
incomplete type in `vfs_subr.c`. A whole build configuration is
broken and nothing tracks it.

**`wd`, `fd` and `wt` are out of `LINK.i386`.** Ten `b_actf`/`b_actb`
sites in `wd.c` and `fd.c`; `wt.c` fails separately on a static
declaration following a non-static one. Donor checked: FreeBSD 2.0.5
alone. These are the local disk, so Tier 1 needs them.

**Seven ports have never been compiled at all** — hp300 (74 sources),
luna68k (59), sparc (55), pmax (48), news3400 (57), vax (61), tahoe
(161). Whether 4.5BSD keeps them is an open question, but "4.4BSD-Lite2
compiles under modern GCC" is not true of the tree until they do or
are removed deliberately.

**`libresolv` and `liby` are not built.** Every other library in
`lib/` is.

**18 program directories still fail**, each for a reason already
recorded: `usr.bin/pascal` (`libcpats.c` absent from Lite and Lite2),
`usr.bin/vacation` (`${LIBDBM}`, a library that does not exist — its
dbm is `ndbm.o` inside libc), `usr.bin/vmstat.sparc` and
`usr.sbin/eeprom` (sparc-only), `libexec/kpasswdd` (is Kerberos), and
13 games — three wanting `-fwritable-strings`, two calling `exit` with
no argument, two passing the wrong type to `log`, two building a
generator and running it, two wanting X11 headers, one declaring a
static function inside another, one wanting `gtty` from libcompat's
absent 4.1 sources.

**A build-ordering defect, found while measuring this and recorded in
no commit.** Eight directories fail a first `make.sh NOMAN=noman
depend all` and succeed on a second, with no change between the runs:
`sbin/disklabel`, `sbin/restore`, `sbin/route`, `usr.bin/lorder`,
`usr.bin/pagesize`, `usr.bin/shar`, `usr.sbin/arp`, `games/phantasia`.
Cause not yet established. Until it is, any sweep of the tree
undercounts, and the counts in this file were taken from a second
pass for that reason.

**`YACC` still names the packaged byacc** although `sysroot.sh` has
built this tree's own into `tools/bin` since `dc51dc50` and `make.sh`
puts that directory first on `PATH`. One line. It would let
`libexec/ftpd/ftpcmd.y` stand as Berkeley wrote it, since its `static`
on `yylex` was dropped only to satisfy byacc.

## Tier 1 — boot on QEMU's legacy machine

The first tier where anything runs, and the one every later tier
depends on: nothing in Tier 2 is testable until a kernel boots
somewhere. QEMU's legacy PC — PIIX3, IDE, NE2000, 8259 PIC, 8254
timer, PS/2 keyboard — is hardware this kernel already almost
recognises, which is what makes it the right target rather than a
detour.

1. **Close the link.** Tier 0's four symbols.
2. **Build `libkern.a`.** Needs a decision on kernel-versus-userland
   include paths, since `bcmp.c` includes `<string.h>`.
3. **Load the kernel.** `i386/stand/boot.c:122` reads `struct exec`
   and accepts only `a_magic` 0407/0410/0413; the kernel links ELF.
   The kernel loads at **physical 0** — `boot.c:148` masks the entry
   with `& 0x000fffff` and `locore.s` reaches its variables as
   `symbol-SYSTEM` with `SYSTEM` = `0xFE000000` — and `start:` writes
   to `0x472` then `.space 0x500` to skip the BIOS data area it
   overlays. Entry contract: `(*entry)(howto, opendev, 0, cyloffset)`
   on the stack. Options in `docs/status.md`; the cheapest first
   experiment is gdb driving QEMU with no loader at all, which
   commits to nothing and answers whether `start` executes.
4. **`DELAY()`.** Before changing it, measure every call site in the
   configured kernel and whether `findcpuspeed()` calibrates sanely
   under emulation. If nothing asks for under a millisecond,
   Berkeley's loop stays. If something does, the answer the record
   already names is the 8254 read, `n * TIMER_FREQ / 1e6` with a
   64-bit intermediate, about thirty lines, four trees agreeing.
5. **The console.** `pccons.c` compiles for the first time in this
   tree's history and has never run. Serial via `com.c` is the more
   debuggable first console.
6. **The settlement bodies.** 34 of the 35 emptied functions are in
   this configuration. Nothing mounts a filesystem without
   `vfs_bio.c`'s fourteen; nothing runs a program without `execve`;
   no tty works without `tty_subr.c`'s ten. Restoration, not
   modernization — removed by a lawsuit rather than a design
   decision. Donor to be chosen by measuring which tree's bodies are
   lineal continuations rather than clean-room rewrites.
7. **`wd.c` for the IDE disk**, which QEMU's PIIX3 presents in
   compatibility mode. FreeBSD 2.0.5 is the donor.
8. **`if_ne` for the NE2000**, which QEMU emulates.
9. **Root filesystem, `init`, single user.**

## Tier 2 — modern hardware

Each of these is a measured absence, not a stylistic gap. Counts are
from the tree as it stands.

**No PCI bus.** Zero files. A grep for `pci` across the whole kernel
returns nine hits and every one is a false positive — `profile.h`'s
`_mcount` internals, `pccons.c`, `nfs.h`, a test program. Nothing on a
modern machine is findable without this, and everything below depends
on it. `sys/i386/eisa/` and `sys/i386/mca/` each contain a single
`tags` file and no code.

**No ACPI.** Zero files. Post-2000 machines describe interrupt
routing, power and device topology there.

**No APIC or IOAPIC.** Zero files. The 8259 PIC is still emulated on
modern chipsets but increasingly vestigially.

**Build-time interrupt wiring.** `usr.sbin/config/mkglue.c` generates
`vector.s` with one stub per configured interrupt and the handler
baked in. Two properties of modern interrupt hardware cannot be
expressed in it at all: PCI lines are *shared*, so a handler chain
must be walked at run time and its length is unknown until the bus is
enumerated; and MSI/MSI-X allocate a vector per device from a pool
when the device is found, so the vector number does not exist at
config time. Neither is a performance argument.

  `5a0ade80` establishes that `mkglue.c` came from 386BSD in January
  1991 as "386BSD additions to config" and was never Berkeley's
  design, which is why every later tree dropped it rather than
  maintaining it. **No Berkeley heritage is lost by replacing it.**
  Donors: NetBSD 1.1 has a static `vector.s` dispatching through a
  run-time table; OpenBSD 1996 has no `mkglue.c` at all; FreeBSD
  2.0.5 carries `register_intr()`/`unregister_intr()` in an `isa.c` of
  1056 lines against this tree's 254, and every driver's attach path
  changes with it.

**No storage driver that matches anything.** `wd.c` is an ST-506/IDE
driver from 1991. No AHCI, no NVMe.

**No USB.** On many machines that means no keyboard.

**No modern NIC.** `if_ne` is an NE2000; `if_we`, `if_ec` and
`if_apx` are of the same era.

**32-bit, no PAE.** The pmap assumes a 32-bit physical address space.
Modern machines have more than 4 GB.

**BIOS boot only, and the bootstrap reads a.out.** UEFI-only machines
cannot boot this at all.

**Uniprocessor.** No kernel threading; `struct proc` is the unit of
scheduling and there is no thread abstraction to carry forward.

### The question Tier 2 cannot start without

"Modern hardware" spans two targets that differ enormously in scope:

- **roughly 2005–2015**: legacy BIOS, PCI, IDE-compatible SATA,
  USB-legacy or PS/2 input. Large work, but every piece has a lineal
  BSD implementation that could be traced.
- **2020 onward**: UEFI-only, NVMe, no CSM, USB-only input. Almost
  none of that has 4.4BSD ancestry to be genetic about, because it
  postdates the lineage.

Which one 4.5BSD targets decides whether Tier 2 is six items or
twenty, and it should be settled before any Tier 2 work begins.
