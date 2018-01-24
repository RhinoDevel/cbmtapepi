
// Marcel Timm, RhinoDevel, 2018jan22

#ifndef MT_BAREGPIO
#define MT_BAREGPIO

#include <stdbool.h>
#include <stdint.h>

void baregpio_write(uint32_t const pin_nr, bool const high);
void baregpio_set_output(uint32_t const pin_nr, bool const high);

#endif //MT_BAREGPIO
