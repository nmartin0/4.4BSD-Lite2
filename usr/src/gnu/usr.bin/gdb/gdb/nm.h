/* Native-dependent definitions for Intel 386 running BSD Unix, for GDB.
   Copyright 1986, 1987, 1989, 1992, 1996 Free Software Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef NM_FBSD_H
#define NM_FBSD_H

#define	ATTACH_DETACH

/* AI-ONLY NOTE: FreeBSD 2.2.8's shared library support -- <solib.h>
   and, below, the a.out `sod' names solib.c expects -- is out
   (2026-10-27): this system has no shared libraries, no <link.h> and
   no dynamic linker, and solib.c is not built. The GNU i386bsd
   configuration, nm-i386bsd.h, has none either.  */

/* This is the amount to subtract from u.u_ar0
   to get the offset in the core file of the register values.  */

#include <machine/vmparam.h>
#define KERNEL_U_ADDR USRSTACK

#define REGISTER_U_ADDR(addr, blockend, regno) \
	(addr) = i386_register_u_addr ((blockend),(regno));

extern int
i386_register_u_addr PARAMS ((int, int));

#define PTRACE_ARG3_TYPE char*

/* Return sizeof user struct to callers in less machine dependent routines */

#define KERNEL_U_SIZE kernel_u_size()
extern int kernel_u_size PARAMS ((void));

#define ADDITIONAL_OPTIONS \
	{"kernel", no_argument, &kernel_debugging, 1}, \
	{"k", no_argument, &kernel_debugging, 1}, \
	{"wcore", no_argument, &kernel_writablecore, 1}, \
	{"w", no_argument, &kernel_writablecore, 1},

#define ADDITIONAL_OPTION_HELP \
	"\
  --kernel           Enable kernel debugging.\n\
  --wcore            Make core file writable (only works for /dev/mem).\n\
                     This option only works while debugging a kernel !!\n\
"

extern int kernel_debugging;
extern int kernel_writablecore;

#define DEFAULT_PROMPT kernel_debugging?"(kgdb) ":"(gdb) "

/* misuse START_PROGRESS to test whether we're running as kgdb */
/* START_PROGRESS is called at the top of main */
#undef START_PROGRESS
#define START_PROGRESS(STR,N) \
  if (!strcmp(STR, "kgdb")) \
     kernel_debugging = 1;

#endif /* NM_FBSD_H */
