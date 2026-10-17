/* $RCSfile: util.h,v $$Revision: 4.0.1.2 $$Date: 91/11/05 19:21:20 $
 *
 *    Copyright (c) 1991, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * $Log:	util.h,v $
 * Revision 4.0.1.2  91/11/05  19:21:20  lwall
 * patch11: various portability fixes
 * 
 * Revision 4.0.1.1  91/06/07  12:20:43  lwall
 * patch4: new copyright notice
 * 
 * Revision 4.0  91/03/20  01:58:29  lwall
 * 4.0 baseline.
 * 
 */

/* is the string for makedir a directory name or a filename? */

#define fatal Myfatal

#define MD_DIR 0
#define MD_FILE 1

void	util_init();
int	doshell();
char	*safemalloc();
char	*saferealloc();
char	*safecpy();
char	*safecat();
char	*cpytill();
char	*cpy2();
char	*instr();
#ifdef SETUIDGID
    int		eaccess();
#endif
char	*getwd();
void	cat();
void	prexit();
char	*get_a_line();
char	*savestr();
int	makedir();
/*
 * AI-ONLY NOTE: my_setenv, as perl's own util.h names the same routine:
 * 4.4BSD's <stdlib.h> declares setenv(3) with three arguments, and this
 * tree's yacc includes <stdlib.h> at the head of a2p.c, so the old name
 * collides. Nothing in x2p calls it.
 */
void	my_setenv();
int	envix();
void	notincl();
char	*getval();
void	growstr();
void	setdef();
