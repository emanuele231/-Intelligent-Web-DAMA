#ifndef MCTS_CORE_H
#define MCTS_CORE_H

#include "bitboard.h"
#include "moves.h"
#include "ai_engine.h" 
#include <stdint.h>
#include <stdbool.h>

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

// Funzioni base (NON static, visibili esternamente)
void init_pool(MemoryPool* pool);
MCTSNode* alloc_node(MemoryPool* pool);
uint8_t generate_legal_moves(Bitboard* bb, MCTSNode* children[], MemoryPool* pool);
double simulate_rollout(Bitboard* state);

// Wrapper per il sistema AI modulare
AI_Instance* mcts_create(const AIConfig* cfg);
void mcts_destroy(AI_Instance* inst);
Move mcts_get_move(AI_Instance* inst, Bitboard* state, float time_budget);

#endif