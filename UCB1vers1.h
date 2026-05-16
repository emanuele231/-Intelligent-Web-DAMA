#ifndef UCB1VER1_H
#define UCB1VER1_H

#include <stdint.h>
#include <stdbool.h>
#include "bitboard.h"

#define UCB1_C             1.414f
#define TIME_LIMIT_DEFAULT 0.2f
#define MAX_NODES          40000
#define MAX_CHILDREN       8

typedef struct {
    uint8_t from;
    uint8_t to;
    uint8_t capture;
} Move;

typedef struct MCTSNode MCTSNode;

struct MCTSNode {
    Bitboard *state;
    struct MCTSNode *parent;
    struct MCTSNode *children[MAX_CHILDREN];
    uint32_t visits;
    double wins;
    Move move;
    uint8_t num_children;
};

typedef struct {
    MCTSNode nodes[MAX_NODES];
    int top;
} MemoryPool;

void init_pool(MemoryPool *pool);
void mcts_search(Bitboard *root_state, float time_limit, MemoryPool *pool);
double ucb1_score(MCTSNode *node, double parent_visits);
MCTSNode* select_best_child(MCTSNode *parent);
Move get_best_move(MCTSNode *root);


#endif