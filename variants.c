#include "ai_engine.h"
#include "mcts_core.h"

static void reg(const char* id, const char* name, Color col, AIConfig cfg) {
    static AIEngineDef defs[16];
    static int n = 0;
    if (n >= 16) return;
    defs[n] = (AIEngineDef){
        .id = id, .name = name, .description = name, .header_color = col,
        .default_cfg = cfg,
        .create = mcts_create, .get_move = mcts_get_move, .destroy = mcts_destroy
    };
    ai_register(&defs[n++]);
}

void register_all_variants(void) {
    // UCB (4)
    reg("ucb1_classic", "UCB1 Classic (√2)",   (Color){0,100,200,150}, (AIConfig){ALGO_UCB1_CLASSIC, 0.2f, 30000, false, 0,0,1.414f, 0,0,0});
    reg("ucb_delta",    "UCB Delta (δ=0.01)",  (Color){0,150,255,150}, (AIConfig){ALGO_UCB_DELTA,    0.2f, 30000, false, 0,0.01f,0, 0,0,0});
    reg("ucb_alpha",    "UCB Alpha (α=1.5)",   (Color){50,200,255,150}, (AIConfig){ALGO_UCB_ALPHA,    1.0f, 50000, false, 1.5f,0,0, 0,0,0});
    reg("ucb_fast",     "UCB Fast (0.2s)",     (Color){100,220,255,150}, (AIConfig){ALGO_UCB_FAST,    0.2f, 20000, true,  0,0,1.414f, 0,0,20});
    
    // PUCT (4)
    reg("puct_std",     "PUCT Standard (c=1.0)", (Color){200,100,0,150}, (AIConfig){ALGO_PUCT_STD,  0.2f, 30000, false, 0,0,0, 1.0f,0,0});
    reg("puct_exp",     "PUCT Explorative (c=2.0)", (Color){255,120,0,150}, (AIConfig){ALGO_PUCT_EXP,  1.0f, 60000, false, 0,0,0, 2.0f,0,0});
    reg("puct_heur",    "PUCT Heuristic",      (Color){255,150,50,150}, (AIConfig){ALGO_PUCT_HEUR, 1.0f, 50000, true,  0,0,0, 1.5f,0,25});
    reg("puct_bal",     "PUCT Balanced (c=1.2)", (Color){255,180,100,150}, (AIConfig){ALGO_PUCT_BAL,  3.0f, 80000, false, 0,0,0, 1.2f,0,0});
}