
// Marcel Timm, RhinoDevel, 2018nov12

#ifndef MT_PL011UART
#define MT_PL011UART

#include <stdint.h>

/** Initializes PL011 UART to be used with GPIO pins 14 (TXD) and 15 (RXD)
 *  with hard-coded communication parameters (baudrate, parity, etc.).
 */
void pl011uart_init();

bool pl011uart_is_ready_to_read();
uint8_t pl011uart_read_byte();
void pl011uart_write(uint32_t const val);
void pl011uart_write_byte(uint8_t const byte);

#endif //MT_PL011UART

// // Read and show PL011 UART (UART0) Baud rate divisor:
// //
// // (x / (16 * 115200) = i.z
// // (0.z * 64) + 0.5 = f
// // int i frac f
// //
// console_write("IBRD: i = "); // Raspi1: 0x1A = 26
// console_write_word(
//     (uint16_t)(
//         0x0000FFFF // Bits 0-15.
//         & mem_read(/*PL011_IBRD*/ PERI_BASE + 0x201000 + 0x24)));
// console_writeline("");
// console_write("FBRD: f = "); // Raspi1: 0x03 = 3
// console_write_byte(
//     (uint8_t)(
//         0x0000003F // Bits 0-5
//         & mem_read(/*PL011_FBRD*/ PERI_BASE + 0x201000 + 0x28)));
// console_writeline("");
