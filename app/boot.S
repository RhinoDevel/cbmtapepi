
// Marcel Timm, RhinoDevel, 2019aug22

// See: https://sourceware.org/binutils/docs-2.29/as/index.html

.section ".text.boot"

.globl _start

// Kernel entry point.
//
// r15: Execution should start at 0x8000.
// r0 : 0 = Boot device code.
// r1 : 0x00000C42 = ARM Linux Machine Type,
//                   see: http://www.arm.linux.org.uk/developer/machines/
// r2 : 0x00000100 = ATAGs beginning,
//                   see: http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html#appendix_tag_reference
//
// Preserve registers above to be used as parameters of kernel_main().
_start:
	// Set stack address (grows downwards to address 0):
	//mov sp, #0x00008000 // Above 32kB address.
    ldr sp, =0x04000000 // Above 64MB address.

	// Set whole BSS section to zero (it is optional, but C standard):
	ldr r4, =__bss_start // See: https://community.arm.com/processors/b/blog/posts/how-to-load-constants-in-assembly-for-arm-architecture
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b       2f

1:
	stmia r4!, {r5-r8}

2:
	cmp r4, r9
	blo 1b

	// Call kernel_main():
	ldr r3, =kernel_main
	blx r3

	// Halt:
halt:
	wfe // Wait for event (hint).
	b halt