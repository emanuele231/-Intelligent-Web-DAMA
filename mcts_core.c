#include "mcts_core.h"
#include <string.h>

void init_pool(MemoryPool* pool) {
    pool->top = 0;
}

MCTSNode* alloc_node(MemoryPool* pool) {
    if (pool->top >= MAX_NODES) return NULL;
    MCTSNode* node = &pool->nodes[pool->top++];
    node->parent = NULL;
    node->visits = 0;
    node->wins = 0.0;
    node->num_children = 0;
    node->state = NULL;
    memset(node->children, 0, sizeof(node->children));
    return node;
}