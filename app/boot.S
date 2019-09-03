
// Marcel Timm, RhinoDevel, 2019aug22

// See: https://sourceware.org/binutils/docs-2.29/as/index.html

.section ".text.boot"

.globl _start
.globl _enable_interrupts

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
//
_start:
    // Setup interrupt vector table at start (should be at 0x00008000):
    //
    ldr pc, _vector_arm_reset
    ldr pc, _vector_undefined_instruction
    ldr pc, _vector_swi
    ldr pc, _vector_abort_prefetch
    ldr pc, _vector_abort_data
    ldr pc, _vector_addr_exception
    ldr pc, _vector_irq
    ldr pc, _vector_fiq

    // Constants for interrupt vector table:
    //
_vector_arm_reset:
    .word _handler_arm_reset
_vector_undefined_instruction:
    .word handler_undefined_instruction
_vector_swi:
    .word handler_swi
_vector_abort_prefetch:
    .word handler_abort_prefetch
_vector_abort_data:
    .word handler_abort_data
_vector_addr_exception:
    .word handler_addr_exception
_vector_irq:
    .word handler_irq
_vector_fiq:
    .word handler_fiq

    // "Fall through" to reset handler.

_handler_arm_reset:
    // Copy interrupt vector table and constants into position:

    ldr r3, =__start
    mov r4, #0

    // Vector table:
    //
    ldmia r3!, {r5, r6, r7, r8, r9, r10, r11, r12}
    stmia r4!, {r5, r6, r7, r8, r9, r10, r11, r12}

    // Constants:
    //
    ldmia r3!, {r5, r6, r7, r8, r9, r10, r11, r12}
    stmia r4!, {r5, r6, r7, r8, r9, r10, r11, r12}

	// Set stack address (grows downwards to address 0):
    //
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
    //
	ldr r3, =kernel_main
	blx r3

	// Halt:
    //
halt:
	wfe // Wait for event (hint).
	b halt

_enable_interrupts:
    mrs r3, cpsr // Copy Current Program Status Register to r3.
    bic r3, r3, #0x80 // Clear bit 7 (will enable "normal" IRQs, later).
    msr cpsr_c, r3 // Update Control byte ("_c") of CPSR, only (to enable IRQs).
    mov pc, lr // Use Link Register to return from this function.
