
// RhinoDevel, MT, 2018nov17

#ifndef MT_NODEMEM
#define MT_NODEMEM

#include <stdint.h>
#include <stdbool.h>

#include "node.h"
#include "allocconf.h"

struct node * nodemem_get_block_node_addr(void const * const block_addr);

bool nodemem_try_to_reserve_node_space();

/** Returns stored node's address or 0, if no space left.
 */
struct node * nodemem_store(struct node const * const n);

/** Returns address of node with unallocated block of sufficient size to hold
 *  given/wanted amount of bytes.
 *
 *  Returns 0, if no such node was found.
 */
struct node * nodemem_get_alloc_node_addr(MT_USIGN const wanted_len);

/** Node at given address must be unallocated!
 */
void nodemem_merge_unallocated_with_next_if_possible(
    struct node * const unallocated_node_addr);

void nodemem_limit_free_nodes();

/** Occupy heap space with one node object that reserves the whole rest
 *  of heap space as one single unallocated node.
 */
void nodemem_init(void * const mem, MT_USIGN const mem_len);

#endif //MT_NODEMEM
