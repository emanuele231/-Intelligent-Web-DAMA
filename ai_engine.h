#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdbool.h>
#include <raylib.h>

// 1. Includi PRIMA i tipi base per garantire che Move/Bitboard siano noti
#include "bitboard.h"
#include "moves.h"

// 2. Dichiarazione opaca per l'istanza AI
typedef struct AI_Instance AI_Instance;

// 3. Configurazione
typedef struct {
    float ucb_c;
    float puct_c;
    float time_limit;
    int   max_nodes;
    int   rollout_depth;
    bool  use_heuristics;
} AIConfig;

// 4. Puntatori a funzione (ora Move è sicuramente definito)
typedef AI_Instance* (*AI_CreateFunc)(const AIConfig* cfg);
typedef Move         (*AI_GetMoveFunc)(AI_Instance* inst, Bitboard* board, float time_budget);
typedef void         (*AI_DestroyFunc)(AI_Instance* inst);

// 5. Definizione motore
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

// 6. API Registry
void                 ai_register(const AIEngineDef* def);
const AIEngineDef*   ai_find(const char* id);
int                  ai_count(void);
const AIEngineDef**  ai_list_all(void);

#endif // AI_ENGINE_H