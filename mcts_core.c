#include "mcts_core.h"
#include <stdlib.h>   // malloc, free
#include <string.h>   // memset
#include <math.h>     // sqrt, log, logf
#include <float.h>    // DBL_MAX
#include <time.h>     // clock_t, CLOCKS_PER_SEC

// Struttura interna per l'istanza MCTS
typedef struct {
    MemoryPool pool;
    AIConfig cfg;
    MCTSNode* root;
} MCTSInstance;

// ============================================================================
// FUNZIONI BASE (già esistenti nel tuo codice)
// ============================================================================

void init_pool(MemoryPool* pool) {
    pool->top = 0;
}

MCTSNode* alloc_node(MemoryPool* pool) {
    if (pool->top >= MAX_NODES) return NULL;
    MCTSNode* node = &pool->nodes[pool->top++];
    memset(node, 0, sizeof(MCTSNode));
    return node;
}

// [Mantieni qui le tue funzioni esistenti: generate_legal_moves, simulate_rollout, ecc.]
// ... (copia il codice che già funzionava per queste funzioni) ...

// ============================================================================
// CALCOLO PUNTEGGIO UNIFICATO (supporta tutte le formule)
// ============================================================================

static double calculate_score(MCTSNode* node, double parent_visits, const AIConfig* cfg) {
    if (node->visits == 0) return DBL_MAX;
    
    double q = node->wins / node->visits;
    double bonus = 0.0;
    
    switch (cfg->algo) {
        case ALGO_UCB1_CLASSIC:
        case ALGO_UCB_FAST:
            bonus = cfg->ucb_c * sqrt(log(parent_visits) / node->visits);
            break;
        case ALGO_UCB_DELTA:
            bonus = sqrt(2.0f * logf(1.0f / cfg->delta) / node->visits);
            break;
        case ALGO_UCB_ALPHA:
            bonus = cfg->alpha * sqrt(log(parent_visits) / node->visits);
            break;
        case ALGO_PUCT_STD:
        case ALGO_PUCT_EXP:
        case ALGO_PUCT_HEUR:
        case ALGO_PUCT_BAL:
            bonus = cfg->cpuct * sqrt(parent_visits) / (1.0 + node->visits);
            break;
    }
    return q + bonus;
}

static MCTSNode* select_best(MCTSNode* parent, const AIConfig* cfg) {
    if (!parent || parent->num_children == 0) return NULL;
    MCTSNode* best = parent->children[0];
    double best_score = calculate_score(best, parent->visits, cfg);
    
    for (int i = 1; i < parent->num_children; i++) {
        double score = calculate_score(parent->children[i], parent->visits, cfg);
        if (score > best_score) {
            best_score = score;
            best = parent->children[i];
        }
    }
    return best;
}

// ============================================================================
// WRAPPER PER IL SISTEMA MODULARE
// ============================================================================

AI_Instance* mcts_create(const AIConfig* cfg) {
    MCTSInstance* inst = malloc(sizeof(MCTSInstance));
    if (!inst) return NULL;
    inst->cfg = cfg ? *cfg : (AIConfig){0};
    init_pool(&inst->pool);
    inst->root = NULL;
    return (AI_Instance*)inst;
}

void mcts_destroy(AI_Instance* instance) {
    free(instance);
}

Move mcts_get_move(AI_Instance* instance, Bitboard* state, float time_budget) {
    MCTSInstance* inst = (MCTSInstance*)instance;
    if (!inst) { Move null = {255,255,0}; return null; }
    
    init_pool(&inst->pool);
    inst->root = alloc_node(&inst->pool);
    if (!inst->root) { Move null = {255,255,0}; return null; }
    
    inst->root->state = state;
    inst->root->num_children = generate_legal_moves(state, inst->root->children, &inst->pool);
    if (inst->root->num_children == 0) { Move null = {255,255,0}; return null; }
    
    clock_t start = clock();
    clock_t limit = (clock_t)(time_budget * CLOCKS_PER_SEC);
    
    while (clock() - start < limit) {
        MCTSNode* curr = inst->root;
        while (curr->num_children > 0) {
            MCTSNode* next = select_best(curr, &inst->cfg);
            if (!next) break;
            curr = next;
        }
        double res = simulate_rollout(curr->state);
        while (curr) {
            curr->visits++;
            curr->wins += res;
            res = 1.0 - res;
            curr = curr->parent;
        }
    }
    
    MCTSNode* best = inst->root->children[0];
    for (int i = 1; i < inst->root->num_children; i++) {
        if (inst->root->children[i]->visits > best->visits) best = inst->root->children[i];
    }
    return best->move;
}