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
    node->num_children = 0;
   /* if (node->num_children >= MAX_CHILDREN) return;

    MCTSNode *child = alloc_node(pool);
    if (!child) return;

    child->parent = node;
    child->state = node->state;

    child->move.from = rand() % 64;
    child->move.to = rand() % 64;

    node->children[node->num_children++] = child;*/
}

uint8_t generate_legal_moves(Bitboard *bb, MCTSNode *children[], MemoryPool *pool) {
    uint64_t black_pieces = bb->black | bb->black_k;
    uint64_t occupied = bb->black | bb->black_k | bb->white | bb->white_k;
    uint8_t move_count = 0;

    if (black_pieces == 0) {
        printf(" CRITICO: Mask Nera è ZERO. L'IA non vede le sue pedine!\n");
        return 0;
    }
    for(int bit = 0; bit < 64; bit++) {
        if((black_pieces >> bit) && 1ULL) {
            int fr = bit / 8;
            int fc = bit % 8;

            int dr = (fr < 4) ? 1 : -1;
            int dc_offsets[2] = {-1, 1};

            printf("Pedina nera trovata in Riga:%d Col:%d (bit %d)\n", fr, fc, bit);

            for (int i = 0; i < 2; i++) {
               int nr = fr + dr;
               int nc = fc + dc_offsets[i];

               //confini
                if (nr <= 0 || nr > 8 || nc < 0 || nc >= 8){
                    printf("FUORI SCACCHIERA"); continue;
                    //solo celle scure
                    if((nr + nc) % 2 != 0){
                        printf("non è una casella valida"); continue;
                        //destinazione libera
                        int nbit = nr * 8 + nc;
                        if(((occupied >> bit) & 1ULL)) {
                            printf("OCCUPATA"); continue;

            //se ce spazio tra i figli, creiamo un nodo per la pedina presa
            if(move_count < MAX_CHILDREN && pool->top < MAX_NODES){
                MCTSNode *child = &pool->nodes[pool->top++];
                memset(child, 0, sizeof(MCTSNode));
                child->state = bb;
                child->move.from = (uint8_t)bit;
                child->move.to = (uint8_t)nbit;
                child->move.capture = 0;
                children[move_count++] = child;
                            
                }
            }
        }
    }
}
        }
    }

    if (move_count == 0) {
        printf(" Nessuna pedina nera trovata! L'IA non può muovere.\n");
    } else {
        printf(" L'IA ha identificato %d pedine nere pronte per il calcolo mosse.\n", move_count);
    }

    return move_count;
}

void mcts_search(Bitboard *current_board, float time_limit, MemoryPool *pool) {
    // loop anytime + 4 fasi MCTS
    init_pool(pool);
    MCTSNode *root = alloc_node(pool);
    if(!root) return;

    root->state = current_board;
    root->num_children = generate_legal_moves(current_board, root->children, pool);
    printf("start MTCS search...\n");

    if(root->num_children == 0){
        printf("nessuna mossa legale, partita terminata, PATTA!");
    }

    //1. SELECTION
    clock_t start = clock();
    int iterations = 0;
    do {
        MCTSNode *current = root;
        while (current->num_children > 0) {
            current = select_best_child(current);
            if (!current) break;
        }
    
        
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

        //controllo del tempo esplicito
        float elapsed = (float)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= time_limit) break;
    } while(1);
    
    printf("UCB1 Selection: %d iterazioni in %.3f sec\n", iterations, time_limit);
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
