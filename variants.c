#include "ai_engine.h"
#include "mcts_core.h"

// Helper per registrare un'IA
static void reg(const char* id, const char* name, Color col, AIConfig cfg) {
    static AIEngineDef defs[16];
    static int n = 0;
    if (n >= 16) return;
    
    defs[n] = (AIEngineDef){
        .id = id,
        .name = name,
        .description = name,
        .header_color = col,
        .default_cfg = cfg,
        .create = mcts_create,
        .get_move = mcts_get_move,
        .destroy = mcts_destroy
    };
    
    ai_register(&defs[n]);
    n++;
}

void register_all_variants(void) {
    // ==========================================
    // UCB1 VARIANTS (Indici 0-3)
    // ==========================================
    
    // 0. Classic: Standard sqrt(2)
    reg("ucb1_classic", "UCB1 Classic", (Color){0, 100, 200, 150}, 
        (AIConfig){ALGO_UCB1_CLASSIC, 0.2f, 30000, false, 0, 0, 1.414f, 0, 0, 0});

    // 1. Delta: Confidence bound variant
    reg("ucb_delta", "UCB Delta", (Color){20, 120, 220, 150}, 
        (AIConfig){ALGO_UCB_DELTA, 0.2f, 30000, false, 0, 0.01f, 0, 0, 0, 0});

    // 2. Alpha: Higher exploration weight
    reg("ucb_alpha", "UCB Alpha", (Color){40, 140, 240, 150}, 
        (AIConfig){ALGO_UCB_ALPHA, 0.2f, 30000, false, 1.5f, 0, 0, 0, 0, 0});

    // 3. Fast: Optimized for quick response
    reg("ucb_fast", "UCB Fast", (Color){60, 160, 255, 150}, 
        (AIConfig){ALGO_UCB_FAST, 0.2f, 20000, true, 0, 0, 1.414f, 0, 0, 20});

    // ==========================================
    // PUCT VARIANTS (Indici 4-7)
    // ==========================================

    // 4. Standard: AlphaGo style base
    reg("puct_std", "PUCT Standard", (Color){200, 100, 0, 150}, 
        (AIConfig){ALGO_PUCT_STD, 0.2f, 30000, false, 0, 0, 0, 1.2f, 0, 0});

    // 5. Explorative: High cpuct encourages trying new moves
    reg("puct_exp", "PUCT Explorative", (Color){220, 120, 20, 150}, 
        (AIConfig){ALGO_PUCT_EXP, 0.2f, 40000, false, 0, 0, 0, 2.5f, 0, 0});

    // 6. Heuristic: Uses domain knowledge in rollout
    reg("puct_heur", "PUCT Heuristic", (Color){240, 140, 40, 150}, 
        (AIConfig){ALGO_PUCT_HEUR, 0.2f, 30000, true, 0, 0, 0, 1.5f, 0, 25});

    // 7. Balanced: Slower but deeper search
    reg("puct_bal", "PUCT Balanced", (Color){255, 160, 60, 150}, 
        (AIConfig){ALGO_PUCT_BAL, 3.0f, 80000, false, 0, 0, 0, 1.2f, 0, 0});
}