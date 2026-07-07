#include "tournament.h"
#include "mcts_core.h"
#include "bitboard.h"
#include "moves.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOTAL_MOVES 400    // Limite massimo mosse totali (regola ufficiale)
#define MAX_IDLE_MOVES 40      // Mosse senza catture prima della patta
#define MAX_MOVES_PER_PLAYER 200  // Limite per singolo giocatore

static void init_tournament_board(int board[8][8]) {
    memset(board, 0, sizeof(int) * 8 * 8);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) {
                if (r <= 2) board[r][c] = 2;  // Nero
                else if (r >= 5) board[r][c] = 1;  // Bianco
            }
        }
    }
}

// Conta pezzi rimanenti
static void count_pieces(int board[8][8], int *white_count, int *black_count) {
    *white_count = 0;
    *black_count = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == 1 || board[r][c] == 3) (*white_count)++;
            if (board[r][c] == 2 || board[r][c] == 4) (*black_count)++;
        }
    }
}

// Calcola il punteggio finale (pedine mancanti all'avversario)
static void calculate_points(int board[8][8], int *pts1, int *pts2) {
    int w_rem = 0, b_rem = 0;
    count_pieces(board, &w_rem, &b_rem);
    *pts1 = 12 - b_rem;  // Punti del Bianco (P1)
    *pts2 = 12 - w_rem;  // Punti del Nero (P2)
}

double play_tournament_game(const AIEngineDef* e1, const AIEngineDef* e2,
                            float time_limit, int max_moves,
                            int* pts1, int* pts2) {
    *pts1 = 0; *pts2 = 0;
    
    int board[8][8];
    init_tournament_board(board);
    
    AIConfig c1 = e1->default_cfg, c2 = e2->default_cfg;
    c1.time_limit = time_limit; c2.time_limit = time_limit;
    
    AI_Instance* a1 = e1->create(&c1);
    AI_Instance* a2 = e2->create(&c2);
    if (!a1 || !a2) return 0.5;
    
    bool turn = true;  // true = Bianco (e1), false = Nero (e2)
    int total_moves = 0;
    int moves_without_capture = 0;  // Contatore stallo
    int w_rem = 12, b_rem = 12;
    int prev_w = 12, prev_b = 12;
    
    printf("\n--- PARTITA: %s (B) vs %s (N) ---\n", e1->name, e2->name);
    
    while (total_moves < MAX_TOTAL_MOVES) {
        // Aggiorna conteggio pezzi
        count_pieces(board, &w_rem, &b_rem);
        
        // 1. VITTORIA PER CATTURA TOTALE
        if (w_rem == 0) {
            printf("   >> Nero vince per cattura totale (Bianco senza pezzi)\n");
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return 0.0;  // Vince Nero
        }
        if (b_rem == 0) {
            printf("   >> Bianco vince per cattura totale (Nero senza pezzi)\n");
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return 1.0;  // Vince Bianco
        }
        
        // 2. VITTORIA PER BLOCCO TOTALE
        int current_player = turn ? 1 : 2;
        if (!has_legal_moves(board, current_player)) {
            printf("   >> %s vince per BLOCCO TOTALE (%s non ha mosse legali)\n",
                   turn ? e2->name : e1->name,
                   turn ? "Bianco" : "Nero");
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return turn ? 0.0 : 1.0;
        }
        
        // 3. Patta per stallo (40 mosse senza catture)
        if (moves_without_capture >= MAX_IDLE_MOVES) {
            printf("   >> PATTA per stallo (%d mosse senza catture)\n", moves_without_capture);
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return 0.5;
        }
        
        // 4. Patta per limite mosse totali (400)
        if (total_moves >= MAX_TOTAL_MOVES) {
            printf("   >> PATTA per limite mosse (%d)\n", total_moves);
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return 0.5;
        }
        
        // Esegui mossa
        Bitboard bb;
        board_to_bitboard(board, &bb);
        bb.turn = turn ? 1 : 2;
        
        Move m = turn ? e1->get_move(a1, &bb, time_limit) 
                      : e2->get_move(a2, &bb, time_limit);
        
        if (m.from == 255 || m.to == 255) {
            printf("   >> Mossa invalida da %s. Vittoria a %s\n",
                   turn ? "Bianco" : "Nero",
                   turn ? e2->name : e1->name);
            calculate_points(board, pts1, pts2);
            e1->destroy(a1); e2->destroy(a2);
            return turn ? 0.0 : 1.0;
        }
        
        int fromR = m.from / 8, fromC = m.from % 8;
        int toR = m.to / 8, toC = m.to % 8;
        
        // Rileva se è una cattura
        bool is_capture = (abs(toR - fromR) == 2 && abs(toC - fromC) == 2);
        
        // Applica mossa
        apply_ai_move(board, fromR, fromC, toR, toC);
        
        // Aggiorna contatore stallo
        if (is_capture) {
            moves_without_capture = 0;
        } else {
            moves_without_capture++;
        }
        
        total_moves++;
        turn = !turn;
    }
    
    // Se usciamo dal ciclo, è patta per limite mosse
    calculate_points(board, pts1, pts2);
    e1->destroy(a1); e2->destroy(a2);
    printf("   >> PATTA (limite mosse raggiunto)\n");
    return 0.5;
}