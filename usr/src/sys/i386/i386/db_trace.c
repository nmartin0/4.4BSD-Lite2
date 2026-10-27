/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 *
 *	$Id: db_trace.c,v 1.7.2.1 1994/10/11 10:00:46 mycroft Exp $
 */

#include <sys/param.h>
#include <sys/proc.h>

#include <machine/db_machdep.h>

#include <ddb/db_access.h>
#include <ddb/db_sym.h>
#include <ddb/db_variables.h>

/*
 * Machine register set.
 */
struct db_variable db_regs[] = {
	"es",	  &ddb_regs.tf_es,     FCN_NULL,
	"ds",	  &ddb_regs.tf_ds,     FCN_NULL,
	"edi",	  &ddb_regs.tf_edi,    FCN_NULL,
	"esi",	  &ddb_regs.tf_esi,    FCN_NULL,
	"ebp",	  &ddb_regs.tf_ebp,    FCN_NULL,
	"ebx",	  &ddb_regs.tf_ebx,    FCN_NULL,
	"edx",	  &ddb_regs.tf_edx,    FCN_NULL,
	"ecx",	  &ddb_regs.tf_ecx,    FCN_NULL,
	"eax",	  &ddb_regs.tf_eax,    FCN_NULL,
	"eip",	  &ddb_regs.tf_eip,    FCN_NULL,
	"cs",	  &ddb_regs.tf_cs,     FCN_NULL,
	"eflags", &ddb_regs.tf_eflags, FCN_NULL,
	"esp",	  &ddb_regs.tf_esp,    FCN_NULL,
	"ss",	  &ddb_regs.tf_ss,     FCN_NULL,
};
struct db_variable *db_eregs = db_regs + sizeof(db_regs)/sizeof(db_regs[0]);

/*
 * Stack trace.
 */
#define	INKERNEL(va)	(((vm_offset_t)(va)) >= USRSTACK)

struct i386_frame {
	struct i386_frame	*f_frame;
	int			f_retaddr;
	int			f_arg0;
};

#define	NONE		0
#define	TRAP		1
#define	SYSCALL		2
#define	INTERRUPT	3

db_addr_t	db_trap_symbol_value = 0;
db_addr_t	db_syscall_symbol_value = 0;
db_addr_t	db_kdintr_symbol_value = 0;
boolean_t	db_trace_symbols_found = FALSE;

void
db_find_trace_symbols()
{
	db_expr_t	value;

	if (db_value_of_name("_trap", &value))
		db_trap_symbol_value = (db_addr_t) value;
	if (db_value_of_name("_kdintr", &value))
		db_kdintr_symbol_value = (db_addr_t) value;
	if (db_value_of_name("_syscall", &value))
		db_syscall_symbol_value = (db_addr_t) value;
	db_trace_symbols_found = TRUE;
}

/*
 * Figure out how many arguments were passed into the frame at "fp".
 */
int
db_numargs(fp)
	struct i386_frame *fp;
{
	int	*argp;
	int	inst;
	int	args;
	extern char	etext[];

	argp = (int *)db_get_value((int)&fp->f_retaddr, 4, FALSE);
	if (argp < (int *)VM_MIN_KERNEL_ADDRESS || argp > (int *)etext) {
		args = 5;
	} else {
		inst = db_get_value((int)argp, 4, FALSE);
		if ((inst & 0xff) == 0x59)	/* popl %ecx */
			args = 1;
		else if ((inst & 0xffff) == 0xc483)	/* addl %n, %esp */
			args = ((inst >> 16) & 0xff) / 4;
		else
			args = 5;
	}
	return (args);
}

/* 
 * Figure out the next frame up in the call stack.  
 * For trap(), we print the address of the faulting instruction and 
 *   proceed with the calling frame.  We return the ip that faulted.
 *   If the trap was caused by jumping through a bogus pointer, then
 *   the next line in the backtrace will list some random function as 
 *   being called.  It should get the argument list correct, though.  
 *   It might be possible to dig out from the next frame up the name
 *   of the function that faulted, but that could get hairy.
 */
void
db_nextframe(fp, ip, argp, is_trap)
	struct i386_frame **fp;		/* in/out */
	db_addr_t	*ip;		/* out */
	int *argp;			/* in */
	int is_trap;			/* in */
{

	switch (is_trap) {
	    case NONE:
		*ip = (db_addr_t)
			db_get_value((int) &(*fp)->f_retaddr, 4, FALSE);
		*fp = (struct i386_frame *)
			db_get_value((int) &(*fp)->f_frame, 4, FALSE);
		break;

	    default: {
		struct trapframe *tf;

		/* The only argument to trap() or syscall() is the trapframe. */
		tf = (struct trapframe *)argp;
		switch (is_trap) {
		case TRAP:
			db_printf("--- trap (number %d) ---\n", tf->tf_trapno);
			break;
		case SYSCALL:
			db_printf("--- syscall (number %d) ---\n", tf->tf_eax);
			break;
		case INTERRUPT:
			db_printf("--- interrupt ---\n");
			break;
		}
		*fp = (struct i386_frame *)tf->tf_ebp;
		*ip = (db_addr_t)tf->tf_eip;
		break;
	    }
	}
}

void
db_stack_trace_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char		*modif;
{
	struct i386_frame *frame, *lastframe;
	struct i386_frame *actframe;
	boolean_t	first;
	int		*argp;
	db_addr_t	callpc;
	int		is_trap;
	boolean_t	kernel_only = TRUE;
	boolean_t	trace_thread = FALSE;

#if 0
	if (!db_trace_symbols_found)
		db_find_trace_symbols();
#endif

	{
		register char *cp = modif;
		register char c;

		while ((c = *cp++) != 0) {
			if (c == 't')
				trace_thread = TRUE;
			if (c == 'u')
				kernel_only = FALSE;
		}
	}

	if (count == -1)
		count = 65535;

	if (!have_addr) {
		frame = (struct i386_frame *)ddb_regs.tf_ebp;
		if (frame == 0)
			frame = (struct i386_frame *)(ddb_regs.tf_esp - 4);
		callpc = (db_addr_t)ddb_regs.tf_eip;
	} else if (trace_thread) {
		/*
		 * AI-ONLY NOTE: changed (2026-10-26) -- return after the
		 * message. NetBSD 1.0, OpenBSD and FreeBSD 2.0 and 2.1 print
		 * it and go on to trace from a frame pointer that was never
		 * set. Here every process runs its kernel stack at one
		 * address in its own address space, so a sleeping process's
		 * saved frame pointer cannot be read from ddb, which runs on
		 * the current one: roadmap-4.6.md, "ddb: tracing another
		 * process or thread". NetBSD 1.3 (sommerfe, March 1997) is
		 * the first to trace by pid; it gives each process's kernel
		 * stack an address of its own.
		 */
		printf ("db_trace.c: can't trace thread\n");
		return;
	} else {
		frame = (struct i386_frame *)addr;
		callpc = (db_addr_t)
			 db_get_value((int)&frame->f_retaddr, 4, FALSE);
	}

	lastframe = 0;
	first = TRUE;
	while (count && frame != 0) {
		int		narg;
		char *	name;
		db_expr_t	offset;
		db_sym_t	sym;
#define MAXNARG	16
		char	*argnames[MAXNARG], **argnp = NULL;

		sym = db_search_symbol(callpc, DB_STGY_ANY, &offset);
		db_symbol_values(sym, &name, NULL);

		/*
		 * AI-ONLY NOTE: changed (2026-10-26) -- the first frame
		 * found as FreeBSD's db_trace.c finds it (bde, 27 March
		 * 1996): stopped at a function's first instruction, at its
		 * second, the movl %esp,%ebp, or at its ret, the caller's
		 * pc is at the top of the stack, not in %ebp's frame; and a
		 * null %ebp, as in the idle loop, begins from %esp. NetBSD
		 * 1.0 knew only the first case, and only without a symbol,
		 * lost the caller at the other two, and printed nothing for
		 * a null %ebp.
		 *
		 * Attempt to determine a (possibly fake) frame that gives
		 * the caller's pc.  It may differ from `frame' if the
		 * current function never sets up a standard frame or hasn't
		 * set one up yet or has just discarded one.  The last two
		 * cases can be guessed fairly reliably for code generated
		 * by gcc.
		 */
		actframe = frame;
		if (first && !have_addr) {
			int	instr;

			instr = db_get_value(callpc, 4, FALSE);
			if ((instr & 0x00ffffff) == 0x00e58955) {
				/* pushl %ebp; movl %esp, %ebp */
				actframe = (struct i386_frame *)
					   (ddb_regs.tf_esp - 4);
			} else if ((instr & 0x0000ffff) == 0x0000e589) {
				/* movl %esp, %ebp */
				actframe = (struct i386_frame *)
					   ddb_regs.tf_esp;
				if (ddb_regs.tf_ebp == 0) {
					/* Fake the caller's frame better. */
					frame = actframe;
				}
			} else if ((instr & 0x000000ff) == 0x000000c3) {
				/* ret */
				actframe = (struct i386_frame *)
					   (ddb_regs.tf_esp - 4);
			} else if (offset == 0) {
				/* Probably a symbol in assembler code. */
				actframe = (struct i386_frame *)
					   (ddb_regs.tf_esp - 4);
			}
		}
		first = FALSE;
		if (INKERNEL((int)frame) && name) {
			/*
			 * AI-ONLY NOTE: added (2026-10-26) -- the names as an
			 * ELF kernel's symbols spell them, without the leading
			 * underscore a.out gave every C name: thorpej's "Make
			 * traces work with ELF kernels" (NetBSD 8f31d96853, 7
			 * June 1999, db_trace.c 1.25), as it is. GCC here
			 * defines __ELF__.
			 */
#ifdef __ELF__
			if (!strcmp(name, "trap")) {
				is_trap = TRAP;
			} else if (!strcmp(name, "syscall")) {
				is_trap = SYSCALL;
			} else if (name[0] == 'X') {
				if (!strncmp(name, "Xintr", 5) ||
				    !strncmp(name, "Xresume", 7) ||
				    !strncmp(name, "Xstray", 6) ||
				    !strncmp(name, "Xhold", 5) ||
				    !strncmp(name, "Xrecurse", 8) ||
				    !strcmp(name, "Xdoreti") ||
				    !strncmp(name, "Xsoft", 5)) {
					is_trap = INTERRUPT;
				} else
					goto normal;
			} else
				goto normal;
			narg = 0;
#else
			if (!strcmp(name, "_trap")) {
				is_trap = TRAP;
			} else if (!strcmp(name, "_syscall")) {
				is_trap = SYSCALL;
			} else if (name[0] == '_' && name[1] == 'X') {
				if (!strncmp(name, "_Xintr", 6) ||
				    !strncmp(name, "_Xresume", 8) ||
				    !strncmp(name, "_Xstray", 7) ||
				    !strncmp(name, "_Xhold", 6) ||
				    !strncmp(name, "_Xrecurse", 9) ||
				    !strcmp(name, "_Xdoreti") ||
				    !strncmp(name, "_Xsoft", 6)) {
					is_trap = INTERRUPT;
				} else
					goto normal;
			} else
				goto normal;
			narg = 0;
#endif /* __ELF__ */
		} else {
		normal:
			is_trap = NONE;
			narg = MAXNARG;
			if (db_sym_numargs(sym, &narg, argnames))
				argnp = argnames;
			else
				narg = db_numargs(frame);
		}

		db_printf("%s(", name);

		argp = &actframe->f_arg0;

		while (narg) {
			if (argnp)
				db_printf("%s=", *argnp++);
			db_printf("%x", db_get_value((int)argp, 4, FALSE));
			argp++;
			if (--narg != 0)
				db_printf(",");
		}
		db_printf(") at ");
		db_printsym(callpc, DB_STGY_PROC);
		db_printf("\n");

		if (actframe != frame) {
			/* `frame' belongs to caller. */
			lastframe = actframe;
			callpc = (db_addr_t)
			    db_get_value((int)&actframe->f_retaddr, 4, FALSE);
			continue;
		}

		lastframe = frame;
		db_nextframe(&frame, &callpc, &frame->f_arg0, is_trap);

		if (frame == 0) {
			/* end of chain */
			break;
		}
		if (INKERNEL((int)frame)) {
			/* staying in kernel */
			if (frame <= lastframe) {
				db_printf("Bad frame pointer: 0x%x\n", frame);
				break;
			}
		} else if (INKERNEL((int)lastframe)) {
			/* switch from user to kernel */
			if (kernel_only)
				break;	/* kernel stack only */
		} else {
			/* in user */
			if (frame <= lastframe) {
				db_printf("Bad user frame pointer: 0x%x\n",
					  frame);
				break;
			}
		}
		--count;
	}

	if (count && is_trap != NONE) {
		db_printsym(callpc, DB_STGY_XTRN);
		db_printf(":\n");
	}
}
