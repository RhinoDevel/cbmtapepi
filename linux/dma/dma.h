
// Marcel Timm, RhinoDevel, 2021oct20

#ifndef MT_DMA
#define MT_DMA

#include <stdbool.h>
#include <stdint.h>

#include "dma_cb.h"

// This is NOT a universal interface to the DMA controller, but to be used to
// send signals to a GPIO pin via DMA transfers.

// Convert a given (unsigned 32-bit) GPIO offset (e.g. GPIO_OFFSET_SET0) into
// its (unsigned 32-bit) bus address:
//
#define DMA_GPIO_OFFSET_TO_BUS_ADDR(gpio_offset) \
    (INF_PERIBASE_BUS_ADDR + INF_PERIBASE_GPIO_OFFSET + (gpio_offset))

// Convert a given (unsigned 32-bit) PWM offset (e.g. PWM_OFFSET_FIF1) into
// its (unsigned 32-bit) bus address:
//
#define DMA_PWM_OFFSET_TO_BUS_ADDR(pwm_offset) \
    (INF_PERIBASE_BUS_ADDR + INF_PERIBASE_PWM_OFFSET + (pwm_offset))

// How to use:
// ===========
//
// 1) Initialize:
// --------------
// Initialize stuff (clock used, PWM controller, etc.) and allocate the amount
// of memory necessary to hold all control blocks that will be used later:
//
// dma_init(32 * 1024 * 1024);
//
// Check the return value! Fill the memory at return value with control blocks,
// if necessary.
//
// 2) Prepare control blocks:
// --------------------------
// Prepare control blocks in memory at pointer being the return value of
// dma_init().
//
// 3) Start DMA transfer(-s):
// --------------------------
// dma_start();
//
// 4) Wait for DMA transfer to be done or enter infinite loop:
// -----------------------------------------------------------
// E.g.: sleep(60) or use dma_is_busy() to check.
//
// You have to make sure to wait long enough, depending on the use case.
//
// 5) Make sure that DMA transfer is stopped and deinitialize:
// -----------------------------------------------------------
// dma_stop();
// dma_deinit();

uint32_t dma_get_bus_addr_from_vc_ptr(void * const ptr);

void dma_start(int const cb_offset);
void dma_stop();

bool dma_is_busy();

void dma_deinit();

/**
 * - Returns pointer to allocated memory to be filled with DMA control blocks.
 *   Returns NULL on error. Otherwise guarantees that the given count of bytes
 *   is available at the returned pointer.
 * 
 * - Does NOT give ownership to caller [meaning: Deallocation happens in
 *   dma_deinit()]!
 */
struct dma_cb * dma_init(
    uint32_t const pwm_freq,
    uint32_t const pwm_range,
    uint32_t const byte_count);

#endif //MT_DMA
