
// RhinoDevel, MT, 2018nov19

#include <stdint.h>
#include <stdbool.h>
#include "../assert.h"

#include "node.h"
#include "alloc_mem.h"
#include "nodemem.h"
#include "alloc.h"
#include "allocconf.h"

static MT_USIGN get_granularized_wanted_len(MT_USIGN const wanted_len)
{
    MT_USIGN ret_val = wanted_len;

    while(ret_val % MT_ALLOC_GRANULARITY != 0)
    {
        ++ret_val;
    }
    return ret_val;
}

void alloc_free(void * const block_addr)
{
    struct node * n = 0;

    if(block_addr == 0)
    {
        //assert(false);
        return; // Nothing to do.
    }

    n = nodemem_get_block_node_addr(block_addr);
    if(n == 0)
    {
        assert(false);
        return; // No block at given (start) address is allocated.
    }

    if(n->is_allocated == 0)
    {
        assert(false);
        return; // Already deallocated.
    }

    n->is_allocated = 0;

#ifndef NDEBUG
    alloc_mem_clear(n->block_addr, n->block_len, MT_ALLOC_DEB_CLR_4);
#endif //NDEBUG

    if(n->next_node_addr != 0)
    {
        nodemem_merge_unallocated_with_next_if_possible(n);
    }
    if(n->last_node_addr != 0)
    {
        struct node * last = n->last_node_addr;

        if(last->is_allocated == 0)
        {
            nodemem_merge_unallocated_with_next_if_possible(n->last_node_addr);
        }
    }

    nodemem_limit_free_nodes();
}

void* alloc_alloc(MT_USIGN const wanted_len)
{
    struct node * new_node_addr = 0,
        * node_addr = 0,
        new_node;

    if(!nodemem_is_initialized())
    {
        return 0;
    }

    if(wanted_len == 0)
    {
        return 0;
    }

    MT_USIGN const gran_wanted_len = get_granularized_wanted_len(wanted_len);

    // Make sure that there is a node space available, first
    // (and maybe frees space from first node, which must
    // be done BEFORE first node may gets selected as
    // "alloc" node):
    //
    if(!nodemem_try_to_reserve_node_space())
    {
        return 0; // No more space for another node available.
    }

    node_addr = nodemem_get_alloc_node_addr(gran_wanted_len);
    if(node_addr == 0)
    {
        return 0;
    }

    assert(node_addr->is_allocated == 0);

    if(node_addr->block_len == gran_wanted_len)
    {
        node_addr->is_allocated = 1;

        return node_addr->block_addr;
    }

    new_node.block_len = gran_wanted_len;
    new_node.is_allocated = 1;
    new_node.last_node_addr = node_addr;
    new_node.next_node_addr = node_addr->next_node_addr;
    new_node.block_addr =
        node_addr->block_addr + node_addr->block_len - gran_wanted_len;

    new_node_addr = nodemem_store(&new_node);
    if(new_node_addr == 0)
    {
        return 0;
    }

    assert(node_addr->block_len > new_node.block_len);

    node_addr->block_len -= new_node.block_len;
    node_addr->next_node_addr = new_node_addr;

    if (new_node.next_node_addr != 0)
    {
        struct node * const next_node = new_node.next_node_addr;

        next_node->last_node_addr = new_node_addr;
    }

    return new_node.block_addr;
}

void alloc_init(void * const mem, MT_USIGN const mem_len)
{
    nodemem_init(mem, mem_len);
}
