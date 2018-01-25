
// Marcel Timm, RhinoDevel, 2018jan25

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_AUXILIARY
#define MT_AUXILIARY

#include "peribase.h"

// Auxiliary peripherals register map (see page 8):
//
#define AUX_BASE (PERI_BASE + 0x215000)
#define AUX_IRQ AUX_BASE // Auxiliary interrupt status (AUXIRQ).
#define AUX_ENABLES (AUX_BASE + 0x04) // Auxiliary enables (AUXENB).
#define AUX_MU_IO_REG (AUX_BASE + 0x40) // Mini UART I/O data.
#define AUX_MU_IER_REG (AUX_BASE + 0x44) // Mini UART interrupt enable.
#define AUX_MU_IIR_REG (AUX_BASE + 0x48) // Mini UART interrupt identify.
#define AUX_MU_LCR_REG (AUX_BASE + 0x4C) // Mini UART line control.
#define AUX_MU_MCR_REG (AUX_BASE + 0x50) // Mini UART modem control.
#define AUX_MU_LSR_REG (AUX_BASE + 0x54) // Mini UART line status.
#define AUX_MU_MSR_REG (AUX_BASE + 0x58) // Mini UART modem status.
#define AUX_MU_SCRATCH (AUX_BASE + 0x5C) // Mini UART scratch.
#define AUX_MU_CNTL_REG (AUX_BASE + 0x60) // Mini UART extra control.
#define AUX_MU_STAT_REG (AUX_BASE + 0x64) // Mini UART extra status.
#define AUX_MU_BAUD_REG (AUX_BASE + 0x68) // Mini UART Baud rate.
//
// The Mini UART emulates 16550 behaviour, but is NOT a 16650 compatible UART.
//
// In addition to the Mini UART there also exist two SPI masters
// as auxiliary peripherals.

#endif //MT_AUXILIARY
