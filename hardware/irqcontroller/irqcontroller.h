
// Marcel Timm, RhinoDevel, 2019sep02

#ifndef MT_IRQCONTROLLER
#define MT_IRQCONTROLLER

// ARM reset handler is defined in boot.S:
//
///** ARM reset handler.
// */
//void __attribute__((interrupt("ABORT"))) handler_arm_reset();

/** Undefined instruction interrupt handler.
 */
void __attribute__((interrupt("UNDEF"))) handler_undefined_instruction();

/** Software interrupt handler.
 */
void __attribute__((interrupt("SWI"))) handler_swi();

/** Abort (pre-fetch) handler.
 */
void __attribute__((interrupt("ABORT"))) handler_abort_prefetch();

/** Abort (data) handler.
 */
void __attribute__((interrupt("ABORT"))) handler_abort_data();

/** Address exception handler.
 */
void __attribute__((interrupt("ABORT"))) handler_addr_exception();

// IRQ interrupt handler is defined in kernel_main.c:
//
///** IRQ interrupt handler.
// */
//void __attribute__((interrupt("IRQ"))) handler_irq();

/** FIQ/FIRQ interrupt handler.
 */
void __attribute__((interrupt("FIQ"))) handler_fiq();

/** Enable ARM timer as interrupt source.
 */
void irqcontroller_irq_src_enable_armtimer();

void irqcontroller_irq_enable();

#endif //MT_IRQCONTROLLER
