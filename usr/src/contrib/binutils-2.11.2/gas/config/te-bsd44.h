/* te-bsd44.h -- target environment declarations for 4.4BSD on ELF,
   i386-unknown-bsd4.4elf.

   This file is part of GAS, the GNU Assembler, and is distributed
   under the same terms; see the file COPYING.

   As te-nbsd.h and te-freebsd.h do for the BSDs descended from this
   one, TE_BSD44 keeps tc-i386.c from taking '/' as a comment
   character, as it does for SVR4: on a BSD it is the divide operator. */

#define TE_BSD44	1

#define LOCAL_LABELS_FB 1

#include "obj-format.h"
