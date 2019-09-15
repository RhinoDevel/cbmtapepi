
// RhinoDevel, MT, 2018dec11

#ifndef MT_ALLOC_CONF
#define MT_ALLOC_CONF

#define MT_ALLOC_BITS_32 32
#define MT_ALLOC_BITS_64 64

#ifndef NDEBUG
    #define MT_ALLOC_DEB_CLR_1 0xAB
    #define MT_ALLOC_DEB_CLR_2 0xBC
    #define MT_ALLOC_DEB_CLR_3 0xCD
    #define MT_ALLOC_DEB_CLR_4 0xDE
    #define MT_ALLOC_DEB_CLR_5 0xEF
#endif //NDEBUG

// ***************************
// *** Set this correctly: ***
// ***************************
//
#define MT_ALLOC_BITS MT_ALLOC_BITS_32
#define MT_ALLOC_GRANULARITY 4 // Set to 1, if not wanted.
#define MT_ALLOC_ALIGN_TO_GRANULARITY // Disable, if not wanted.

#if MT_ALLOC_BITS == MT_ALLOC_BITS_32
    #define MT_USIGN uint32_t
    #define MT_USIGN_MAX 0xFFFFFFFF
#elif MT_ALLOC_BITS == MT_ALLOC_BITS_64
    #define MT_USIGN uint64_t
    #define MT_USIGN_MAX 0xFFFFFFFFFFFFFFFF
#endif //MT_ALLOC_BITS == MT_ALLOC_BITS_32

#endif //MT_ALLOC_CONF
