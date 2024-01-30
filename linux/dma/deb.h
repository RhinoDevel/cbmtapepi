
// Marcel Timm, RhinoDevel, 2021oct20

#ifndef MT_DEB
#define MT_DEB

#include <stdio.h>

#define DEB_LOG(format, arg...) DEB_LOG_HELPER(format, ## arg)

#define DEB_LOG_HELPER(format, arg...) \
    { \
        printf( \
            "%s: " format "\n", \
            __FUNCTION__ , \
            ## arg); \
    }

#endif //MT_DEB
