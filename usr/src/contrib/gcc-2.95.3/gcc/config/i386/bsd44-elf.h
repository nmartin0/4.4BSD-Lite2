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
     libraries nor a dynamic linker;
   - where the compiler's own programs and libgcc are: in
     /usr/libexec/gcc2 itself, as Lite2's gcc-2.3.3 had them, whose
     Makefile made that directory its libsubdir.  The driver finds
     them there by MD_EXEC_PREFIX, as netbsd.h's finds NetBSD's in
     /usr/libexec; GCC's own headers are in its include, where
     contrib/gcc-2.95.3/makefile's libsubdir puts them.  */

#include <i386/netbsd-elf.h>

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-D__ELF__ -Dunix -D__BSD_4_4__ -Asystem(unix) -Asystem(bsd) -Acpu(i386) -Amachine(i386)"

#undef MD_EXEC_PREFIX
#define MD_EXEC_PREFIX "/usr/libexec/gcc2/"

#undef LINK_SPEC
#define LINK_SPEC \
 "-m elf_i386 \
  %{assert*} \
  -dc -dp \
  %{!nostdlib:%{!r*:%{!e*:-e __start}}} \
  %{static:-static}"

/* AI-ONLY NOTE: added (2026-10-26) -- no `gcc2_compiled.' label at the
   start of each file's code.  netbsd-elf.h writes one into every object
   compiled without -g, for a debugger to know the compiler by; in the
   kernel, linked -X so ddb has its static functions' names, the label
   sat at the address of each file's first function, and where that
   function was static ddb named it `gcc2_compiled.'.  GCC's own
   definitions for the i386 under OSF/1, which runs on Mach, leave the
   label out in these words, i386/osf1elf.h and i386/osfrose.h: "We
   don't need or want this for OSF1", with OSF/1's reason given beside
   it: "To keep symbol space down, and not confuse kdb" -- Mach's
   kernel debugger, which ddb is.  With -g the label is a stabs entry,
   which gdb reads, written by ASM_IDENTIFY_GCC_AFTER_SOURCE; that is
   kept.  */
#undef ASM_IDENTIFY_GCC
#define ASM_IDENTIFY_GCC(FILE)
