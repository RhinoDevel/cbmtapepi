// Marcel Timm, RhinoDevel, 2021oct20

#include <sys/types.h>
#include <stddef.h>

#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "mem.h"
#include "../deb.h"
#include "../inf/inf.h"

// Convert bus address to a physical address:
//
#define BUS_TO_PHYS_ADDR(bus_addr) ((off_t)((uint32_t)(bus_addr) & ~0xC0000000))

size_t mem_get_rounded_up_to_pages(size_t const min)
{
    size_t const page_size = inf_get_page_size(),
        remainder = min % page_size;

    if(remainder == 0)
    {
        return min;
    }
    return min + page_size - remainder;
}

void* mem_create_mapped(off_t const addr, size_t const size)
{
    size_t const size_to_use = mem_get_rounded_up_to_pages(size);
    int fd = -1;
    void* ptr = NULL;

    fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);
    if(fd == -1)
    {
       return NULL;
    }
    ptr = mmap(NULL, size_to_use, PROT_WRITE | PROT_READ, MAP_SHARED, fd, addr);

    close(fd); // Return value ignored!
    fd = -1;

    if(ptr == MAP_FAILED)
    {
        return NULL;
    }

    DEB_LOG("0x%X mapped to 0x%X.", (unsigned int)addr, (unsigned int)ptr);

    return ptr;
}

void mem_free_mapped(void * const ptr, size_t const size)
{
    if(ptr == NULL)
    {
        DEB_LOG("Given pointer is NULL, doing nothing.");
        return;
    }

    size_t const size_to_use = mem_get_rounded_up_to_pages(size);

#ifdef NDEBUG
    munmap(ptr, size_to_use); // Return value ignored!
#else //NDEBUG
    assert(munmap(ptr, size_to_use) == 0);
    DEB_LOG("Freed %d bytes from 0x%X on.", (int)size, (unsigned int)ptr);
#endif //NDEBUG
}

void* mem_create_mapped_bus(uint32_t const bus_addr, uint32_t const size)
{
    if(size % inf_get_page_size() != 0)
    {
        DEB_LOG("Error: Given size is not a multiple of page size!");
        return NULL;
    }
    return mem_create_mapped(BUS_TO_PHYS_ADDR(bus_addr), (uint32_t)size);
}

void* mem_create_mapped_gpio()
{
    return mem_create_mapped(inf_get_gpio_addr(), inf_get_page_size());
    //
    // Assuming that (single) page size is enough for GPIO registers.
}

void mem_free_mapped_gpio(void * const ptr)
{
    mem_free_mapped(ptr, inf_get_page_size());
    //
    // Hard-coded size, see mem_create_mapped_gpio().
}

void* mem_create_mapped_dmac()
{
    return mem_create_mapped(inf_get_dmac_addr(), inf_get_page_size());
    //
    // Assuming that (single) page size is enough for DMA controller registers.
}

void mem_free_mapped_dmac(void * const ptr)
{
    mem_free_mapped(ptr, inf_get_page_size());
    //
    // Hard-coded size, see mem_create_mapped_dmac().
}

void* mem_create_mapped_pwm()
{
    return mem_create_mapped(inf_get_pwm_addr(), inf_get_page_size());
    //
    // Assuming that (single) page size is enough for PWM registers.
}

void mem_free_mapped_pwm(void * const ptr)
{
    mem_free_mapped(ptr, inf_get_page_size());
    //
    // Hard-coded size, see mem_create_mapped_pwm().
}

void* mem_create_mapped_clk()
{
    return mem_create_mapped(inf_get_clk_addr(), inf_get_page_size());
    //
    // Assuming that (single) page size is enough for clock registers.
}

void mem_free_mapped_clk(void * const ptr)
{
    mem_free_mapped(ptr, inf_get_page_size());
    //
    // Hard-coded size, see mem_create_mapped_clk().
}
