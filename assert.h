
// RhinoDevel, MT, 2018dec11

#ifndef MT_ASSERT
#define MT_ASSERT

#ifdef NDEBUG
    #define MT_ASSERT_ACTIVE 0
#else
    #define MT_ASSERT_ACTIVE 1
#endif //NDEBUG

#include <stdbool.h>

// Original source code: http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
//
#define assert(result) \
    do \
    { \
        if(MT_ASSERT_ACTIVE) \
        { \
            /* Currently doing nothing, even in debug mode!! */ \
        } \
    }while(0);

#endif // MT_ASSERT
