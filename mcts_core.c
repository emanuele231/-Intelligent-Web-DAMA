#include "mcts_core.h"
#include <stdlib.h>   // malloc, free
#include <string.h>   // memset
#include <stdio.h>
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
    uint8_t cap_count = 0;
    uint8_t move_count = 0;

    uint64_t my_pieces = bb->black | bb->black_k;
    uint64_t opp_pieces = bb->white | bb->white_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;

    printf("🔍 IA NERA genera mosse. Pezzi neri: %lld, Pezzi bianchi: %lld\n", 
           my_pieces, opp_pieces);

    // ==========================================
    // FASE 1: CERCA CATTURE (OBBLIGATORIE)
    // ==========================================
    for (int bit = 0; bit < 64; bit++) {
        if (!((my_pieces >> bit) & 1ULL)) continue;

        int r = bit / 8, c = bit % 8;
        int is_king = (bb->black_k >> bit) & 1ULL;

        // 4 direzioni diagonali
        int dr[4] = {-2, -2, 2, 2};
        int dc[4] = {-2, 2, -2, 2};

        for (int i = 0; i < 4; i++) {
            // Pedina nera: solo avanti (+2), Dama: tutte le direzioni
            if (!is_king && dr[i] != 2) continue;

            int mid_r = r + dr[i] / 2;
            int mid_c = c + dc[i] / 2;
            int to_r = r + dr[i];
            int to_c = c + dc[i];

            if (to_r < 0 || to_r >= 8 || to_c < 0 || to_c >= 8) continue;

            int mid_bit = mid_r * 8 + mid_c;
            int to_bit = to_r * 8 + to_c;

            // Deve esserci un pezzo avversario in mezzo
            if (!((opp_pieces >> mid_bit) & 1ULL)) continue;
            // Destinazione deve essere libera
            if ((occupied >> to_bit) & 1ULL) continue;

            if (cap_count < MAX_CHILDREN && pool->top < MAX_NODES) {
                MCTSNode *ch = &pool->nodes[pool->top++];
                memset(ch, 0, sizeof(MCTSNode));
                ch->state = bb;
                ch->move.from = (uint8_t)bit;
                ch->move.to = (uint8_t)to_bit;
                ch->move.capture = 1;
                children[cap_count++] = ch;
                printf("   ✅ Cattura trovata: (%d,%d) -> (%d,%d)\n", r, c, to_r, to_c);
            }
        }
    }

    if (cap_count > 0) {
        printf("🎯 IA NERA: %d CATTURE disponibili (OBBLIGO)\n", cap_count);
        return cap_count;
    }

    // ==========================================
    // FASE 2: MOSSE SEMPLICI (se nessuna cattura)
    // ==========================================
    for (int bit = 0; bit < 64; bit++) {
        if (!((my_pieces >> bit) & 1ULL)) continue;

        int r = bit / 8, c = bit % 8;
        int is_king = (bb->black_k >> bit) & 1ULL;

        int dr[4] = {-1, -1, 1, 1};
        int dc[4] = {-1, 1, -1, 1};

        for (int i = 0; i < 4; i++) {
            if (!is_king && dr[i] != 1) continue; // Nero va solo giù

            int to_r = r + dr[i];
            int to_c = c + dc[i];
            if (to_r < 0 || to_r >= 8 || to_c < 0 || to_c >= 8) continue;

            int to_bit = to_r * 8 + to_c;
            if ((occupied >> to_bit) & 1ULL) continue;

            if (move_count < MAX_CHILDREN && pool->top < MAX_NODES) {
                MCTSNode *ch = &pool->nodes[pool->top++];
                memset(ch, 0, sizeof(MCTSNode));
                ch->state = bb;
                ch->move.from = (uint8_t)bit;
                ch->move.to = (uint8_t)to_bit;
                ch->move.capture = 0;
                children[move_count++] = ch;
            }
        }
    }

    printf(" IA NERA: %d mosse semplici trovate\n", move_count);
    
    if (move_count == 0 && cap_count == 0) {
        printf(" IA NERA: NESSUNA MOSSA DISPONIBILE! Sconfitta.\n");
    }
    
    return move_count;
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