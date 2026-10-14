/* Definitions of target machine for GNU CC,
   for the i386 running 4.4BSD on ELF, i386-unknown-bsd4.4elf.

   This file is part of GNU CC, and is distributed under the same
   terms; see the file COPYING.

   This system's ELF conventions are NetBSD 1.6's: its startup files
   (crt0.o, crtbegin.o, crtend.o, in lib/csu) and the types its headers
   give size_t, ptrdiff_t and wchar_t are NetBSD's, imported.  So this
   takes NetBSD's i386 ELF definitions whole and changes only where
   the systems differ:

   - the names predefined: this is not NetBSD, and it names itself as
     m68k/hp3bsd44.h names 4.4BSD on the hp300, with __BSD_4_4__ and
     the system assertion bsd;
   - the link, which is static: the system has neither shared
     libraries nor a dynamic linker.  */

#include <i386/netbsd-elf.h>

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-D__ELF__ -Dunix -D__BSD_4_4__ -Asystem(unix) -Asystem(bsd) -Acpu(i386) -Amachine(i386)"

#undef LINK_SPEC
#define LINK_SPEC \
 "-m elf_i386 \
  %{assert*} \
  -dc -dp \
  %{!nostdlib:%{!r*:%{!e*:-e __start}}} \
  %{static:-static}"
