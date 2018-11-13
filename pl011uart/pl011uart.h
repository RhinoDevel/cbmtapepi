
// Marcel Timm, RhinoDevel, 2018nov12

#ifndef MT_PL011UART
#define MT_PL011UART

#include <stdint.h>

/** Initializes PL011 UART to be used with GPIO pins 14 (TXD) and 15 (RXD)
 *  with hard-coded communication parameters (baudrate, parity, etc.).
 */
void pl011uart_init();

uint8_t pl011uart_read_byte();
void pl011uart_write_byte(uint8_t const byte);

#endif //MT_PL011UART
