#include "UCB1vers1.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

void init_pool(MemoryPool *pool) {
    pool->top = 0;
}

static MCTSNode* alloc_node(MemoryPool *pool){
        if(pool->top >=MAX_NODES) {
        fprintf(stderr, "Pool esaurito, aumentare il MAX_NODES");
        return NULL;
    }
    MCTSNode *node = &pool->nodes[pool->top++];
    node->parent = NULL;
    node->visits = 0;
    node->wins = 0.0;
    node->num_children = 0;
    node->state = NULL;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

double ucb1_score(MCTSNode *node, double parent_visits){
    if(node->visits == 0) return DBL_MAX;
    double exploitation = node->wins / node->visits;
    double exploration = UCB1_C * sqrt(log(parent_visits) / node->visits);
    return exploitation + exploration;
}

MCTSNode* select_best_child(MCTSNode *parent) {
    if (!parent || parent->num_children == 0) return NULL;

    MCTSNode *best = parent->children[0];
    double best_score = ucb1_score(best, parent->visits);

    for(int i = 1; i < parent->num_children; i++){
        double score = ucb1_score(parent->children[i], parent->visits);
        if(score > best_score) {
            best_score = score;
            best = parent->children[i];
        }
    }
    return best;
}
// roll-out simuliamo una partita veloce a caso
double simulate_rollout(Bitboard *state){
    return (rand() % 2 == 0) ? 1.0 : 0.0;
}

void expand_node(MCTSNode *node, MemoryPool *pool){
    if (node->num_children >= MAX_CHILDREN) return;

    MCTSNode *child = alloc_node(pool);
    if (!child) return;

    child->parent = node;
    child->state = node->state;

    child->move.from = rand() % 64;
    child->move.to = rand() % 64;

    node->children[node->num_children++] = child;
}

uint8_t generate_legal_moves(Bitboard *board, MCTSNode *children[], MemoryPool *pool) {
    (void)board; (void)children; (void)pool;
    return 0;
}

void mcts_search(Bitboard *current_board, float time_limit, MemoryPool *pool) {
    // loop anytime + 4 fasi MCTS
    init_pool(pool);
    MCTSNode *root = alloc_node(pool);
    if(!root) return;

    root->state = current_board;
    root->num_children = 0;
    printf("start MTCS search...\n");

    //1. SELECTION
    clock_t start = clock();
    int iterations = 0;
    while (((clock() - start) / (float)CLOCKS_PER_SEC) < time_limit) {
        //1. SELECTION
        MCTSNode *current = root;
        while (current->num_children > 0 && current->children[0] != NULL){
            MCTSNode *best_child = select_best_child(current);
            if(best_child) {
                current = best_child;
            } else break;
        
        //2. EXPANSION
         if (current->num_children < MAX_CHILDREN) {
            expand_node(current, pool);
            current = current->children[current->num_children - 1]; // Vai sul nuovo nodo
        }
        //3. SIMULATION
        double result = simulate_rollout(current->state);

        //4. BACKPROPAGATION
        MCTSNode *temp = current;
        while (temp != NULL) {
            temp->visits++;
            temp->wins += result; 
            // Inverti il risultato per il giocatore precedente
            // (Se io vinco 1.0, il mio genitore ha subito una sconfitta 0.0)
            result = 1.0 - result;
            temp = temp->parent;
        }
        iterations++;
    }
    printf("UCB1 Selection: %d iterazioni in %.3f sec\n", iterations, time_limit);
  }
}

Move get_best_move(MCTSNode *root) {
    Move null_move = {0, 0, 0};
    if (!root || root->num_children == 0) return null_move;

    // Scegli il figlio con PIÙ VISITE (più robusto di UCB1 alla fine)
    MCTSNode *best = root->children[0];
    for (int i = 1; i < root->num_children; i++) {
        if (root->children[i]->visits > best->visits) {
            best = root->children[i];
        }
    }
    
    printf("Mossa Scelta: From %d -> To %d (Visits: %d)\n", 
           best->move.from, best->move.to, best->visits);
    return best->move;
    }
