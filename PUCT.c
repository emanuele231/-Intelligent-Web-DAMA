#include "ai_engine.h"
#include "mcts_core.h"
#include <stdlib.h>

static AI_Instance* puct_create(const AIConfig* cfg) {
    AIConfig puct_cfg = *cfg;
    if (puct_cfg.cpuct == 0) puct_cfg.cpuct = 1.2f; // Default PUCT
    puct_cfg.algo = ALGO_PUCT_STD;
    return mcts_create(&puct_cfg);
}

static Move puct_get_move(AI_Instance* inst, Bitboard* board, float time_budget) {
    return mcts_get_move(inst, board, time_budget);
}

static void puct_destroy(AI_Instance* inst) {
    mcts_destroy(inst);
}

void register_puct_base(void) {
    static AIEngineDef puct_def = {
        .id = "puct_base",
        .name = "PUCT-0.2 (Base)",
        .description = "PUCT standard, 0.2s",
        .header_color = {200, 100, 0, 150},
        .default_cfg = {
            .algo = ALGO_PUCT_STD,
            .cpuct = 1.2f,
            .time_limit = 0.2f,
            .max_nodes = 40000
        },
        .create = puct_create,
        .get_move = puct_get_move,
        .destroy = puct_destroy
    };
    ai_register(&puct_def);
}