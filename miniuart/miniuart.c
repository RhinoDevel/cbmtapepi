
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
#include <stdbool.h>

#include "miniuart.h"
#include "../hardware/auxiliary.h"
#include "../mem/mem.h"
#include "../hardware/armtimer/armtimer.h"
#include "../baregpio/baregpio.h"

bool miniuart_is_ready_to_read()
{
    return (mem_read(AUX_MU_LSR_REG) & 0x01) != 0;
}

uint8_t miniuart_read_byte()
{
    while(!miniuart_is_ready_to_read())
    {
        ;
    }
    return (uint8_t)mem_read(AUX_MU_IO_REG);
}

void miniuart_flush()
{
    while(miniuart_is_ready_to_read())
    {
        miniuart_read_byte();
    }
}

void miniuart_write(uint32_t const val)
{
    // Print to UART1:
    //
    while((mem_read(AUX_MU_LSR_REG) & 0x20) == 0)
    {
        ;
    }

    mem_write(AUX_MU_IO_REG, val);
}

void miniuart_write_byte(uint8_t const byte)
{
    miniuart_write((uint32_t)byte);
}

void miniuart_init()
{
    // Enable pull-down resistors (page 101):
    //
    baregpio_set_pud(14, gpio_pud_down);
    baregpio_set_pud(15, gpio_pud_down);

    // Set GPIO pin 14 and 15 to alternate function 5 (page 92 and page 102),
    // which is using UART1:
    //
    baregpio_set_func(14, gpio_func_alt5); // TXD1
    baregpio_set_func(15, gpio_func_alt5); // RXD1

    // Enable Mini UART (disables SPI1 & SPI2, page 9):
    //
    mem_write(AUX_ENABLES, 1/*...001*/);

    // Disable receive and transmit interrupts (see page 12 - yes, page 12!):
    //
    mem_write(AUX_MU_IER_REG, 0);

    // Disable auto flow control (see page 16):
    //
    mem_write(AUX_MU_CNTL_REG, 0);

    // Set 8 bit mode (instead of 7 bit mode; error on page 14!):
    //
    mem_write(AUX_MU_LCR_REG, 3/*...00XXXX11*/);

    mem_write(AUX_MU_MCR_REG, 0); // Sets UART1_RTS line to high (page 14).

    // AGAIN (maybe not necessary to do this twice?):
    //
    // Disable receive and transmit interrupts (see page 12 - yes, page 12!):
    //
    mem_write(AUX_MU_IER_REG, 0);

    // Clear receive and transmit FIFOs (page 13, yes, page 13!):
    //
    mem_write(AUX_MU_IIR_REG, 0xC6/*...11000110*/);

    // Hard-coded for 250 MHz [((250,000,000/115200)/8)-1 = 270]:
    //
    mem_write(AUX_MU_BAUD_REG, 270);

    // Enables receiver and transmitter (see page 17):
    //
    mem_write(AUX_MU_CNTL_REG, 3/*...00000011*/);

    armtimer_busywait_microseconds(1000);

    miniuart_flush();

    // Notes:
    // ------
    //
    // Flow control via extra GPIO PINs and cables (not used, here):
    // - RTS = Request to send.
    // - CTS = Clear to send.
    //
    // There is also auto-flow control (disable, here).
}
