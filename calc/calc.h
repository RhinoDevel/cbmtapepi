
// Marcel Timm, RhinoDevel, 2018oct26

#ifndef MT_CALC
#define MT_CALC

#include <stdint.h>

/** Split given byte into high and low parts and fill characters at
 *  addresses given with ASCII representation of the corresponding parts.
 *
 *  E.g.: byte == 254 => out_high = 'f', out_low = 'e'
 */
void calc_byte_to_hex(
    uint8_t const byte, char * const out_high, char * const out_low);

/**
 *  E.g.: word == 43981 => out_four[0...3] = 'a', 'b', 'c', 'd'
 */
void calc_word_to_hex(uint16_t const word, char * const out_four);

#endif //MT_CALC
