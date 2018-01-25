
// Marcel Timm, RhinoDevel, 2018jan25

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

// There are two UARTs:
//
// - UART0: (ARM) PL011 UART (see page 175).
// - UART1: Mini UART (used here; emulates a 16550, see page 8).

#include <stdint.h>

#include "miniuart.h"
#include "../auxiliary.h"
#include "../mem/mem.h"
#include "../baregpio/baregpio.h"

uint8_t miniuart_read_byte()
{
    while((mem_read(AUX_MU_LSR_REG) & 0x01)==0)
    {
        ;
    }
    return mem_read(AUX_MU_IO_REG);
}

void miniuart_write_byte(uint8_t byte)
{
    // Print to UART1:
    //
    while((mem_read(AUX_MU_LSR_REG) & 0x20) == 0)
    {
        ;
    }

    mem_write(AUX_MU_IO_REG , byte);
}

void miniuart_init()
{
    mem_write(
        AUX_ENABLES, 1); // Enables Mini UART (disables SPI1 & SPI2, page 9).
    mem_write(AUX_MU_IER_REG, 0); // See page 12 (yes, page 12!).
    mem_write(AUX_MU_CNTL_REG, 0); // Not using extra features. See page 16.
    mem_write(AUX_MU_LCR_REG, 3); // Sets 8 bit mode (error on page 14!).
    mem_write(AUX_MU_MCR_REG, 0); // Sets UART1_RTS line to high (page 14).
    mem_write(AUX_MU_IER_REG, 0); // See page 12 (yes, page 12!).
    mem_write( // Clears receive and transmit FIFOs (page 13, yes, page 13!).
        AUX_MU_IIR_REG, 0xC6/*11000110*/);
    mem_write( // Hard-coded for 250 MHz [((250,000,000/115200)/8)-1 = 270]!
        AUX_MU_BAUD_REG, 270);

    // Set GPIO pin 14 and 15 to alternate function 5 (page 92 and page 102),
    // which is using UART1:
    //
    baregpio_set_func(14, gpio_func_alt5); // TXD1
    baregpio_set_func(15, gpio_func_alt5); // RXD1

    // Enable pull-down resistors (page 101):
    //
    baregpio_set_pud(14, gpio_pud_down);
    baregpio_set_pud(15, gpio_pud_down);

    mem_write(AUX_MU_CNTL_REG, 3);
}
