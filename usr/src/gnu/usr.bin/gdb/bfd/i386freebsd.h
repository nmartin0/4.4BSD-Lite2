/* Intel 386 running any FreeBSD Unix */

#include <machine/param.h>
#include <machine/vmparam.h>

/* AI-ONLY NOTE: FreeBSD 2.2.8's `#define NBPG PAGE_SIZE' is out (2026-10-27):
   this system's <machine/param.h>, which Lite2 wrote, defines NBPG as 4096,
   and its PAGE_SIZE is cnt.v_page_size, a variable in the kernel's
   address space. HOST_PAGE_SIZE below then is NBPG, as GNU's own
   bfd/hosts/i386bsd.h and gdb/config/i386/xm-i386bsd.h have it.  */
#define	HOST_PAGE_SIZE		NBPG
#define	HOST_MACHINE_ARCH	bfd_arch_i386
#define	HOST_TEXT_START_ADDR		USRTEXT

/* Jolitz suggested defining HOST_STACK_END_ADDR to
   (u.u_kproc.kp_eproc.e_vm.vm_maxsaddr + MAXSSIZ), which should work on
   both BSDI and 386BSD, but that is believed not to work for BSD 4.4.  */

/* This seems to be the right thing for FreeBSD and BSDI */
#define	HOST_STACK_END_ADDR	USRSTACK

/* BSDI defines this too (PST) */         
#define HOST_DATA_START_ADDR	((bfd_vma)u.u_kproc.kp_eproc.e_vm.vm_daddr)

#define TRAD_UNIX_CORE_FILE_FAILING_SIGNAL(core_bfd) \
  ((core_bfd)->tdata.trad_core_data->u.u_sig)
#define u_comm u_kproc.kp_proc.p_comm
