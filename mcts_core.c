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

// ============================================================================
// LOGICA MCTS BASE (spostata qui per risolvere il linking)
// ============================================================================

double simulate_rollout(Bitboard *state) {
    int b_cnt = 0, w_cnt = 0;
    uint64_t tb = state->black | state->black_k;
    uint64_t tw = state->white | state->white_k;
    while (tb) { b_cnt += (tb & 1); tb >>= 1; }
    while (tw) { w_cnt += (tw & 1); tw >>= 1; }
    for (int i = 0; i < 20; i++) {
        if (b_cnt == 0) return 0.0;
        if (w_cnt == 0) return 1.0;
        if (rand() % 10 == 0) b_cnt--;
        if (rand() % 10 == 0) w_cnt--;
    }
    if (b_cnt > w_cnt + 1) return 1.0;
    if (w_cnt > b_cnt + 1) return 0.0;
    return 0.5;
}

uint8_t generate_legal_moves(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    // 1. Genera catture
    uint64_t current = bb->black | bb->black_k;
    uint64_t opponent = bb->white | bb->white_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;
    uint8_t cap_count = 0;
    
    for (int bit = 0; bit < 64; bit++) {
        if (!((current >> bit) & 1ULL)) continue;
        int r = bit / 8, c = bit % 8;
        int is_king = (bb->black_k >> bit) & 1ULL;
        int dr_list[4] = { (is_king || r < 4) ? 2 : 0, (is_king || r < 4) ? -2 : 0, is_king ? 2 : 0, is_king ? -2 : 0 };
        int dc_list[4] = {-2, 2, 2, -2};
        int valid_dirs = is_king ? 4 : 2;
        for (int i = 0; i < valid_dirs; i++) {
            int dr = dr_list[i], dc = dc_list[i];
            if (dr == 0) continue;
            int mid_r = r + dr/2, mid_c = c + dc/2;
            int to_r = r + dr, to_c = c + dc;
            if (mid_r < 0 || mid_r >= 8 || mid_c < 0 || mid_c >= 8) continue;
            if (to_r < 0 || to_r >= 8 || to_c < 0 || to_c >= 8) continue;
            int mid_bit = mid_r * 8 + mid_c, to_bit = to_r * 8 + to_c;
            if (((opponent >> mid_bit) & 1ULL) && !((occupied >> to_bit) & 1ULL)) {
                if (cap_count < MAX_CHILDREN && pool->top < MAX_NODES) {
                    MCTSNode *ch = &pool->nodes[pool->top++];
                    memset(ch, 0, sizeof(MCTSNode));
                    ch->state = bb;
                    ch->move.from = (uint8_t)bit;
                    ch->move.to = (uint8_t)to_bit;
                    ch->move.capture = 1;
                    children[cap_count++] = ch;
                }
            }
        }
    }
    if (cap_count > 0) return cap_count; // Regola italiana: cattura obbligatoria

    // 2. Mosse semplici
    uint8_t count = 0;
    for (int bit = 0; bit < 64; bit++) {
        if (!((current >> bit) & 1ULL)) continue;
        int r = bit / 8, c = bit % 8;
        int dr = (r < 4) ? 1 : -1;
        int dc[2] = {-1, 1};
        for (int i = 0; i < 2; i++) {
            int nr = r + dr, nc = c + dc[i];
            if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
            if ((nr + nc) % 2 == 0) continue;
            int nbit = nr * 8 + nc;
            if ((occupied >> nbit) & 1ULL) continue;
            if (count < MAX_CHILDREN && pool->top < MAX_NODES) {
                MCTSNode *ch = &pool->nodes[pool->top++];
                memset(ch, 0, sizeof(MCTSNode));
                ch->state = bb;
                ch->move.from = (uint8_t)bit;
                ch->move.to = (uint8_t)nbit;
                ch->move.capture = 0;
                children[count++] = ch;
            }
        }
    }
    return count;
}
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