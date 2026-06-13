#include "ai_engine.h"
#include "mcts_core.h"
#include <stdlib.h>

// Wrapper di creazione specifico per UCB1 (opzionale, può usare mcts_create direttamente)
static AI_Instance* ucb1_create(const AIConfig* cfg) {
    AIConfig ucb_cfg = *cfg;
    if (ucb_cfg.ucb_c == 0) ucb_cfg.ucb_c = 1.414f; // Default
    ucb_cfg.algo = ALGO_UCB1_CLASSIC;
    return mcts_create(&ucb_cfg);
}

static Move ucb1_get_move(AI_Instance* inst, Bitboard* board, float time_budget) {
    return mcts_get_move(inst, board, time_budget);
}

static void ucb1_destroy(AI_Instance* inst) {
    mcts_destroy(inst);
}

void register_ucb1_base(void) {
    static AIEngineDef ucb1_def = {
        .id = "ucb1_base",
        .name = "UCB1-0.2 (Base)",
        .description = "UCB1 classico, 0.2s",
        .header_color = {0, 100, 200, 150},
        .default_cfg = {
            .algo = ALGO_UCB1_CLASSIC,
            .ucb_c = 1.414f,
            .time_limit = 0.2f,
            .max_nodes = 40000
        },
        .create = ucb1_create,
        .get_move = ucb1_get_move,
        .destroy = ucb1_destroy
    };
    ai_register(&ucb1_def);
}