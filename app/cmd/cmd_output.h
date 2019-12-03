
// Marcel Timm, RhinoDevel, 2019dec03

#ifndef MT_CMD_OUTPUT
#define MT_CMD_OUTPUT

#include <stdint.h>

struct cmd_output
{
    char* name;
    uint8_t* bytes;
    uint32_t count;
};

#endif //MT_CMD_OUTPUT
