
// Marcel Timm, RhinoDevel, 2021oct23

#ifndef MT_MBO_ALLOC_MEM_FLAG
#define MT_MBO_ALLOC_MEM_FLAG

enum mbo_alloc_mem_flag
{
    mbo_alloc_mem_flag_direct = 4, // Alias uncached.
    mbo_alloc_mem_flag_zero = 16 // Initialise buffer with zeros.
    //
    // Add more, when necessary (see
    // https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface).
};

#endif //MT_MBO_ALLOC_MEM_FLAG
