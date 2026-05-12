#include "UCB1vers1.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

void init_pool(MemoryPool *pool) {
    pool->top = 0;
}

void mcts_search(Bitboard *root_state, float time_limit, MemoryPool *pool) {
    // TODO: loop anytime + 4 fasi MCTS
}

Move get_best_move(MCTSNode *root) {
    // TODO: seleziona figlio con max visits
    Move null_move = {0, 0, 0};
    return null_move;
}

double ucb1_score(MCTSNode *node, double parent_visits) {
    // TODO: formula UCB1
    return 0.0;
}

MCTSNode* select_best_child(MCTSNode *parent) {
    // TODO: argmax UCB1
    return NULL;
}