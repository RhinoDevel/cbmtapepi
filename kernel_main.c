
// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

// There are two UARTs:
//
// - UART0: (ARM) PL011 UART (see page 175).
// - UART1: Mini UART (emulates a 16550, see page 8).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "peribase.h"
#include "baregpio/baregpio.h"
#include "mem/mem.h"
#include "busywait/busywait.h"

// GPIO registers (see page 90):
//
#define GPIO_BASE (PERI_BASE + 0x200000)
#define GPFSEL1 (GPIO_BASE + 0x04) // GPIO function select 1.
#define GPSET0 (GPIO_BASE + 0x1C) // GPIO pin output set 0.
#define GPCLR0 (GPIO_BASE + 0x28) // GPIO pin output clear 0.
#define GPPUD (GPIO_BASE + 0x94) // GPIO pin pull-up/-down enable (page 100).
#define GPPUDCLK0 (GPIO_BASE + 0x98) // GPIO pin pull-up/-down enable clock 0.

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

extern uint32_t __heap; // There is not really an uint32_t object allocated.

static void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

static void init_uart1()
{
    uint32_t buf;

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
    buf = mem_read(GPFSEL1);
    buf &= ~(7<<12); // GPIO pin 14.
    buf |= 2<<12; // TXD1.
    buf &= ~(7<<15); // GPIO pin 15.
    buf |= 2<<15; // RXD1.
    mem_write(GPFSEL1, buf);

    // Enable pull-down resistors (page 101):
    //
    mem_write(GPPUD, 0);
    delay(150);
    mem_write(GPPUDCLK0, (1<<14)|(1<<15));
    delay(150);
    //mem_write(GPPUD, 0); // Not necessary.
    mem_write(GPPUDCLK0, 0);

    mem_write(AUX_MU_CNTL_REG, 3);
}

#if 0
void tape_test()
{
    // Pulse length detection triggers on descending (negative) edges.

    baregpio_set_output(2, true);

    // Short: 2840 Hz
    //
    // 1 / (2840 Hz) = 352 microseconds
    //
    // 352 microseconds / 2 = 176 microseconds
    //
    static uint32_t const s = 176;

    // Medium: 1953 Hz
    //
    // 1 / (1953 Hz) = 512 microseconds
    //
    // 512 microseconds / 2 = 256 microseconds
    //
    static uint32_t const m = 256;

    // Long: 1488 Hz
    //
    // 1 / (1488 Hz) = 672 microseconds
    //
    // 672 microseconds / 2 = 336 microseconds
    //
    //static uint32_t const l = 336;

    while(true)
    {
        // Logical 0
        //
        baregpio_write(2, false);
        busywait_microseconds(s);
        baregpio_write(2, true);
        busywait_microseconds(s);
        //
        baregpio_write(2, false);
        busywait_microseconds(m);
        baregpio_write(2, true);
        busywait_microseconds(m);

        // Logical 1
        //
        baregpio_write(2, false);
        busywait_microseconds(m);
        baregpio_write(2, true);
        busywait_microseconds(m);
        //
        baregpio_write(2, false);
        busywait_microseconds(s);
        baregpio_write(2, true);
        busywait_microseconds(s);
    }
}
#endif //0

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    uint32_t * const heap = &__heap; // This is correct!

    uint32_t * const buf = heap; // Test of heap memory usage.

    // To be able to compile, although these are not used (stupid?):
    //
	(void)r0;
	(void)r1;
	(void)r2;

    //tape_test();

    // WORKS:
    //
    // Raspi1, print to UART1 serial out and blink (internal & external LED):
    //
    baregpio_set_output(16, true); // Internal LED (green one).
    //
    init_uart1();
    //
    baregpio_set_output(2, false); // GPIO pin via expansion port.
    //
    *buf = 0;
    while(true)
    {
        // Print to UART1:
        //
        while((mem_read(AUX_MU_LSR_REG) & 0x20) == 0)
        {
            ;
        }
        mem_write(AUX_MU_IO_REG , 0x30 + ((*buf)++));
        *buf = (*buf)%10;

        // Blink:
        //
        baregpio_write(16, false);
        baregpio_write(2, true);
        busywait_milliseconds(500); // 0.5 seconds.
        baregpio_write(16, true);
        baregpio_write(2, false);
        busywait_microseconds(500000); // Also 0.5 seconds.
    }

    return;

    //// WORKS:
    ////
    //// Raspi2: Alternatively toggle two LEDs on/off:
    //
    // volatile uint32_t * const GPFSEL4 = (uint32_t *)0x3F200010;
    // volatile uint32_t * const GPFSEL3 = (uint32_t *)0x3F20000C;
    // volatile uint32_t * const GPSET1  = (uint32_t *)0x3F200020;
    // volatile uint32_t * const GPCLR1  = (uint32_t *)0x3F20002C;
    //
    // *GPFSEL4 = (*GPFSEL4 & ~(7 << 21)) | (1 << 21);
    // *GPFSEL3 = (*GPFSEL3 & ~(7 << 15)) | (1 << 15);
    //
    // while(true)
    // {
    //     *GPSET1 = 1 << (47 - 32);
    //     *GPCLR1 = 1 << (35 - 32);
    //     delay(0x100000);
    //     *GPCLR1 = 1 << (47 - 32);
    //     *GPSET1 = 1 << (35 - 32);
    //     delay(0x200000);
    // }

    // // Does NOT work: Print to UART0 serial out:
    // //
    // init_uart0();
    // //
    // *buf = 0;
    // while(true)
    // {
    //     while((mem_read(UART0_FR) & 0x20) != 0)
    //     {
    //         ;
    //     }
    //     mem_write(UART0_DR , 0x30 + ((*buf)++));
    //     delay(0x1000000);
    //
    //     *buf = (*buf)%10;
    // }
}

// // UART0 (PL011) register map (see page 177):
// //
// #define UART0_BASE (PERI_BASE + 0x201000)
// #define UART0_DR UART0_BASE
// #define UART0_RSRECR (UART0_BASE + 0x04)
// #define UART0_FR (UART0_BASE + 0x18)
// #define UART0_ILPR (UART0_BASE + 0x20)
// #define UART0_IBRD (UART0_BASE + 0x24)
// #define UART0_FBRD (UART0_BASE + 0x28)
// #define UART0_LCRH (UART0_BASE + 0x2C)
// #define UART0_CR (UART0_BASE + 0x30)
// #define UART0_IFLS (UART0_BASE + 0x34)
// #define UART0_IMSC (UART0_BASE + 0x38)
// #define UART0_RIS (UART0_BASE + 0x3C)
// #define UART0_MIS (UART0_BASE + 0x40)
// #define UART0_ICR (UART0_BASE + 0x44)
// #define UART0_DMACR (UART0_BASE + 0x48)
// #define UART0_ITCR (UART0_BASE + 0x80)
// #define UART0_ITIP (UART0_BASE + 0x84)
// #define UART0_ITOP (UART0_BASE + 0x88)
// #define UART0_TDR (UART0_BASE + 0x8C)
//
// static void uart0_flush_rx_fifo()
// {
//     while((mem_read(UART0_FR) & 0x10)==0)
//     {
//         mem_read(UART0_DR);
//     }
// }
// static void init_uart0()
// {
//     uint32_t buf;
//
//     mem_write(UART0_CR, 0);
//
//     buf = mem_read(GPFSEL1);
//     buf &= ~(7<<12);
//     buf |= 4<<12;
//     buf &= ~(7<<15);
//     buf |= 4<<15;
//     mem_write(GPFSEL1, buf);
//
//     mem_write(GPPUD, 0);
//     delay(150);
//     mem_write(GPPUDCLK0, (1<<14)|(1<<15));
//     delay(150);
//     //mem_write(GPPUD, 0); // Not necessary.
//     mem_write(GPPUDCLK0, 0);
//
//     // Hard-coded for UART0 frequency:
//     //
//     // (3000000 / (16 * 115200) = 1.627
//     // (0.627*64)+0.5 = 40
//     // int 1 frac 40
//     //
//     mem_write(UART0_ICR, 0x7FF);
//     mem_write(UART0_IBRD, 1);
//     mem_write(UART0_FBRD, 40);
//     mem_write(UART0_LCRH, 0x70);
//     mem_write(UART0_CR, 0x301);
//
//     uart0_flush_rx_fifo();
// }
