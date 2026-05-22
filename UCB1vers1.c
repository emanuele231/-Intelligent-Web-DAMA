#include "UCB1vers1.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>
#include <windows.h>

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
    int black_count = 0;
    int white_count = 0;
    uint64_t temp_b = state->black | state->black_k;
    uint64_t temp_w = state->white | state->white_k;

    while(temp_b) {if(temp_b&1) black_count++; temp_b >>= 1; }
    while(temp_w) {if(temp_w&1) white_count++; temp_w >>= 1; }

    for(int i = 0; i < 20; i++){
        if(black_count == 0) return 0.0;
        if(white_count == 0) return 1.0;
        if(rand() % 10 == 0) black_count--;
        if(rand() % 10 == 0) white_count--;
    }

    if(black_count > white_count + 1) return 1.0;
    if(white_count > black_count + 1) return 0.0;
    return 0.5;
}

void expand_node(MCTSNode *node, MemoryPool *pool){
   // node->num_children = 0;
   /* if (node->num_children >= MAX_CHILDREN) return;

    MCTSNode *child = alloc_node(pool);
    if (!child) return;

    child->parent = node;
    child->state = node->state;

    child->move.from = rand() % 64;
    child->move.to = rand() % 64;

    node->children[node->num_children++] = child;*/
    return;
}

uint8_t generate_captures(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    uint64_t current = bb->black | bb->black_k;
    uint64_t opponent = bb->white | bb->white_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;
    uint8_t count = 0;

    for (int bit = 0; bit < 64; bit++) {
        if (!((current >> bit) & 1ULL)) continue;

        int r = bit / 8;
        int c = bit % 8;
        int is_king = (bb->black_k >> bit) &1ULL;

        //direzione di salto
        int dr_list[4] = { (is_king || r < 4) ? 2 : 0, (is_king || r < 4) ? -2 : 0, is_king ? 2 : 0, is_king ? -2 : 0};
        int dc_list[4] = {-2, 2, 2, 2};
        int valid_dirs = is_king ? 2 : 4;

        for (int i = 0; i < valid_dirs; i++) {
        int dr = dr_list[i];
        int dc = dc_list[i];
        if (dr == 0) continue;  

        int mid_r = r + dr/2;
        int mid_c = c + dc/2;
        int to_r = r + dr;
        int to_c = c + dc;

        if (mid_r < 0 || mid_r >= 8 || mid_c < 0 || mid_c >= 8) continue;
        if (to_r < 0 || to_r >= 8 || to_c < 0 || to_c >= 8) continue;

        int mid_bit = mid_r * 8 + mid_c;
        int to_bit = to_r * 8 + to_c;

        if (((opponent >> mid_bit) & 1ULL) && !((occupied >> to_bit) & 1ULL)) {
            if (count < MAX_CHILDREN && pool->top < MAX_NODES) {
                MCTSNode *ch = &pool->nodes[pool->top++];
                memset(ch, 0, sizeof(MCTSNode));
                ch->state = bb;
                ch->move.from = (uint8_t)bit;
                ch->move.to   = (uint8_t)to_bit;
                ch->move.capture = 1;
                children[count++] = ch;
            }
        }
    }
}
    return count;
}

uint8_t generate_legal_moves(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    // 1. Prova a generare catture
    uint8_t cap_count = generate_captures(bb, children, pool);
    
    // REGOLA ITALIANA: Se esiste una cattura, è OBBLIGATORIA.
    if (cap_count > 0) {
        return cap_count;
    }

    // 2. Nessuna cattura? Genera mosse semplici 
    uint64_t current = bb->black | bb->black_k;
    uint64_t occupied = bb->white | bb->black | bb->white_k | bb->black_k;
    uint8_t count = 0;

    for (int bit = 0; bit < 64; bit++) {
        if (!((current >> bit) & 1ULL)) continue;
        int r = bit / 8; int c = bit % 8;
        int dr = (r < 4) ? 1 : -1;
        int dc[2] = {-1, 1};

        for (int i = 0; i < 2; i++) {
            int nr = r + dr; int nc = c + dc[i];
            if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
            if ((nr + nc) % 2 == 0) continue;
            int nbit = nr * 8 + nc;
            if ((occupied >> nbit) & 1ULL) continue;
            if (count < MAX_CHILDREN && pool->top < MAX_NODES) {
                MCTSNode *ch = &pool->nodes[pool->top++];
                memset(ch, 0, sizeof(MCTSNode));
                ch->state = bb;
                ch->move.from = (uint8_t)bit;
                ch->move.to   = (uint8_t)nbit;
                ch->move.capture = 0;
                children[count++] = ch;
            }
        }
    }
    return count;
}

void mcts_search(Bitboard *current_board, float time_limit, MemoryPool *pool) {
    init_pool(pool);
    MCTSNode *root = alloc_node(pool);
    if (!root) return;

    root->state = current_board;
    root->num_children = generate_legal_moves(current_board, root->children, pool);

    if (root->num_children == 0) {
        printf("Nessuna mossa legale. Passo il turno.\n");
        return;
    }

    DWORD start = GetTickCount();
    DWORD limit_ms = (DWORD)(time_limit * 1000);
    int iterations = 0;

    printf("MCTS avviato (limite %.1fs)...\n", time_limit);

    while (1) {
        // 1. Controllo tempo sicuro
        DWORD elapsed = GetTickCount() - start;
        if (elapsed >= limit_ms) break;

        // 2. SELECTION (scende solo se ci sono figli validi)
        MCTSNode *current = root;
        while (current->num_children > 0) {
            MCTSNode *next = select_best_child(current);
            if (!next) break; // Sicurezza anti-NULL
            current = next;
        }

        // 3. SIMULATION (nessuna espansione dinamica per ora)
        double result = simulate_rollout(current->state);

        // 4. BACKPROPAGATION
        MCTSNode *temp = current;
        while (temp != NULL) {
            temp->visits++;
            temp->wins += result;
            result = 1.0 - result; // Inverte prospettiva
            temp = temp->parent;
        }
        iterations++;
    }

    printf("MCTS terminato: %d iterazioni\n", iterations);
}


Move get_best_move(MCTSNode *root, Bitboard *bb) {
    Move null_move = {255, 255, 0};
    if (!root || root->num_children == 0) return null_move;

    MCTSNode *best = root->children[0];
    for (int i = 1; i < root->num_children; i++) {
        if (root->children[i]->visits > best->visits) best = root->children[i];
    }

    // 🔒 Validazione rigorosa
    if (best) {
        int r = best->move.from / 8;
        int c = best->move.from % 8;
        // Verifica che la partenza sia davvero una pedina nera
        if (r < 0 || r >= 8 || c < 0 || c >= 8 || 
            !((bb->black | bb->black_k) >> (r*8+c) & 1ULL)) {
            printf("⚠️ Mossa scartata (partenza invalida). Fallback.\n");
            best = root->children[0];
        }
        return best->move;
    }
    return null_move;
}