
// Marcel Timm, RhinoDevel, 2021oct25

// Pages mentioned in source code comments can be found in the document
// "BCM2835 ARM Peripherals".
//
// Also you MUST USE https://elinux.org/BCM2835_datasheet_errata
// together with that document!

#ifndef MT_DMA_CB
#define MT_DMA_CB

#include <stdint.h>

// DMA control block (see page 40).
//
struct __attribute__ ((aligned(32))) dma_cb
{
    uint32_t transfer_info; // TI
    uint32_t src_addr; // SOURCE_AD
    uint32_t dest_addr; // DEST_AD
    uint32_t transfer_len; // TXFR_LEN
    uint32_t stride; // STRIDE (2D mode stride).
    uint32_t next_cb_addr; // NEXTCONBK (next control block address).
    uint32_t reserved0; // Set to zero (debug?).
    uint32_t reserved1; // Set to zero.
};
//
// "__attribute__", etc. seems to be GCC-specific..

#endif //MT_DMA_CB
