/*
 * SPDX-FileCopyrightText: 2026 Nicholas Martin
 *
 * Multiboot entry for the shim.  Loaded by QEMU or GRUB at 1 MB,
 * where multiboot requires it; the kernel it carries runs at
 * physical 0, which no multiboot loader will place.  See shim.c.
 */
	.set	MAGIC, 0x1BADB002
	.set	FLAGS, 0x00000001		/* page-align modules */
	.section .multiboot
	.align	4
	.long	MAGIC
	.long	FLAGS
	.long	-(MAGIC+FLAGS)

	.section .text
	.globl	_start
_start:
	movl	$stack_top, %esp
	pushl	%ebx				/* multiboot info */
	pushl	%eax				/* multiboot magic */
	call	shim_main
1:	hlt
	jmp	1b

/*
 * jump_kernel(entry, howto, opendev, cyloffset)
 * Build the stack boot.c builds -- (howto, opendev, 0, cyloffset)
 * above a return address -- and enter the kernel.
 */
	.globl	jump_kernel
jump_kernel:
	movl	4(%esp), %eax			/* entry */
	movl	8(%esp), %ecx			/* howto */
	movl	12(%esp), %edx			/* opendev */
	movl	16(%esp), %ebx			/* cyloffset */
	movl	$0x80000, %esp			/* a stack out of the way */
	pushl	%ebx
	pushl	$0
	pushl	%edx
	pushl	%ecx
	pushl	$0				/* return address slot */
	jmp	*%eax

	.section .bss
	.align	16
	.skip	16384
stack_top:
