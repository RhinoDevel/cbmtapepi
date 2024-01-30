// Marcel Timm, RhinoDevel, 2021oct31

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#include "pwm.h"
#include "../reg.h"
#include "../deb.h"
#include "../mem/mem.h"
#include "../clk/clk.h"

// Page 141 (more in the header file):
//
#define PWM_OFFSET_CTL ((uint32_t)0x00) // Control
#define PWM_OFFSET_STA ((uint32_t)0x04) // Status
#define PWM_OFFSET_DMAC ((uint32_t)0x08) // DMA configuration.
#define PWM_OFFSET_RNG1 ((uint32_t)0x10) // Channel 1 range.

static void* s_mapped_pwm = NULL; // See pwm_init() & pwm_deinit().

static uint32_t get_ctl()
{
    assert(s_mapped_pwm != NULL);

    return *REG_PTR(s_mapped_pwm, PWM_OFFSET_CTL);
}
static uint32_t get_sta()
{
    assert(s_mapped_pwm != NULL);

    return *REG_PTR(s_mapped_pwm, PWM_OFFSET_STA);
}
static uint32_t get_dmac()
{
    assert(s_mapped_pwm != NULL);

    return *REG_PTR(s_mapped_pwm, PWM_OFFSET_DMAC);
}

static void set_ctl(uint32_t const val)
{
    *REG_PTR(s_mapped_pwm, PWM_OFFSET_CTL) = val;
}
static void set_sta(uint32_t const val)
{
    *REG_PTR(s_mapped_pwm, PWM_OFFSET_STA) = val;
}
static void set_rng1(uint32_t const val)
{
    *REG_PTR(s_mapped_pwm, PWM_OFFSET_RNG1) = val;
}
static void set_fif1(uint32_t const val)
{
    *REG_PTR(s_mapped_pwm, PWM_OFFSET_FIF1) = val;
}
static void set_dmac(uint32_t const val)
{
    *REG_PTR(s_mapped_pwm, PWM_OFFSET_DMAC) = val;
}

static void reset_ctl()
{
    set_ctl(0); // See page 143.
}
static void reset_all_sta_flags()
{
    set_sta(0x1FFF & get_sta()); // Write bits 13 - 31 as 0 (see page 144).
}

#ifndef NDEBUG

static void pwm_deb_log_registers()
{
    if (s_mapped_pwm == NULL)
    {
        DEB_LOG("Error: PWM is not mapped to memory, yet!");
        return;
    }

    DEB_LOG("CTL: 0x%08X", get_ctl());
    DEB_LOG("STA: 0x%08X", get_sta());
}

#endif //NDEBUG

/**
 * - Delays by 400 us.
 */
void pwm_stop()
{
    if (s_mapped_pwm == NULL)
    {
        return; // Nothing to do.
    }

    reset_ctl();
    usleep(200); // Even using sleep(10) instead does not prevent STA.BERR = 1.
    reset_all_sta_flags(); // In case (e.g.) BERR is set..
    usleep(200);
}

void pwm_deinit()
{
    pwm_stop();

    if(s_mapped_pwm == NULL)
    {
        return; // Nothing to do.
    }

    mem_free_mapped_pwm(s_mapped_pwm);
    s_mapped_pwm = NULL;

    clk_deinit();
}

void pwm_start()
{
    assert(s_mapped_pwm != NULL);

    set_ctl( // It is OK to set all other bits to 0 [also see pwm_stop()].
        (1 << 5) // USEF1 - Enable channel 1 FIFO usage (page 143).
            | 1); // PWEN1 - Enable channel 1 (page 143).
}

bool pwm_init(uint32_t const clk_freq_hz, uint32_t const range)
{
    assert(s_mapped_pwm == NULL);

    uint32_t buf;

    // Get access to PWM controller via mapping it into virtual address space:
    //
    s_mapped_pwm = mem_create_mapped_pwm();
    if(s_mapped_pwm == NULL)
    {
        return false;
    }

    pwm_stop();

    pwm_deb_log_registers();

    if(!clk_init(clk_freq_hz))
    {
        pwm_deinit();
        return false;
    }
    
    set_rng1(range);
    set_fif1(range >> 1);

    // Enable PWM with data threshold 1 and DMA:
    //
    buf = get_dmac();
    //
    // ENAB, bit 31 (see below).
    // | Write bits 16 - 30 as 0.
    // | |               Keep DMA threshold for PANIC signal (bits 8 - 15).
    // | |               |        DREQ, bits 0 - 7 (see below).
    // | |               |        |
    // 1 098765432109876 54321098 76543210
    // 0 000000000000000 11111111 00000000
    //
    buf &= 0xFF00;
    //
    buf |= 1 << 31; // ENAB - Enable DMA (page 145).
    // [just keep DMA threshold for PANIC signal (bits 8 - 15)]
    buf |= 1; // DMA threshold for DREQ (page 145).
    //
    set_dmac(buf);

    return true;
}
