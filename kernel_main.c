
// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "auxiliary.h"
#include "baregpio/baregpio.h"
#include "mem/mem.h"
#include "busywait/busywait.h"
#include "miniuart/miniuart.h"
#include "pl011uart/pl011uart.h"

#include "tape/tape.h"
#include "tape/tape_sample.h"

#include "console/console.h"

extern uint32_t __heap;

bool tape_test(
    uint32_t const gpio_pin_nr_read,
    uint32_t const gpio_pin_nr_sense,
    uint32_t const gpio_pin_nr_motor,
    uint32_t * const mem)
{
    console_writeline("Setting tape read line to HIGH..");
    baregpio_set_output(gpio_pin_nr_read, true); // Tape read.

    // Use start of given memory for sample input structure:

    struct tape_input * const sample = (struct tape_input *)mem;

    // Use follow-up memory for sample data to send:

    uint8_t * const buf = (uint8_t *)mem + sizeof *sample;

    // Get sample input structure (content):

    console_writeline("Filling input structure with sample data..");
    tape_sample_fill_buf(sample);

    // Get sample data to send:

    console_writeline("Filling send buffer from input structure..");
    tape_fill_buf(sample, buf);

    // Send sample data via GPIO pin with given nr:

    console_writeline("Setting sense line to LOW..");
    baregpio_set_output(gpio_pin_nr_sense, false); // Sense

    console_writeline("Sending buffer content..");
    if(tape_transfer_buf(buf, gpio_pin_nr_motor, gpio_pin_nr_read))
    {
        console_writeline(
            "Success. Setting tape read line and sense line to HIGH..");
        baregpio_set_output(gpio_pin_nr_read, true); // Tape read.
        baregpio_set_output(gpio_pin_nr_sense, true); // Sense
        return true;
    }
    console_writeline(
        "Failure! Setting tape read line and sense line to HIGH..");
    baregpio_set_output(gpio_pin_nr_read, true); // Tape read.
    baregpio_set_output(gpio_pin_nr_sense, true); // Sense
    return false;
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t r2)
{
    uint32_t * const heap = &__heap; // Gets heap address as pointer.

    uint32_t * const buf = heap; // Test of heap memory usage.

    // To be able to compile, although these are not used (stupid?):
    //
	(void)r0;
	(void)r1;
	(void)r2;

/*
    // Initialize console via MiniUART:
    //
    {
        struct console_params p;

        p.read_byte = miniuart_read_byte;
        p.write_byte = miniuart_write_byte;

        miniuart_init();
        console_init(&p);
    }
*/
    // Initialize console via PL011 UART:
    //
    {
        struct console_params p;

        p.read_byte = pl011uart_read_byte;
        p.write_byte = pl011uart_write_byte;

        pl011uart_init();
        console_init(&p);
    }

    // while(true)
    // {
    //         console_write_key(0x30 + 0);
    //         baregpio_set_output(16, false); // Raspi1: Internal LED (green one).
    //         busywait_milliseconds(500);
    //
    //         console_write_key(0x30 + 1);
    //         baregpio_set_output(16, true); // Raspi1: Internal LED (green one).
    //         busywait_milliseconds(500);
    // }

    {
        console_writeline("Setting sense line to HIGH..");
        baregpio_set_output(3, true); // Sense

        console_writeline("Setting motor line to input without pull..");
        baregpio_set_input_pull_off(4);

        while(true)
        {
            for(int i = 0;i < 10;++i)
            {
                console_write_key(0x30 + 10 - i - 1);
                console_writeline("");

                baregpio_set_output(16, false); // Raspi1: Internal LED (green one).
                busywait_milliseconds(500);

                baregpio_set_output(16, true); // Raspi1: Internal LED (green one).
                busywait_milliseconds(500);

                console_writeline(baregpio_read(4) ? "Motor: ON" : "Motor: OFF");
            }
            console_writeline("GO!");
            tape_test(2, 3, 4, buf); // Return value ignored.
        }
    }
    return;

    // // WORKS:
    // //
    // // Raspi1, print to UART1 serial out and blink (internal & external LED):
    // //
    // baregpio_set_output(16, true); // Internal LED (green one).
    // //
    // miniuart_init();
    // //
    // baregpio_set_output(2, false); // GPIO pin via expansion port.
    // //
    // *buf = 0;
    // while(true)
    // {
    //     // Print to UART1:
    //     //
    //     miniuart_write_byte(0x30 + ((*buf)++));
    //
    //     *buf = (*buf)%10;
    //
    //     // Blink:
    //     //
    //     baregpio_write(16, false);
    //     baregpio_write(2, true);
    //     busywait_milliseconds(500); // 0.5 seconds.
    //     baregpio_write(16, true);
    //     baregpio_write(2, false);
    //     busywait_microseconds(500000); // Also 0.5 seconds.
    //
    //     if(*buf==0)
    //     {
    //         miniuart_write_byte(miniuart_read_byte());
    //     }
    // }
    // return;

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
    //     busywait_clockcycles(0x100000);
    //     *GPCLR1 = 1 << (47 - 32);
    //     *GPSET1 = 1 << (35 - 32);
    //     busywait_clockcycles(0x200000);
    // }
}
