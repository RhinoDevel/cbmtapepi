
// Marcel Timm, RhinoDevel, 2021mar29

#ifndef MT_BARRIER
#define MT_BARRIER

#include "peribase.h"

#if PERI_BASE == PERI_BASE_PI1

    //"mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0)
    #define barrier_datasync() \
        asm volatile ( \
            "mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory" \
        ) 
    
/*
    //"mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0)
    #define barrier_datamem() \
        asm volatile ( \
            "mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory" \
        )
*/

#else //PERI_BASE == PERI_BASE_PI2AND3

    // DSB / Data synchronisation barrier acts as special kind of memory
    // barrier, where NO instructions appearing "in source code" after it
    // execute until DSB completes.
    // DSB completes, when all explicit memory accesses before it completed
    // and all cache, branch predictor and TLB maintenance operatons before DSB
    // completed.
    // 
    #define barrier_datasync() \
        asm volatile ( \
            "dsb" ::: "memory" \
        )

/*
    // DMB / Data memory barrier ensures that all explicit memory accesses that
    // appear "in source code" before it are observed before any explicit memory
    // accesses "in source code" after it. It does not affect the ordering of
    // any other instructions executing on the processor.
    //
    #define barrier_datamem() \
        asm volatile ( \
            "dmb" ::: "memory" \
        )
*/

#endif //PERI_BASE == PERI_BASE_PI1

#endif //MT_BARRIER
