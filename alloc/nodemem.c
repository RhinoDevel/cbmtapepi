
// RhinoDevel, MT, 2018nov17

#include <stdint.h>
#include <stdbool.h>

#include "../assert.h"
#include "nodemem.h"
#include "mem.h"
#include "node.h"
#include "allocconf.h"

static MT_USIGN const s_node_free_flag_word = MT_USIGN_MAX;
static MT_USIGN const s_node_len = sizeof (struct node);

static MT_USIGN s_mem_len = 0;
static MT_USIGN s_max_node_count = 0; // How many nodes can currently be stored.
static struct node * s_first_node_addr = 0; // First node's address.

static void * get_first_block_addr()
{
    assert(s_first_node_addr != 0);
    assert(s_max_node_count != 0);

    // First block begins behind last node.

    return s_first_node_addr + s_max_node_count;
}

/** Mark node's space in memory at given position as free for reuse.
 */
static void mark_node_space_as_free(struct node * const node_addr)
{
    assert(s_first_node_addr != 0);
    assert(node_addr >= s_first_node_addr);
    assert((void*)node_addr <= get_first_block_addr());

    *((MT_USIGN*)node_addr) = s_node_free_flag_word;
}

static struct node * get_free_node_addr()
{
    assert(s_max_node_count != 0);
    assert(s_first_node_addr != 0);

    for(MT_USIGN i = 0;i < s_max_node_count; ++i)
    {
        struct node * const addr = s_first_node_addr + i;
        MT_USIGN const first_word = *((MT_USIGN const *)addr);

        if (first_word == s_node_free_flag_word)
        {
            return addr; // Found free space for a node.
        }
    }
    return 0; // No space for a node found.
}

static struct node * get_first_block_node_addr()
{
    return nodemem_get_block_node_addr(get_first_block_addr());
}

static struct node * create_free_node_addr()
{
    assert(get_free_node_addr() == 0);

    struct node * const first_block_node_addr =
        get_first_block_node_addr();
    struct node * free_node_addr = 0;

    // Is first block's node unallocated?
    //
    if(first_block_node_addr->is_allocated != 0)
    {
        return 0;
    }

    // Is first block's length equal or longer than a node's length
    // plus one (at least one byte must be on stack for new node to
    // be created for this to makes sense..)?
    //
    if(first_block_node_addr->block_len < s_node_len + 1)
    {
        return 0;
    }

    free_node_addr = (struct node *)(first_block_node_addr->block_addr);
    first_block_node_addr->block_addr += s_node_len;
    first_block_node_addr->block_len -= s_node_len;

    ++s_max_node_count;
    mark_node_space_as_free(free_node_addr);

    assert(get_free_node_addr() == free_node_addr);
    return free_node_addr;
}

static struct node * get_or_create_free_node_space()
{
    struct node * node_addr = get_free_node_addr();

    if(node_addr != 0)
    {
        return node_addr;
    }
    return create_free_node_addr();
}

struct node * nodemem_get_block_node_addr(void const * const block_addr)
{
    assert(block_addr != 0);
    assert(s_max_node_count != 0);
    assert(s_first_node_addr != 0);

    struct node * ret_val = 0;

    for (MT_USIGN i = 0; i < s_max_node_count; ++i)
    {
        struct node * const addr = s_first_node_addr + i;
        MT_USIGN const first_word = *((MT_USIGN const *)addr);

        if (first_word == s_node_free_flag_word)
        {
            continue; // Free space for a node.
        }

        if(addr->block_addr == block_addr)
        {
            ret_val = addr;
            break;
        }
    }
    assert(ret_val != 0);
    return ret_val;
}

bool nodemem_try_to_reserve_node_space()
{
    return get_or_create_free_node_space() != 0;
}

struct node * nodemem_store(struct node const * const n)
{
    assert(n != 0);

    struct node * const ret_val = get_or_create_free_node_space();

    if(ret_val == 0)
    {
        return 0;
    }

    ret_val->last_node_addr = n->last_node_addr;
    ret_val->block_addr = n->block_addr;
    ret_val->block_len = n->block_len;
    ret_val->is_allocated = n->is_allocated;
    ret_val->next_node_addr = n->next_node_addr;

    return ret_val;
}

struct node * nodemem_get_alloc_node_addr(MT_USIGN const wanted_len)
{
    assert(wanted_len != 0);

    struct node * ret_val = 0;
    void const * const first_block_addr = get_first_block_addr();

    for (struct node * addr = s_first_node_addr;
        addr != 0;
        addr = addr->next_node_addr)
    {
        if (addr->is_allocated != 0)
        {
            continue; // Node's block is already allocated.
        }

        if (addr->block_len < wanted_len)
        {
            continue; // Would not fit in node's block.
        }

        if (addr->block_addr == first_block_addr)
        {
            if (ret_val == 0)
            {
                ret_val = addr; // No better candidate found, yet.
            }
            continue;
        }

        if (addr->block_len == wanted_len)
        {
            ret_val = addr;
            break; // Fits perfectly. Found!
        }

        if (ret_val == 0)
        {
            ret_val = addr;
            continue; // No better candidate found, yet.
        }

        if (ret_val->block_len > addr->block_len)
        {
            // Would fit better than current best candidate.

            ret_val = addr;
        }
    }
    return ret_val;
}

static MT_USIGN get_last_free_node_count()
{
    assert(*((MT_USIGN*)s_first_node_addr) != s_node_free_flag_word);
    assert(sizeof (uint8_t) == 1);

    MT_USIGN retVal = MT_USIGN_MAX;
    uint8_t const * addr = (uint8_t*)get_first_block_addr();

    do
    {
        ++retVal;

        addr -= s_node_len;
    } while (*((MT_USIGN const *)addr) == s_node_free_flag_word);

    return retVal;
}

void nodemem_merge_unallocated_with_next_if_possible(
    struct node * const unallocated_node_addr)
{
    assert(unallocated_node_addr->is_allocated == 0);

    if(unallocated_node_addr->next_node_addr == 0)
    {
        return; // Nothing to do.
    }

    struct node const * const next = unallocated_node_addr->next_node_addr;

    assert(
        unallocated_node_addr->block_addr + unallocated_node_addr->block_len
            == next->block_addr);

    if(next->is_allocated != 0)
    {
        return; // Not possible. Nothing to do.
    }

    if(next->next_node_addr != 0)
    {
        // Merge current with next node:

        struct node * next_next_node = next->next_node_addr;

        assert(
            next_next_node->last_node_addr
                == unallocated_node_addr->next_node_addr);
        assert(next_next_node->is_allocated == 1);
        assert(
            next->block_addr + next->block_len == next_next_node->block_addr);

        next_next_node->last_node_addr = unallocated_node_addr;
    }

    mark_node_space_as_free(unallocated_node_addr->next_node_addr);

    unallocated_node_addr->next_node_addr = next->next_node_addr;
    unallocated_node_addr->block_len += next->block_len;
}

void nodemem_limit_free_nodes()
{
    // ??????????????????|FFFF??????????????|*
    // ^                  ^                  ^
    // |                  |                  |
    // s_first_node_addr     |                  first block's address
    // OR                 Last free node's address
    // last non-free
    // node's address

    MT_USIGN c = 0;

    if(s_first_node_addr->is_allocated != 0)
    {
        // Debug.WriteLine(
        //     "NodeMem.LimitFreeNodes : Warning:"
        //     + " First node is allocated.");
        return;
    }

    c = get_last_free_node_count();
    if(c < 2)
    {
        return; // Nothing to do (keep one free node, if existing).
    }
    --c; // Keep a single free node.

    s_max_node_count -= c;
    s_first_node_addr->block_len += (MT_USIGN)(c * s_node_len);
    s_first_node_addr->block_addr = get_first_block_addr();

    assert(
        *((MT_USIGN*)(s_first_node_addr->block_addr)) == s_node_free_flag_word);
    assert(get_last_free_node_count() == 1);

#ifndef NDEBUG
    mem_clear(
        s_first_node_addr->block_addr, s_first_node_addr->block_len, MT_ALLOC_DEB_CLR_3);
#endif //NDEBUG
}

void nodemem_init(void * const mem, MT_USIGN const mem_len)
{
    assert(s_max_node_count == 0);
    assert(s_first_node_addr == 0);
    assert(s_mem_len == 0);

    assert(mem != 0);
    assert(mem_len != 0);

    s_mem_len = mem_len;
    s_max_node_count = 1;
    s_first_node_addr = (struct node *)mem;

#ifndef NDEBUG
    mem_clear(s_first_node_addr, s_node_len, MT_ALLOC_DEB_CLR_5);
#endif //NDEBUG

    // The first (and initially only) block will occupy the complete
    // last part of heap space after the first node's memory space:
    //
    s_first_node_addr->last_node_addr = 0;
    s_first_node_addr->block_addr = get_first_block_addr();
    s_first_node_addr->block_len = s_mem_len - s_node_len;
    s_first_node_addr->is_allocated = 0;
    s_first_node_addr->next_node_addr = 0;
}
