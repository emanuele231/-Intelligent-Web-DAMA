#include "tournament.h"
#include "mcts_core.h"
#include "bitboard.h"
#include "moves.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_board(int board[8][8]) {
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) printf("%d ", board[r][c]);
        printf("\n");
    }
    printf("\n");
}

static void init_tournament_board(int board[8][8]) {
    memset(board, 0, sizeof(int) * 8 * 8);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) {
                if (r <= 2) board[r][c] = 2;
                else if (r >= 5) board[r][c] = 1;
            }
        }
    }
}

double play_tournament_game(const AIEngineDef* e1, const AIEngineDef* e2,
                            float time_limit, int max_moves,
                            int* pts1, int* pts2) {
    *pts1 = 0; *pts2 = 0;
    
    int board[8][8];
    init_tournament_board(board);
    
    printf("\n========== NUOVA PARTITA: %s vs %s ==========\n", e1->name, e2->name);
    printf("Tavola Iniziale:\n");
    print_board(board);
    
    AIConfig c1 = e1->default_cfg, c2 = e2->default_cfg;
    c1.time_limit = time_limit; c2.time_limit = time_limit;
    
    AI_Instance* a1 = e1->create(&c1);
    AI_Instance* a2 = e2->create(&c2);
    if (!a1 || !a2) { printf("ERRORE: Creazione IA fallita\n"); return 0.5; }
    
    bool turn = true; // true = e1 (Bianco), false = e2 (Nero)
    int moves = 0;
    bool w_has = true, b_has = true;
    
    while (moves < max_moves) {
        Bitboard bb;
        board_to_bitboard(board, &bb);
        bb.turn = turn ? 1 : 2; // 1 = Bianco, 2 = Nero
        
        printf("Turno %d (%s). Mosse totali: %d\n", moves+1, turn ? "BIANCO" : "NERO", moves);
        
        Move m = turn ? e1->get_move(a1, &bb, time_limit) 
                      : e2->get_move(a2, &bb, time_limit);
        
        if (m.from == 255 || m.to == 255) {
            printf(">> Mossa INVALIDA (255) da %s. Partita terminata.\n", turn ? "BIANCO" : "NERO");
            break;
        }
        
        int fr = m.from / 8, fc = m.from % 8;
        int tr = m.to / 8, tc = m.to % 8;
        printf(">> Mossa: (%d,%d) -> (%d,%d) | Pezzo muove: %d\n", fr, fc, tr, tc, board[fr][fc]);
        
        apply_ai_move(board, fr, fc, tr, tc);
        
        if (moves % 5 == 0 || moves == 1) {
            printf("Stato tavola dopo mossa %d:\n", moves+1);
            print_board(board);
        }
        
        w_has = false; b_has = false;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1 || board[r][c] == 3) w_has = true;
                if (board[r][c] == 2 || board[r][c] == 4) b_has = true;
            }
        }
        if (!w_has || !b_has) {
            printf(">> SFINIMENTO! %s ha perso tutti i pezzi.\n", !w_has ? "BIANCO" : "NERO");
            break;
        }
        
        turn = !turn;
        moves++;
    }
    
    w_has = false; b_has = false;
    int w_rem = 0, b_rem = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == 1 || board[r][c] == 3) { w_has = true; w_rem++; }
            if (board[r][c] == 2 || board[r][c] == 4) { b_has = true; b_rem++; }
        }
    }
    
    *pts1 = 12 - b_rem;
    *pts2 = 12 - w_rem;
    
    printf("========== FINE PARTITA ==========\n");
    printf("Bianchi rimasti: %d | Neri rimasti: %d\n", w_rem, b_rem);
    printf("Punti finali: %s(%d) - %s(%d)\n\n", e1->name, *pts1, e2->name, *pts2);
    
    if (!w_has) return 0.0;
    if (!b_has) return 1.0;
    if (*pts1 > *pts2) return 1.0;
    if (*pts2 > *pts1) return 0.0;
    return 0.5;
}