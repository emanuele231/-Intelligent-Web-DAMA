#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdbool.h>
#include <raylib.h>
#include "bitboard.h"
#include "moves.h"

// Tipi di algoritmo
typedef enum {
    ALGO_UCB1_CLASSIC,
    ALGO_UCB_DELTA,
    ALGO_UCB_ALPHA,
    ALGO_PUCT_STD,
    ALGO_PUCT_EXP,
    ALGO_PUCT_HEUR,
    ALGO_PUCT_BAL,
    ALGO_UCB_FAST
} AlgoType;

// Configurazione COMPLETA (tutti i campi possibili)
typedef struct {
    // Core
    AlgoType algo;
    float time_limit;
    int max_nodes;
    bool use_heuristics;
    
    // Parametri formule UCB
    float alpha;    
    float delta;    
    float ucb_c;    
    
    // Parametri PUCT
    float cpuct;   
    float puct_c;   
    
    // Rollout
    int rollout_depth;
} AIConfig;

// Interfaccia opaca
typedef struct AI_Instance AI_Instance;
typedef AI_Instance* (*AI_CreateFunc)(const AIConfig* cfg);
typedef Move         (*AI_GetMoveFunc)(AI_Instance* inst, Bitboard* board, float time_budget);
typedef void         (*AI_DestroyFunc)(AI_Instance* inst);

// Definizione motore
typedef struct {
    const char*     id;
    const char*     name;
    const char*     description;
    Color           header_color;
    AIConfig        default_cfg;
    AI_CreateFunc   create;
    AI_GetMoveFunc  get_move;
    AI_DestroyFunc  destroy;
} AIEngineDef;

// Registry API
void                 ai_register(const AIEngineDef* def);
const AIEngineDef*   ai_find(const char* id);
int                  ai_count(void);
const AIEngineDef**  ai_list_all(void);

#endif