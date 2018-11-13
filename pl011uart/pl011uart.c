
// Marcel Timm, RhinoDevel, 2018nov12

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

// There are two UARTs:
//
// - UART0: (ARM) PL011 UART (used here; see page 175).
// - UART1: Mini UART (emulates a 16550, see page 8).

#include <stdint.h>

#include "pl011uart.h"
#include "../peribase.h"
#include "../busywait/busywait.h"
#include "../mem/mem.h"
#include "../baregpio/baregpio.h"

// UART0 (PL011) register map (see page 177):
//
#define PL011_BASE (PERI_BASE + 0x201000)
#define PL011_DR PL011_BASE
#define PL011_RSRECR (PL011_BASE + 0x04)
#define PL011_FR (PL011_BASE + 0x18) // Flag register.
#define PL011_ILPR (PL011_BASE + 0x20) // IrDA register (is disabled).
#define PL011_IBRD (PL011_BASE + 0x24) // Holds integer part of Baud rate divisor (IBRD field = Bits 0 to 15).
#define PL011_FBRD (PL011_BASE + 0x28)
#define PL011_LCRH (PL011_BASE + 0x2C) // Line control register (page 184).
#define PL011_CR (PL011_BASE + 0x30) // Control register.
#define PL011_IFLS (PL011_BASE + 0x34)
#define PL011_IMSC (PL011_BASE + 0x38)
#define PL011_RIS (PL011_BASE + 0x3C)
#define PL011_MIS (PL011_BASE + 0x40)
#define PL011_ICR (PL011_BASE + 0x44)
#define PL011_DMACR (PL011_BASE + 0x48)
#define PL011_ITCR (PL011_BASE + 0x80)
#define PL011_ITIP (PL011_BASE + 0x84)
#define PL011_ITOP (PL011_BASE + 0x88)
#define PL011_TDR (PL011_BASE + 0x8C)

static void flush_rx_fifo()
{
    // Read, until receive FIFO is empty (page 182):
    //
    while((mem_read(PL011_FR) & (1 << 4)) == 0) // Checks RXFE.
    {
        // RXFE flag is 0 <=> Receive FIFO is not empty.

        mem_read(PL011_DR);
    }
}

uint8_t pl011uart_read_byte()
{
    // Wait for receive FIFO being full (page 182):
    //
    while((mem_read(PL011_FR) & (1 << 4)) != 0) // Checks RXFE.
    {
        ; // RXFE flag is 1 <=> Receive FIFO is empty.
    }
    return mem_read(PL011_DR);
}

void pl011uart_write_byte(uint8_t const byte)
{
    // Wait for the transmit FIFO being empty (page 182):
    //
    while((mem_read(PL011_FR) & (1 << 5)) != 0) // Checks TXFF.
    {
        ; // TXFF flag is 1 <=> Transceive FIFO is full.
    }
    mem_write(PL011_DR, byte);
}

// TODO: Fix bug(-s):
//
void pl011uart_init()
{
    mem_write(PL011_CR, 0); // Disables everything (UARTEN = bit 0, page 187).

    busywait_clockcycles(150); // Maybe not necessary.

    // Set GPIO pin 14 and 15 to alternate function 0 (page 92 and page 102),
    // which is using UART0:
    //
    baregpio_set_func(14, gpio_func_alt0); // TXD0
    baregpio_set_func(15, gpio_func_alt0); // RXD0

    // Enable pull-down resistors (page 101):
    //
    baregpio_set_pud(14, gpio_pud_down);
    baregpio_set_pud(15, gpio_pud_down);

    mem_write(PL011_ICR, 0x7FF); // Clears pending interrupts.

    // Does NOT work on Raspi1:
    //
    // Hard-coded for UART0 default reference clock frequency of 3 MHz:
    //
    // (3000000 / (16 * 115200) = 1.627
    // (0.627*64)+0.5 = 40
    // int 1 frac 40
    //
    // mem_write(PL011_IBRD, 1);
    // mem_write(PL011_FBRD, 40); // Fractional Baud rate divisor (bits 0-5).

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity):
    //
    mem_write(PL011_LCRH,
        (1 << 4) // FEN (enable FIFOs, page 184).
        | (1 << 5) | (1 << 6)); // WLEN = "b11" = 8 bits (page 184).

    // Mask all interrupts:
    //
    mem_write(
        PL011_IMSC,
        (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6)
        | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // (Re-)enable UART, receive & transfer:
    //
    mem_write(
        PL011_CR,
        (1 << 0) // UARTEN (page 187).
        | (1 << 8) // TXE (page 186).
        | (1 << 9)); // RXE (page 186).

    flush_rx_fifo(); // Maybe not necessary.
}
