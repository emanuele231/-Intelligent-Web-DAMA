#ifndef MCTS_CORE_H
#define MCTS_CORE_H

#include "bitboard.h"
#include "moves.h"
#include <stdint.h>

#define MAX_CHILDREN 64
#define MAX_NODES 40000

typedef struct MCTSNode {
    Bitboard* state;
    Move move;
    struct MCTSNode* parent;
    struct MCTSNode* children[MAX_CHILDREN];
    uint8_t num_children;
    uint8_t capture;
    int visits;
    double wins;
} MCTSNode;

typedef struct {
    MCTSNode nodes[MAX_NODES];
    int top;
} MemoryPool;

void init_pool(MemoryPool* pool);
MCTSNode* alloc_node(MemoryPool* pool);

#endif // MCTS_CORE_H