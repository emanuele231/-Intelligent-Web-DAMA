#include "ai_engine.h"
#include "UCB1.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <windows.h>

typedef struct {
    MemoryPool pool;
    AIConfig cfg;
} UCB1Instance;

//-----------CALCOLO VALORE UCB1 DI UN NODO
static double ucb1_score(MCTSNode *node, double parent_visits) {
    if (node->visits == 0) return DBL_MAX;
    double exploitation = node->wins / node->visits;
    double exploration = 1.414f * sqrt(log(parent_visits) / node->visits);
    return exploitation + exploration;
}

//----------------SELEZIONE FIGLIO CON PUNTEGGIO MASSIMO
static MCTSNode* select_best_child(MCTSNode *parent) {
    if (!parent || parent->num_children == 0) return NULL;
    MCTSNode *best = parent->children[0];
    double best_score = ucb1_score(best, parent->visits);
    for (int i = 1; i < parent->num_children; i++) {
        double score = ucb1_score(parent->children[i], parent->visits);
        if (score > best_score) { best_score = score; best = parent->children[i]; }
    }
    return best;
}

//-------------------SIMULAZIONE DELLA PARTITA 
static double simulate_rollout(Bitboard *state) {
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


//==================================================
// MOSSE
//==================================================

//CATTURA
static uint8_t generate_captures(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    uint64_t current = bb->black | bb->black_k;
    uint64_t opponent = bb->white | bb->white_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;
    uint8_t count = 0;
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
                if (count < MAX_CHILDREN && pool->top < MAX_NODES) {
                    MCTSNode *ch = &pool->nodes[pool->top++];
                    memset(ch, 0, sizeof(MCTSNode));
                    ch->state = bb;
                    ch->move.from = (uint8_t)bit;
                    ch->move.to = (uint8_t)to_bit;
                    ch->move.capture = 1;
                    children[count++] = ch;
                }
            }
        }
    }
    return count;
}

//MOVIMENTO
static uint8_t generate_legal_moves(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    uint8_t cap_count = generate_captures(bb, children, pool);
    if (cap_count > 0) return cap_count;
    uint64_t current = bb->black | bb->black_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;
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

//==================================================
// RICERCA E DECISIONE
//==================================================

//----------------------------RICERCA
static void mcts_search_impl(Bitboard *current_board, float time_limit, MemoryPool *pool, float ucb_c) {
    init_pool(pool);
    MCTSNode *root = alloc_node(pool);
    if (!root) return;
    root->state = current_board;
    root->num_children = generate_legal_moves(current_board, root->children, pool);
    if (root->num_children == 0) return;
    DWORD start = GetTickCount();
    DWORD limit_ms = (DWORD)(time_limit * 1000);
    while (1) {
        if (GetTickCount() - start >= limit_ms) break;
        MCTSNode *current = root;
        while (current->num_children > 0) {
            MCTSNode *next = select_best_child(current);
            if (!next) break;
            current = next;
        }
        double result = simulate_rollout(current->state);
        MCTSNode *temp = current;
        while (temp != NULL) {
            temp->visits++;
            temp->wins += result;
            result = 1.0 - result;
            temp = temp->parent;
        }
    }
}

//------------------ESTRAZIONE MOSSA CON PIUì VISITE
static Move get_best_move_impl(MCTSNode *root, Bitboard *bb) {
    Move null_move = {255, 255, 0};
    if (!root || root->num_children == 0) return null_move;
    MCTSNode *best = root->children[0];
    for (int i = 1; i < root->num_children; i++)
        if (root->children[i]->visits > best->visits) best = root->children[i];
    if (best) {
        int r = best->move.from / 8, c = best->move.from % 8;
        if (r < 0 || r >= 8 || c < 0 || c >= 8 || !((bb->black | bb->black_k) >> (r*8+c) & 1ULL))
            best = root->children[0];
        return best->move;
    }
    return null_move;
}

// === AI Engine Interface ===
static AI_Instance* ucb1_create(const AIConfig* cfg) {
    UCB1Instance* inst = calloc(1, sizeof(UCB1Instance));
    inst->cfg = cfg ? *cfg : (AIConfig){.ucb_c=1.414f, .time_limit=0.2f, .max_nodes=40000};
    init_pool(&inst->pool);
    return (AI_Instance*)inst;
}
// === MOSSA ===
static Move ucb1_get_move(AI_Instance* instance, Bitboard* board, float time_budget) {
    UCB1Instance* inst = (UCB1Instance*)instance;
    mcts_search_impl(board, time_budget, &inst->pool, inst->cfg.ucb_c);
    return get_best_move_impl(&inst->pool.nodes[0], board);
}


static void ucb1_destroy(AI_Instance* instance) {
    free(instance);
}

static const AIEngineDef ucb1_def = {
    .id = "ucb1_base",
    .name = "UCB1-0.2 (Base)",
    .description = "UCB1 standard, 0.2s per mossa",
    .header_color = {0, 0, 0, 150},
    .default_cfg = {.ucb_c = 1.414f, .time_limit = 0.2f, .max_nodes = 40000},
    .create = ucb1_create,
    .get_move = ucb1_get_move,
    .destroy = ucb1_destroy
};

__attribute__((constructor)) void register_ucb1_base() { ai_register(&ucb1_def); }