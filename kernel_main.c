
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

// Peripherals physical address range start
// (these replace bus address range start 0x7E000000 in bare metal mode,
// see page 6):
//
#define PERI_BASE_PI1 0x20000000
#define PERI_BASE_PI2 0x3F000000

// Choose Raspi type:
//
//#define PERI_BASE PERI_BASE_PI1
#define PERI_BASE PERI_BASE_PI2

// GPIO registers (see page 90):
//
#define GPIO_BASE (PERI_BASE + 0x00200000)
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

static void mmio_write(uint32_t const reg, uint32_t const data)
{
	*(volatile uint32_t *)reg = data;
}

static uint32_t mmio_read(uint32_t const reg)
{
	return *(volatile uint32_t *)reg;
}

static void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

static void init_uart1()
{
    uint32_t buf;

    mmio_write(
        AUX_ENABLES, 1); // Enables Mini UART (disables SPI1 & SPI2, page 9).
    mmio_write(AUX_MU_IER_REG, 0); // See page 12 (yes, page 12!).
    mmio_write(AUX_MU_CNTL_REG, 0); // Not using extra features. See page 16.
    mmio_write(AUX_MU_LCR_REG, 3); // Sets 8 bit mode (error on page 14!).
    mmio_write(AUX_MU_MCR_REG, 0); // Sets UART1_RTS line to high (page 14).
    mmio_write(AUX_MU_IER_REG, 0); // See page 12 (yes, page 12!).
    mmio_write( // Clears receive and transmit FIFOs (page 13, yes, page 13!).
        AUX_MU_IIR_REG, 0xC6/*11000110*/);
    mmio_write( // Hard-coded for 250 MHz [((250,000,000/115200)/8)-1 = 270]!
        AUX_MU_BAUD_REG, 270);

    // Set GPIO pin 14 and 15 to alternate function 5 (page 92 and page 102),
    // which is using UART1:
    //
    buf = mmio_read(GPFSEL1);
    buf &= ~(7<<12); // GPIO pin 14.
    buf |= 2<<12; // TXD1.
    buf &= ~(7<<15); // GPIO pin 15.
    buf |= 2<<15; // RXD1.
    mmio_write(GPFSEL1, buf);

    // Enable pull-down resistors (page 101):
    //
    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0, (1<<14)|(1<<15));
    delay(150);
    //mmio_write(GPPUD, 0); // Not necessary.
    mmio_write(GPPUDCLK0, 0);

    mmio_write(AUX_MU_CNTL_REG, 3);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    // To be able to compile, although these are not used (stupid?):
    //
	(void)r0;
	(void)r1;
	(void)r2;

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

    // // WORKS:
    // //
    // // Raspi1: Toggle one LED (ACT - a green one) on/off:
    //
    // volatile uint32_t * const GPFSEL1 = (uint32_t*)0x20200004;
    // volatile uint32_t * const GPSET0 = (uint32_t*)0x2020001C;
    // volatile uint32_t * const GPCLR0 = (uint32_t*)0x20200028;
    //
    // *GPFSEL1 = (*GPFSEL1 & (~(7 << 18))) | (1 << 18);
    // while(true)
    // {
    //     *GPSET0 = 1 << 16;
    //     delay(0x1000000);
    //
    //     *GPCLR0 = 1 << 16;
    //     delay(0x500000);
    // }

    // WORKS:
    //
    // Raspi1, print to UART serial out:

    init_uart1();

    uint32_t buf = 0;

    while(true)
    {
        while((mmio_read(AUX_MU_LSR_REG) & 0x20) == 0)
        {
            ;
        }
        mmio_write(AUX_MU_IO_REG , 0x30 + (buf++));

        delay(0x1000000);

        buf = buf%10;
    }
}
