/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * shim -- load this tree's kernel from a multiboot loader.
 *
 * This is scratch tooling and is meant to be deleted.  It exists
 * because of one fact about 4.4BSD-Lite2's i386 port: the kernel runs
 * at physical 0.  locore.s reaches its variables as `symbol-SYSTEM'
 * with SYSTEM 0xFE000000 before paging is on, and the tree's own
 * bootstrap masks the a.out entry with & 0x000fffff to turn
 * 0xFE000000 into 0.  Multiboot will not place an image below 1 MB --
 * measured: QEMU answers `invalid load_addr address' even with the
 * a.out kludge fields set -- so no multiboot loader can start this
 * kernel directly.
 *
 * What replaces this: every descendant moved the i386 kernel to
 * physical 1 MB, and all of them before 4.4BSD-Lite2 shipped.
 * NetBSD 1.0 has KERNTEXTOFF 0xf8100000, OpenBSD 1996 has
 * 0xf0100000, and FreeBSD 2.0.5 links at LOAD_ADDRESS F0100000.
 * Lite2 has no KERNTEXTOFF at all and links at KERNBASE itself.
 * Moving this tree to 1 MB is therefore the genetic answer with
 * three contemporary donors, and once it is made the kernel can
 * carry a multiboot header in locore.s the way NetBSD's does, and
 * this program goes away.
 *
 * It is done in that order, and not the other way round, because
 * moving KERNBASE touches the paging setup, the pmap's recursive
 * page directory and the fault handler's kernel-address test, in a
 * kernel that has never executed an instruction.  That change wants
 * a baseline to compare against, and this is how one is obtained.
 *
 * The kernel is passed as a multiboot module.  Segments are copied
 * to vaddr - KERNBASE; the first LOAD segment of our image sits at
 * KERNBASE - 0x1000 and holds the ELF headers rather than any
 * section, an artefact of linking with -Ttext and no script, so
 * anything below KERNBASE is skipped.
 */

#define KERNBASE	0xF0000000

typedef unsigned long u32;
typedef unsigned short u16;

struct mbinfo { u32 flags, memlo, memhi, bootdev, cmdline, modcount, modaddr; };
struct module { u32 start, end, string, reserved; };

struct ehdr {
	unsigned char e_ident[16];
	u16 e_type, e_machine; u32 e_version, e_entry, e_phoff, e_shoff, e_flags;
	u16 e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx;
};
struct phdr { u32 p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags, p_align; };

static void outb(unsigned p, unsigned char v)
	{ __asm__ __volatile__("outb %0,%w1" :: "a"(v), "d"(p)); }
static unsigned char inb(unsigned p)
	{ unsigned char v; __asm__ __volatile__("inb %w1,%0" : "=a"(v) : "d"(p)); return v; }
static void putc_(char c)
	{ if (c == '\n') putc_('\r'); while (!(inb(0x3fd) & 0x20)) ; outb(0x3f8, c); }
static void puts_(const char *s) { while (*s) putc_(*s++); }
static void puthex(u32 n)
	{ int i; puts_("0x"); for (i = 28; i >= 0; i -= 4) putc_("0123456789abcdef"[(n>>i)&0xf]); }

static void *mymove(void *d, const void *s, u32 n)
	{ char *a = d; const char *b = s; while (n--) *a++ = *b++; return d; }
static void myzero(void *d, u32 n) { char *a = d; while (n--) *a++ = 0; }

extern void jump_kernel(u32 entry, u32 howto, u32 opendev, u32 cyloffset);

void
shim_main(u32 magic, struct mbinfo *mb)
{
	struct module *mod;
	struct ehdr *eh;
	struct phdr *ph;
	u32 base, i, entry;

	outb(0x3f9,0); outb(0x3fb,0x80); outb(0x3f8,1); outb(0x3f9,0);
	outb(0x3fb,3); outb(0x3fc,3);

	puts_("shim: 4.4BSD-Lite2 i386 kernel loader (scratch tooling)\n");
	if (magic != 0x2BADB002) {
		puts_("shim: not started by a multiboot loader\n"); return;
	}
	if (!(mb->flags & (1<<3)) || mb->modcount < 1) {
		puts_("shim: no kernel module; pass it with -initrd\n"); return;
	}

	mod = (struct module *)mb->modaddr;
	base = mod->start;
	eh = (struct ehdr *)base;
	if (eh->e_ident[0] != 0x7f || eh->e_ident[1] != 'E') {
		puts_("shim: module is not ELF\n"); return;
	}
	puts_("shim: kernel module at "); puthex(base);
	puts_(", entry "); puthex(eh->e_entry); puts_("\n");

	ph = (struct phdr *)(base + eh->e_phoff);
	for (i = 0; i < eh->e_phnum; i++, ph++) {
		if (ph->p_type != 1)			/* PT_LOAD */
			continue;
		if (ph->p_vaddr < KERNBASE) {		/* the header page */
			puts_("  skip   "); puthex(ph->p_vaddr); puts_(" (below KERNBASE)\n");
			continue;
		}
		puts_("  load   "); puthex(ph->p_vaddr);
		puts_(" -> "); puthex(ph->p_vaddr - KERNBASE);
		puts_(" filesz "); puthex(ph->p_filesz);
		puts_(" memsz "); puthex(ph->p_memsz); puts_("\n");
		mymove((void *)(ph->p_vaddr - KERNBASE),
		    (void *)(base + ph->p_offset), ph->p_filesz);
		if (ph->p_memsz > ph->p_filesz)
			myzero((void *)(ph->p_vaddr - KERNBASE + ph->p_filesz),
			    ph->p_memsz - ph->p_filesz);
	}

	entry = eh->e_entry - KERNBASE;
	puts_("shim: entering kernel at "); puthex(entry); puts_("\n\n");
	jump_kernel(entry, 0, 0, 0);
	puts_("shim: kernel returned\n");
}
