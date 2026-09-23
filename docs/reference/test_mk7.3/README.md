# Reference import: test_mk7.3

Copied **verbatim** from `github.com/nmartin0/test_mk7.3` at commit
`fb0c4d515d7232b3fc66c416f8d99d38934af1e1`. Every file here except
this README is byte-identical to that commit; `sha1sum` values are
below. The copies are not edited: corrections and adaptations for
this project belong in our own files, not here.

**"This project" in these files means test_mk7.3** -- OSF Mach Kernel
7.3 with LITES, booted under QEMU -- not this 4.4BSD-Lite2 port. Their
numbers, addresses, file paths and build commands describe that tree.

## Why each file is here

| file | used for |
|---|---|
| `RULES.md` | the working rules, written to be carried to another project |
| `PRINCIPLES.md`, `AGENTS.md`, `WORKFLOW.md` | the reasoning and worked cases behind those rules |
| `DEBUGGING.md` | QEMU, gdb and monitor traps; most are not Mach-specific |
| `docs/METHODOLOGY.md`, `docs/SHELL.md` | general debugging and shell method |
| `docs/GIT-HYGIENE.md` | commit and history conventions |
| `build/mksandbox.sh` | precedent: the modern-GCC flags test_mk7.3's 32-bit i386 kernel needed, each tied to the failure that forced it |
| `tools/*.py` | QEMU serial console and monitor memory readers |

## Mach-specific statements to watch for

- `tools/vmem.py`'s docstring says the kernel is relocated by
  segmentation with base `0xC0000000`. That is OSFMK. It is not
  established for our kernel.
- `tools/vgadump.py` probes `0xa0000` because OSFMK's `kd` console wrote
  there.
- `tools/console.py`'s header refers to `boot-ide.sh` and
  `boot-debug.sh`, which are not in this repository.
- `DEBUGGING.md` and `AGENTS.md` describe ODE, MIG, `Buildconf`, the
  `-r` boot flag, and OSFMK addresses throughout.

## Not imported, and why

State documents of that project (`HANDOFF.md`, `ROADMAP.md`,
`docs/current-blocker.md`, `docs/lites-survey.md`,
`docs/bootstrap-fork.md`, `docs/archive/`), and its ODE, LITES, minix
and boot tooling. They record another system's state and would read
as claims about ours. They remain available at the commit above.

## Known discrepancies in test_mk7.3 and new_mk, measured

Recorded so nobody trusts the wrong half. None is corrected here.
Paths and commit ids in items 1-3 are test_mk7.3's.

1. `docs/current-blocker.md` and commit `ea32115` say LITES's
   `include/i386/stdarg.h` is vendor-pristine again. The shipped
   `tools/lites/lites-osfmk73.patch` still adds a `/* CONTROL: ... */`
   line and deletes the non-`KERNEL` `va_arg` arm, checked against
   pristine `nmartin0/lites-1.1.u3`.
2. Commit `df53983` and `docs/current-blocker.md` say the `argv_space`
   comment in `server_init.c` "now says" the table is unused. `df53983`
   does not touch the patch; the only commit that wrote that comment is
   `8ca2810`, and it says `argc == 0` "always".
3. `tools/lites/build-lites.sh` defines `MAKEARGS` and never uses it.
4. `github.com/nmartin0/new_mk`, a one-commit squash of test_mk7.3,
   records `osfmk7.3` as a submodule gitlink `1d9150b` with no
   `.gitmodules`, so a clone of it cannot populate the vendor tree.

## Licensing

These files are Nicholas Martin's work, imported from another project
of his. test_mk7.3's own README records that the licence for its
`build/` contributions is not yet decided, and no licence is declared
for its documents either.

So no licence is asserted for them here. Nothing in this directory may
be redistributed on the assumption that this tree's terms cover it:
4.4BSD-Lite2's licence (see ./COPYRIGHT at the top of this tree)
applies to the Berkeley material, not to these files. The copyright
holder is the maintainer of this tree, so the question is his to
settle when it matters.

The same applies to the scripts this tree adds under `build/`: they
carry an SPDX-FileCopyrightText line and deliberately no
SPDX-License-Identifier, pending that decision.

## sha1 of each imported file

```
84d4743d9e4a0d4acbae8b324f975c2dd0e037cd  AGENTS.md
5ac0bd57c084dcc354c41b5f24ec74b717e0f30d  DEBUGGING.md
9b9a820cd74690ccfb0fdbff8a76f0457602b7ae  PRINCIPLES.md
e0882b8df7b0c3a581a6258d86078be1be6ac6f8  RULES.md
884ad3b3a3087435d79351f0af4bfd99c14eaeb8  WORKFLOW.md
a83c5a3b58c8e18e42894ebcf5885dcd695685ca  build/mksandbox.sh
deed9fc0a3ef801d4e7e9333a6f8cc11fa62c1ba  docs/GIT-HYGIENE.md
3ace56592c37b5d082e3c2adc24c157b59826409  docs/METHODOLOGY.md
358359a87b151ef76b0eafcb275d21b7ff707dae  docs/SHELL.md
bc6c4b57f0d501920587727c0754cb53530209d0  tools/console.py
7a9127bbfb3eb058cd0f4c2d2cd512ac60e312c7  tools/pmem.py
61a4928032df6ef45ba36fd35118d4a380ca1bf2  tools/vgadump.py
40daef1d2e9ebe8a9b35048bf202a49760ce9a8f  tools/vmem.py
```
