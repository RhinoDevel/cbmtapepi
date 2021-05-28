
// Marcel Timm, RhinoDevel, 2018feb03

#ifndef MT_TAPE_SAMPLE
#define MT_TAPE_SAMPLE

#include <stdbool.h>

#include "tape_sample_type.h"

#ifdef __cplusplus
extern "C" {
#endif

bool tape_sample_send(enum tape_sample_type const t);

#ifdef __cplusplus
}
#endif

#endif //MT_TAPE_SAMPLE
