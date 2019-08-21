
// RhinoDevel, MT, 2018nov18

#ifndef MT_NODE
#define MT_NODE

#include <stdint.h>

#include "allocconf.h"

struct node
{
    struct node * last_node_addr;
    void* block_addr;
    MT_USIGN block_len;
    uint8_t is_allocated;
    struct node * next_node_addr;
};

#endif //MT_NODE
