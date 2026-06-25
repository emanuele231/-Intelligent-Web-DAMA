#include "tournament.h"
#include "mcts_core.h"
#include "bitboard.h"
#include "moves.h"
#include <stdio.h>
#include <stdlib.h>

static void init_tournament_board(int board[8][8]) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) {
                if (r >= 5) board[r][c] = 1;
                else if (r <= 2) board[r][c] = 2;
                else board[r][c] = 0;
            } else {
                board[r][c] = 0;
            }
        }
    }
}

// tournament.c (implementazione)
double play_tournament_game(const AIEngineDef* e1, const AIEngineDef* e2,
                            float time_limit, int max_moves,
                            int* caps1, int* caps2) {
    *caps1 = 0; *caps2 = 0; // Reset contatori
    
    int board[8][8];
    init_tournament_board(board);
    
    AIConfig c1 = e1->default_cfg, c2 = e2->default_cfg;
    c1.time_limit = time_limit; c2.time_limit = time_limit;
    
    AI_Instance* a1 = e1->create(&c1);
    AI_Instance* a2 = e2->create(&c2);
    if (!a1 || !a2) { fprintf(stderr, "Errore creazione IA\n"); return 0.5; }
    
    bool turn = true; // true = e1 (Bianco)
    int moves = 0;
    
    while (moves < max_moves) {
        Bitboard bb;
        board_to_bitboard(board, &bb);
        
        Move m = turn ? e1->get_move(a1, &bb, time_limit) 
                      : e2->get_move(a2, &bb, time_limit);
        
        if (m.from == 255 || m.to == 255) {
            e1->destroy(a1); e2->destroy(a2);
            return turn ? 0.0 : 1.0;
        }
        
        // Traccia cattura
        if (m.capture) {
            if (turn) (*caps1)++; else (*caps2)++;
        }
        
        apply_ai_move(board, m.from/8, m.from%8, m.to/8, m.to%8);
        check_promotion(board, m.to/8, m.to%8);
        
        bool w_has = false, b_has = false;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1 || board[r][c] == 3) w_has = true;
                if (board[r][c] == 2 || board[r][c] == 4) b_has = true;
            }
        }
        if (!w_has) { e1->destroy(a1); e2->destroy(a2); return 0.0; }
        if (!b_has) { e1->destroy(a1); e2->destroy(a2); return 1.0; }
        
        turn = !turn; moves++;
    }
    
    e1->destroy(a1); e2->destroy(a2);
    return 0.5;
}