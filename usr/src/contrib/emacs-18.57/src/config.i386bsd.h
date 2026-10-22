/* GNU Emacs site configuration template file.
   Copyright (C) 1988 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */



/* Include here a s- file that describes the system type you are using.
   See the file ../etc/MACHINES for a list of systems and
   the names of the s- files to use for them.
   See s-template.h for documentation on writing s- files.  */
#include "s-bsd4-4.h"

/* Include here a m- file that describes the machine and system you use.
   See the file ../etc/MACHINES for a list of machines and
   the names of the m- files to use for them.
   See m-template.h for info on what m- files should define.
   */
#include "m-intel386.h"

/*
 * AI-ONLY NOTE: the i386's configuration, made from Berkeley's two in
 * the way they differ from each other -- config.sparcbsd.h is
 * config.hp300bsd.h but for its m- file -- since ./makefile links
 * config.${MACHINE}bsd.h as config.h and Berkeley wrote none for the
 * i386. m-intel386.h is emacs's own m- file for the 80386, whose BSD
 * case GNU's 18.59 uses for 386BSD. What the i386 here needs beyond
 * it, each as emacs's own files have it for another system:
 *
 *   LIBS_TERMCAP	the system's termcap, as m-sparcbsd.h has it
 *			("Must use the system's termcap") and 18.59's
 *			s-386bsd.h for 386BSD.
 *   UNEXEC		unexelf.o: this system's programs are ELF, which
 *			unexec.c, for a.out and COFF, cannot dump.
 *   USG_SHARED_LIBRARIES  for unexelf.c's bss_end, which emacs.c
 *			defines only under it; s-usg5-4.h, the ELF
 *			system unexelf.c was written for, has it.
 *   DATA_SEG_BITS	0x08000000, as s-usg5-4.h has it: an i386 ELF
 *			program is loaded at 0x08048000, above the
 *			24 bits of address a Lisp object holds, and
 *			these are put back into every pointer.
 *   C_SWITCH_ASM	for crt0.c alone, the ymakefile's switch for
 *			it: crt0.c's i386 start finds argc at a fixed
 *			distance from the frame _start pushes, and GCC
 *			2.95.3 aligns the stack to 16 bytes there unless
 *			-mpreferred-stack-boundary=2, its old 4.
 *
 * And no X: HAVE_X_WINDOWS and X11 are left undefined, as
 * config.h-dist leaves them, since the system has no X11, where
 * Berkeley's hp300 and sparc were built with X11R5.
 */
#define LIBS_TERMCAP -ltermcap
#define UNEXEC unexelf.o
#define USG_SHARED_LIBRARIES
#define DATA_SEG_BITS 0x08000000
#define C_SWITCH_ASM -mpreferred-stack-boundary=2

/* Load in the conversion definitions if this system
   needs them and the source file being compiled has not
   said to inhibit this.  There should be no need for you
   to alter these lines.  */

#ifdef SHORTNAMES
#ifndef NO_SHORTNAMES
#include "../shortnames/remap.h"
#endif /* not NO_SHORTNAMES */
#endif /* SHORTNAMES */

/* Define HAVE_X_WINDOWS if you want to use the X window system.  */

/* #define HAVE_X_WINDOWS */

/* Define X11 if you want to use version 11 of X windows.
   Otherwise, Emacs expects to use version 10.  */

/* #define X11 */

/* Define HAVE_X_MENU if you want to use the X window menu system.
   This appears to work on some machines that support X
   and not on others.  */

/* #define HAVE_X_MENU */

/* Define `subprocesses' should be defined if you want to
   have code for asynchronous subprocesses
   (as used in M-x compile and M-x shell).
   These do not work for some USG systems yet;
   for the ones where they work, the s-*.h file defines this flag.  */

#ifndef VMS
#ifndef USG
#define subprocesses
#endif
#endif

/* Define USER_FULL_NAME to return a string
   that is the user's full name.
   It can assume that the variable `pw'
   points to the password file entry for this user.

   At some sites, the pw_gecos field contains
   the user's full name.  If neither this nor any other
   field contains the right thing, use pw_name,
   giving the user's login name, since that is better than nothing.  */

#define USER_FULL_NAME pw->pw_gecos

/* Define AMPERSAND_FULL_NAME if you use the convention
   that & in the full name stands for the login id.  */

/* #define AMPERSAND_FULL_NAME */

/* # bytes of pure Lisp code to leave space for.
   Note that s-vms.h and m-sun2.h may override this default.  */

#ifndef PURESIZE
#ifdef HAVE_X_WINDOWS
#define PURESIZE 122000
#else
#define PURESIZE 119000
#endif
#endif

/* Define HIGHPRI as a negative number
   if you want Emacs to run at a higher than normal priority.
   For this to take effect, you must install Emacs with setuid root.
   Emacs will change back to the users's own uid after setting
   its priority.  */

/* #define HIGHPRI */
