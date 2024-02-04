
// MT, 2016mar30

#ifndef MT_PROGRESSBAR
#define MT_PROGRESSBAR

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ProgressBar_print(int const inFirst, int const inCur, int const inLast, int const inWidth, bool const inShowAbs);

#ifdef __cplusplus
}
#endif

#endif // MT_PROGRESSBAR

