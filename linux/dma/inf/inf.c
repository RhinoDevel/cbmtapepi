// Marcel Timm, RhinoDevel, 2021oct22

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include <bcm_host.h>

#include "inf.h"
#include "../deb.h"

// (more in the header file)
//
#define INF_PERIBASE_CLK_OFFSET ((uint32_t)0x00101000) // Audio & PWM clocks.

static off_t s_peribase_addr = 0; // To be set by inf_get_peribase_addr().
static off_t s_gpio_addr = 0; // To be set by inf_get_gpio_addr().
static off_t s_dmac_addr = 0; // To be set by inf_get_dmac_addr().
static off_t s_pwm_addr = 0; // To be set by inf_get_pwm_addr().
static off_t s_clk_addr = 0; // To be set by inf_get_clk_addr().

static size_t s_page_size = 0; // To be set by get_page_size().

static off_t get_peri_addr(
    off_t const peribase_offset, char const * const title, off_t * const addr)
{
    assert(peribase_offset > 0);
    assert(title != NULL);
    assert(addr != NULL);

    if(*addr == 0)
    {
        *addr = inf_get_peribase_addr() + peribase_offset;

        DEB_LOG("%s address is 0x%X.", title, (unsigned int)(*addr));
    }
    assert(*addr > 0);
    return *addr;
}

size_t inf_get_page_size()
{
    if(s_page_size == 0)
    {
        s_page_size = (size_t)sysconf(_SC_PAGE_SIZE);

        DEB_LOG("Page size is %d bytes.", (int)s_page_size);
    }
    assert(s_page_size != 0);
    return s_page_size;
}

off_t inf_get_peribase_addr() 
{
    if(s_peribase_addr == 0)
    {
        s_peribase_addr = (off_t)bcm_host_get_peripheral_address();

        DEB_LOG(
            "Peripheral base address is 0x%X.", (unsigned int)s_peribase_addr);
    }
    assert(s_peribase_addr > 0);
    return s_peribase_addr;
}

off_t inf_get_gpio_addr()
{
    return get_peri_addr(INF_PERIBASE_GPIO_OFFSET, "GPIO", &s_gpio_addr);
}

off_t inf_get_dmac_addr()
{
    return get_peri_addr(0x00007000, "DMA controller", &s_dmac_addr);
}

off_t inf_get_pwm_addr()
{
    return get_peri_addr(
        INF_PERIBASE_PWM_OFFSET, "PWM controller", &s_pwm_addr);
}

off_t inf_get_clk_addr()
{
    return get_peri_addr(INF_PERIBASE_CLK_OFFSET, "Clock", &s_clk_addr);
}
