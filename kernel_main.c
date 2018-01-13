
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Raspi1:
//
#define GPFSEL1 0x20200004
#define GPSET0 0x2020001C
#define GPCLR0 0x20200028
#define GPPUD 0x20200094
#define GPPUDCLK0 0x20200098
#define AUX_ENABLES 0x20215004
#define AUX_MU_IO_REG 0x20215040
#define AUX_MU_IER_REG 0x20215044
#define AUX_MU_IIR_REG 0x20215048
#define AUX_MU_LCR_REG 0x2021504C
#define AUX_MU_MCR_REG 0x20215050
#define AUX_MU_LSR_REG 0x20215054
#define AUX_MU_MSR_REG 0x20215058
#define AUX_MU_SCRATCH 0x2021505C
#define AUX_MU_CNTL_REG 0x20215060
#define AUX_MU_STAT_REG 0x20215064
#define AUX_MU_BAUD_REG 0x20215068
//
//GPIO14 - TXD0 and TXD1
//GPIO15 - RXD0 and RXD1
// Alt. function 5 for uart1
// Alt. function 0 for uart0

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
    // Raspi1: Print to UART serial out:

    uint32_t buf;

    mmio_write(AUX_ENABLES, 1);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3);
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6);
    mmio_write(AUX_MU_BAUD_REG, 270); //((250,000,000/115200)/8)-1 = 270

    buf = mmio_read(GPFSEL1);
    buf &= ~(7<<12); // gpio14
    buf |= 2<<12; // alt5
    mmio_write(GPFSEL1, buf);

    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0,(1<<14));
    delay(150);
    mmio_write(GPPUDCLK0, 0);

    mmio_write(AUX_MU_CNTL_REG, 2);

    buf = 0;
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
