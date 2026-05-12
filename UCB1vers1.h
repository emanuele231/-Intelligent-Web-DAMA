#ifndef UCB1VER1_H
#define UCB1VER1_H

#include <stdint.h>
#include <stdbool.h>

#define UCB1_C 1.414f      //costante di esplorazione
#define TIME_LIMIT_DEFAULT 0.2f   //limite base per la prima versione
#define MAX_NODES 40000     //numero massimo di iterazioni
#define MAX_CHILDREN 8   //numero mosse legali massime

//mossa compatta
typedef struct {
    uint8_t from;
    uint8_t to;
    uint8_t capture;
} Move;

typedef struct Bitboard Bitboard;

typedef struct MTCSNode {
    Bitboard *state;  //puntatore allo stato (senza clonare il roll-out)
    struct MTCSNode *parent;  //padre della backprop
    struct MTCSNode *children[MAX_CHILDREN];
    uint32_t visits;     //contatore visite
    double wins;     //accumulo dei risultati
    Move move;     //mossa (del nodo)
    uint8_t num_children;     //figli attuali
} MTCSNode;

typedef struct {
    MTCSNode nodes[MAX_NODES];
    int top;  //prossimo nodo libero
} MemoryPool; 

void init_pool(MemoryPool *pool);
void mtcs_search(Bitboard *root_state, float time_limit, MemoryPool *pool);
Move get_best_move(MTCSNode *root);
double ucb1_score(MTCSNode *node, double parent_visits);
MTCSNode* select_best_child(MTCSNode *parent);

#endif