// Marcel Timm, RhinoDevel, 2021oct20

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "dma.h"
#include "dma_cb.h"
#include "pwm/pwm.h"
#include "gpio/gpio.h"
#include "deb.h"
#include "mem/mem.h"
#include "reg.h"
#include "mbo/mbo.h"
#include "mbo/mbo_alloc_mem_flag.h"
#include "inf/inf.h"

// Convert a pointer pointing into video core memory (that is mapped into
// application's virtual address space) to its (unsigned 32-bit) bus address:
//
#define VC_PTR_TO_BUS_ADDR(ptr) \
    ((uint32_t)((ptr) - s_mapped_vc_mem) + s_vc_mem_bus_addr)
    //
    // (converting from ptrdiff_t)

// DMA channel to use (warning: The offsets calculated from channel nr. are
// correct for channels 0 to 14 and not for channel 15):
//
static unsigned int const s_channel = 5; // TODO: Hard-coded! Make dynamic(?)!
//
// Page 41:
//
static off_t const s_offset_cs = (off_t)(s_channel * 0x100 + 0x00);
static off_t const s_offset_conblk_ad = (off_t)(s_channel * 0x100 + 0x04);
static off_t const s_offset_debug = (off_t)(s_channel * 0x100 + 0x20);

static off_t const s_offset_enable = 0xFF0; // Global enable bits per channel.

static void* s_mapped_dmac = NULL; // See dma_init() & dma_deinit().
 
static uint32_t s_byte_count = 0;      // See dma_init() & dma_deinit().
static uint32_t s_vc_mem_addr = 0;     //
static uint32_t s_vc_mem_bus_addr = 0; //
static void* s_mapped_vc_mem = NULL;   // 

static void reset()
{
    assert(s_mapped_dmac != NULL);
    assert(s_channel < 15); // See comment at definition.

    volatile uint32_t * const cs_reg = REG_PTR(s_mapped_dmac, s_offset_cs);

    *cs_reg = 1 << 31; // Reset
}

static void enable()
{
    assert(s_mapped_dmac != NULL);

    volatile uint32_t * const enable_reg =
        REG_PTR(s_mapped_dmac, s_offset_enable);

    *enable_reg = *enable_reg | (uint32_t)(1 << s_channel); // Enable channel.
    reset();
}

uint32_t dma_get_bus_addr_from_vc_ptr(void * const ptr)
{
    return VC_PTR_TO_BUS_ADDR(ptr);   
}

void dma_pause()
{
    assert(s_mapped_dmac != NULL);
    assert(s_channel < 15); // See comment at definition.

    volatile uint32_t * const cs_reg = REG_PTR(s_mapped_dmac, s_offset_cs);

    // Page 47 - 50:
    //
    // 1098 7654   3210 9876   5432 1098   7654 3210
    // 0011 0000 | 1111 1111 | 0000 0000 | 0000 0000
    //                                             ^
    //                                             |
    //                                             ACTIVE = 0 => Pause!
    //
    *cs_reg = 0x30FF0000 & *cs_reg;
}

void dma_resume()
{
    assert(s_mapped_dmac != NULL);
    assert(s_channel < 15); // See comment at definition.

    volatile uint32_t * const cs_reg = REG_PTR(s_mapped_dmac, s_offset_cs);

    // Page 47 - 50:
    //
    // 1098 7654   3210 9876   5432 1098   7654 3210
    // 0000 0000 | 0000 0000 | 0000 0000 | 0000 0001
    //                                             ^
    //                                             |
    //                                             ACTIVE = 1 => Resume!
    //
    *cs_reg = 0x00000001 | *cs_reg;
}

void dma_start(int const cb_offset)
{
    volatile uint32_t * const cs_reg = REG_PTR(s_mapped_dmac, s_offset_cs);
    volatile uint32_t * const conblk_ad_reg = REG_PTR(s_mapped_dmac, s_offset_conblk_ad);
    volatile uint32_t * const debug_reg = REG_PTR(s_mapped_dmac, s_offset_debug);

    enable();

    pwm_start();

    *conblk_ad_reg = VC_PTR_TO_BUS_ADDR(
        s_mapped_vc_mem + cb_offset * sizeof (struct dma_cb));
    *cs_reg = 2; // Clear end flag.
    *debug_reg = 7; // Clear error bits.
    *cs_reg = 1; // Start
}

void dma_stop()
{
    pwm_stop();
    // TODO: Disable channel [see enable()]!
    reset();
}

bool dma_is_busy()
{
    volatile uint32_t * const cs_reg = REG_PTR(s_mapped_dmac, s_offset_cs);

    return (*cs_reg & 1) != 0; // ACTIVE flag.
}

void dma_deinit()
{
    assert(s_mapped_dmac != NULL);

    reset();
    
    if(s_mapped_vc_mem != NULL)
    {
        assert(s_byte_count != 0);
        mem_free_mapped(s_mapped_vc_mem, s_byte_count);
    }
    s_byte_count = 0;

    mbo_unlock_mem(s_vc_mem_addr); // Return value ignored (OK, if addr. is 0).
    s_vc_mem_bus_addr = 0;

    mbo_free_mem(s_vc_mem_addr); // Return value ignored (OK, if addr. is 0).
    s_vc_mem_addr = 0;

    pwm_deinit();

    mem_free_mapped_dmac(s_mapped_dmac);
    s_mapped_dmac = NULL;
}

struct dma_cb * dma_init(
    uint32_t const pwm_freq,
    uint32_t const pwm_range,
    uint32_t const byte_count)
{
    assert(byte_count > 0);
    assert(s_mapped_dmac == NULL);

    // Get access to DMA controller via mapping it into virtual address space:
    //
    s_mapped_dmac = mem_create_mapped_dmac();
    if(s_mapped_dmac == NULL)
    {
        return NULL;
    }

    if(!pwm_init(pwm_freq, pwm_range))
    {
        dma_deinit();
        return NULL;
    }

    // The memory buffer size must be a multiple of the system's page size:
    //
    s_byte_count = (uint32_t)mem_get_rounded_up_to_pages((size_t)byte_count);

    // Get uncached memory buffer of wanted size (filled with zeros):
    //
    s_vc_mem_addr = mbo_alloc_mem(
        s_byte_count,
        (uint32_t)inf_get_page_size(),
        mbo_alloc_mem_flag_direct | mbo_alloc_mem_flag_zero);
    if(s_vc_mem_addr == 0)
    {
        dma_deinit();
        return NULL;
    }
    DEB_LOG(
        "Allocated %d bytes of video core memory at 0x%X.",
        (int)s_byte_count,
        (unsigned int)s_vc_mem_addr);

    // The buffer must be locked before use, also retrieve bus address of the
    // allocated buffer:
    //
    s_vc_mem_bus_addr = mbo_lock_mem(s_vc_mem_addr);
    if(s_vc_mem_bus_addr == 0)
    {
        dma_deinit();
        return NULL;
    }
    DEB_LOG(
        "Locked video core memory, bus address is 0x%X.",
        (unsigned int)s_vc_mem_bus_addr);

    // Map the buffer into virtual address space:
    //
    s_mapped_vc_mem = mem_create_mapped_bus(s_vc_mem_bus_addr, s_byte_count);
    if(s_mapped_vc_mem == NULL)
    {
        dma_deinit();
        return NULL;
    }
    return (struct dma_cb *)s_mapped_vc_mem;
} 
