
.section ".text.boot"

.globl _start

// Kernel entry point.
//
// r15: Execution should start at 0x8000.
// r0 : 0
// r1 : 0x00000C42
// r2 : 0x00000100
//
// Preserve registers above to be used as parameters of kernel_main().
_start:
	// Set stack address:
	mov sp, #0x8000

	// Set whole BSS section to zero (it is optional):
	ldr r4, =__bss_start
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
	wfe
	b halt
