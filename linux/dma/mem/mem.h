
// Marcel Timm, RhinoDevel, 2021oct20

#ifndef MT_MEM
#define MT_MEM

#include <sys/types.h>
#include <stddef.h>

size_t mem_get_rounded_up_to_pages(size_t const min);

void* mem_create_mapped(off_t const addr, size_t const size);
void mem_free_mapped(void * const ptr, size_t const size);

/**
 * - Given size must already be a multiple of the page size.
 */
void* mem_create_mapped_bus(uint32_t const bus_addr, uint32_t const size);

void* mem_create_mapped_gpio();
void mem_free_mapped_gpio(void * const ptr);

void* mem_create_mapped_dmac();
void mem_free_mapped_dmac(void * const ptr);

void* mem_create_mapped_pwm();
void mem_free_mapped_pwm(void * const ptr);

void* mem_create_mapped_clk();
void mem_free_mapped_clk(void * const ptr);

#endif //MT_MEM
