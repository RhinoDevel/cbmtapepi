
// Marcel Timm, RhinoDevel, 2021oct22

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_INF
#define MT_INF

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

// (more in the C file)
//
#define INF_PERIBASE_BUS_ADDR ((uint32_t)0x7E000000) // Page 5.
#define INF_PERIBASE_GPIO_OFFSET ((uint32_t)0x00200000) // Page 90.
#define INF_PERIBASE_PWM_OFFSET ((uint32_t)0x0020C000) // Page 141 (see errata).

size_t inf_get_page_size();

off_t inf_get_peribase_addr();

off_t inf_get_gpio_addr();

off_t inf_get_dmac_addr();

off_t inf_get_pwm_addr();

off_t inf_get_clk_addr();

#endif //MT_INF
