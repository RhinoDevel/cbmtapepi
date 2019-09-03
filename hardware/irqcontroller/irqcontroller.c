
// Marcel Timm, RhinoDevel, 2019sep02

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include "irqcontroller.h"
#include "irqcontroller_registers.h"

#include "../peribase.h"

// ARM peripherals interrupt table (see page 113):
//
#define IRQ_ARM_TIMER (1 << 0)
#define IRQ_ARM_MAILBOX (1 << 1)
#define IRQ_ARM_DOORBELL_0 (1 << 2)
#define IRQ_ARM_DOORBELL_1 (1 << 3)
#define IRQ_GPU_0_HALTED (1 << 4) // (or GPU 1 halted, see page 113)
#define IRQ_GPU_1_HALTED (1 << 5)
#define IRQ_ILLEGAL_ACCESS_TYPE_1 (1 << 6)
#define IRQ_ILLEGAL_ACCESS_TYPE_0 (1 << 7)

static uint32_t const s_irqcontroller_addr = PERI_BASE + 0xB000;
static uint32_t const s_irqcontroller_reg_addr = s_irqcontroller_addr + 0x200;
static struct irqcontroller_registers * const s_irqcontroller_reg =
    (struct irqcontroller_registers *)s_irqcontroller_reg_addr;

void __attribute__((interrupt("UNDEF"))) handler_undefined_instruction()
{
    //assert(false); // For debugging, only.
}

void __attribute__((interrupt("SWI"))) handler_swi()
{
    //assert(false); // Not used / enabled.
}

void __attribute__((interrupt("ABORT"))) handler_abort_prefetch()
{
    //assert(false); // For debugging, only.
}

void __attribute__((interrupt("ABORT"))) handler_abort_data()
{
    //assert(false); // For debugging, only.
}

void __attribute__((interrupt("ABORT"))) handler_addr_exception()
{
    //assert(false); // For debugging, only.
}

void __attribute__((interrupt("FIQ"))) handler_fiq()
{
    //assert(false); // Not used / enabled.
}

void irqcontroller_irq_src_enable_armtimer()
{
    // It is OK to use "=" sign, only, see page 117:
    //
    s_irqcontroller_reg->enable_basic_irqs = IRQ_ARM_TIMER;
}
