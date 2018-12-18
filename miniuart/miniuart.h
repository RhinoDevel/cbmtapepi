
// Marcel Timm, RhinoDevel, 2018jan25

#ifndef MT_MINIUART
#define MT_MINIUART

#include <stdint.h>
#include <stdbool.h>

/** Initializes MiniUART to be used with GPIO pins 14 (TXD) and 15 (RXD)
 *  with hard-coded communication parameters (baudrate, parity, etc.).
 */
void miniuart_init();

bool miniuart_is_ready_to_read();
uint8_t miniuart_read_byte();
void miniuart_write(uint32_t const val);
void miniuart_write_byte(uint8_t const byte);

#endif //MT_MINIUART
