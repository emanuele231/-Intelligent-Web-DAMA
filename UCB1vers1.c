#include "UCB1vers1.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

void init_pool(MemoryPool *pool) {
    pool->top = 0;
}

static MTCSNode* alloc_node(MemoryPool *pool){
        if(pool->top >=MAX_NODES) {
        fprintf(stderr, "Pool esaurito, aumentare il MAX_NODES");
        return NULL;
    }
    MTCSNode *node = &pool->nodes[pool->top++];
    node->parent = NULL;
    node->visits = 0;
    node->wins = 0.0;
    node->num_children = 0;
    node->state = NULL;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

void mcts_search(Bitboard *current_board, float time_limit, MemoryPool *pool) {
    // loop anytime + 4 fasi MCTS
    init_pool(pool);
    MTCSNode *root = alloc_node(pool);
    if(!root) return;

    root->state = current_board;

    root->move.from = 255;
    root->move.to = 255;
    root->move.capture = 0;

    root->num_children = generate_legal_moves(current_board, root->children, pool);
    printf("Root inizializzato | Board: %p | Figli legali: %d\n");
    (void*)current_board, root->num_children;

    if (root->num_children == 0){
        printf("partita terminata o in stallo, nessuna mossa legale");
        return;
    }
}

//Generatore di mosse legali
uint8_t generate_legal_moves(Bitboard *board, MTCSNode *children[], MemoryPool *pool) {
    return 0;
}
/*
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
}*/