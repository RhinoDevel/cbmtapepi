
// Marcel Timm, RhinoDevel, 2018jan25

#ifndef MT_MINIUART
#define MT_MINIUART

#include <stdint.h>

/** Initializes MiniUART to be used with GPIO pins 14 (TXD) and 15 (RXD)
 *  with hard-coded communication parameters (baudrate, parity, etc.).
 */
void miniuart_init();

uint8_t miniuart_read_byte();
void miniuart_write_byte(uint8_t byte);

#endif //MT_MINIUART
