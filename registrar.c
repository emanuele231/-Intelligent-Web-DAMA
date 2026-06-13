#include "ai_engine.h"
#include "UCB1.h"
#include "PUCT.h"
#include "params.h"

// Macro per registrare un motore con parametri specifici
#define REGISTER_IA(VARIANT_NAME, TIME_LIMIT, PUCT_C, USE_HEURISTICS) \
    static AIEngineDef VARIANT_NAME##_engine = { \
        .name = #VARIANT_NAME, \
        .description = "IA " #VARIANT_NAME " (Time: " #TIME_LIMIT "s)", \
        .default_cfg = { \
            .time_limit = TIME_LIMIT, \
            .puct_c = PUCT_C, \
            .use_heuristics = USE_HEURISTICS, \
            .max_nodes = 30000 \
        }, \
        .create = (VARIANT_NAME##_create), \
        .destroy = (VARIANT_NAME##_destroy), \
        .get_move = (VARIANT_NAME##_get_move) \
    }; \
    register_engine(&VARIANT_NAME##_engine);

// Registrazione automatica di tutte le IA
void register_all_ia(void) {
    // UCB1 variants (base)
    REGISTER_IA(ucb1_fast, 0.2f, 0.0f, false)
    REGISTER_IA(ucb1_medium, 1.0f, 0.0f, false)
    REGISTER_IA(ucb1_strong, 3.0f, 0.0f, false)
    
    // PUCT variants (base)
    REGISTER_IA(puct_fast, 0.2f, 1.2f, false)
    REGISTER_IA(puct_medium, 1.0f, 1.2f, false)
    REGISTER_IA(puct_strong, 3.0f, 1.2f, false)
    
    // Varianti avanzate
    REGISTER_IA(ucb1_tuned, 0.2f, 0.0f, true)
    REGISTER_IA(puct_heuristic, 0.2f, 1.2f, true)
}

// Funzione di inizializzazione (chiamata da main.c)
void init_ai_engines(void) {
    register_all_ia();
}