
// Marcel Timm, RhinoDevel, 2018oct25

#ifndef MT_CONSOLE
#define MT_CONSOLE

#include "console_params.h"

void console_init(struct console_params const * const p);

char console_read_char();

/** Read characters into buffer, until '\r' was read
 *  or buffer is full (indicated by buf_len).
 *
 *  Also writes each character to console immediately
 *  after being read.
 *
 *  Anyway, at the end there will be a '\0' character.
 */
void console_read(char * const buf, int const buf_len);

void console_write_key(char const key);

void console_write_byte(uint8_t const byte);
void console_write_word(uint16_t const word);
void console_write_dword(uint32_t const dword);

/* Write characters from buffer, until '\0' was read.
 * '\0' will not be written.
 */
void console_write(char const * const buf);

void console_writeline(char const * const buf);

#endif //MT_CONSOLE
