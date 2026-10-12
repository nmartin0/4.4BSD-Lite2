ENTRY(_start)
SECTIONS {
	. = 8M;	/* clear of the kernel, which now runs at 1M */
	.text ALIGN(4K) : { *(.multiboot) *(.text) }
	.rodata ALIGN(4K) : { *(.rodata) }
	.data ALIGN(4K) : { *(.data) }
	.bss ALIGN(4K) : { *(COMMON) *(.bss) }
}
