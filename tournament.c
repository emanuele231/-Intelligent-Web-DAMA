#include "tournament.h"
#include "mcts_core.h"
#include "bitboard.h"
#include "moves.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Inizializza board standard
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

// Conta quanti pezzi ci sono attualmente sulla board
static int count_pieces_on_board(int board[8][8]) {
    int count = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] != 0) count++;
        }
    }
    return count;
}

// Gioca una singola partita e restituisce vittorie/catture
double play_tournament_game(const AIEngineDef* e1, const AIEngineDef* e2, 
                            float time_limit, int max_moves,
                            int* caps1, int* caps2) {
    *caps1 = 0; 
    *caps2 = 0; 
    
    int board[8][8];
    init_tournament_board(board);
    
    // Crea istanze IA
    AIConfig c1 = e1->default_cfg, c2 = e2->default_cfg;
    c1.time_limit = time_limit; 
    c2.time_limit = time_limit;
    
    AI_Instance* a1 = e1->create(&c1);
    AI_Instance* a2 = e2->create(&c2);
    if (!a1 || !a2) { fprintf(stderr, "Errore creazione IA\n"); return 0.5; }
    
    bool turn = true; // true = e1 (Bianco), false = e2 (Nero)
    int moves = 0;
    
    printf("🏁 PARTITA: %s (Bianco) vs %s (Nero)\n", e1->name, e2->name);
    
    while (moves < max_moves) {
        Bitboard bb;
        board_to_bitboard(board, &bb);
        
        // Conta pezzi PRIMA della mossa
        int pieces_before = count_pieces_on_board(board);
        
        Move m = turn ? e1->get_move(a1, &bb, time_limit) 
                      : e2->get_move(a2, &bb, time_limit);
        
        // Mossa invalida -> sconfitta immediata
        if (m.from == 255 || m.to == 255) {
            printf("⚠️ Mossa INVALIDA da %s! Vittoria assegnata a %s.\n", 
                   turn ? e1->name : e2->name, turn ? e2->name : e1->name);
            e1->destroy(a1); e2->destroy(a2);
            return turn ? 0.0 : 1.0;
        }
        
        // Applica mossa sulla board
        apply_ai_move(board, m.from/8, m.from%8, m.to/8, m.to%8);
        check_promotion(board, m.to/8, m.to%8);
        
        // Conta pezzi DOPO la mossa
        int pieces_after = count_pieces_on_board(board);
        
        //  Se i pezzi sono diminuiti, c'è stata una cattura
        if (pieces_after < pieces_before) {
            int captured = pieces_before - pieces_after;
            if (turn) *caps1 += captured;
            else      *caps2 += captured;
            
            printf(" CATTURA! %s ha eliminato %d pezzo/i. "
                   "Totale partita: %s=%d | %s=%d\n",
                   turn ? e1->name : e2->name, captured,
                   e1->name, *caps1, e2->name, *caps2);
        }
        
        // Controllo fine partita (pedine finite)
        bool white_has = false, black_has = false;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1 || board[r][c] == 3) white_has = true;
                if (board[r][c] == 2 || board[r][c] == 4) black_has = true;
            }
        }
        
        if (!white_has) {
            printf(" %s vince per sfinimento!\n", e2->name);
            e1->destroy(a1); e2->destroy(a2);
            return 0.0;
        }
        if (!black_has) {
            printf(" %s vince per sfinimento!\n", e1->name);
            e1->destroy(a1); e2->destroy(a2);
            return 1.0;
        }
        
        turn = !turn;
        moves++;
    }
    
    // Patta per limite mosse
    printf("Patta per limite mosse (%d)\n", max_moves);
    e1->destroy(a1); e2->destroy(a2);
    return 0.5;
}