
// Marcel Timm, RhinoDevel, 2019sep02

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_IRQCONTROLLER_REGISTERS
#define MT_IRQCONTROLLER_REGISTERS

#include <stdint.h>

// See page 112.
//
struct irqcontroller_registers
{
    volatile uint32_t irq_basic_pending; // 0x200
    volatile uint32_t irq_pending_1; // 0x204
    volatile uint32_t irq_pending_2; // 0x208
    volatile uint32_t fiq_control; // 0x20C
    volatile uint32_t enable_irqs_1; // 0x210
    volatile uint32_t enable_irqs_2; // 0x214
    volatile uint32_t enable_basic_irqs; // 0x218
    volatile uint32_t disable_irqs_1; // 0x21C
    volatile uint32_t disable_irqs_2; // 0x220
    volatile uint32_t disable_basic_irqs; // 0x224
};

#endif //MT_IRQCONTROLLER_REGISTERS
