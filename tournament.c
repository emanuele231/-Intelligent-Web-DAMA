#include "ai_engine.h"
#include "bitboard.h"
#include "moves.h"
#include "params.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Struttura per i risultati
typedef struct {
    const char* engine_name;
    int wins;
    int losses;
    int draws;
} TournamentResult;

// ============================================================================
// HELPER LOCALI (Per evitare dipendenze da funzioni non esposte in moves.h)
// ============================================================================

static void tournament_init_board(int board[8][8]) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) {
                if (r >= 5) board[r][c] = 1;      // Bianco
                else if (r <= 2) board[r][c] = 2; // Nero
                else board[r][c] = 0;
            } else {
                board[r][c] = 0;
            }
        }
    }
}

// Controllo semplice: il giocatore ha ancora pedine?
static int has_pieces(int board[8][8], int player_val) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board[r][c] == player_val) return 1;
    return 0;
}

// ============================================================================
// LOGICA DI GIOCO
// ============================================================================

static int play_game(const AIEngineDef* engine1, const AIEngineDef* engine2, float time_limit) {
    int board[8][8] = {0};
    tournament_init_board(board);
    
    // Config
    AIConfig cfg1 = engine1->default_cfg;
    AIConfig cfg2 = engine2->default_cfg;
    cfg1.time_limit = time_limit;
    cfg2.time_limit = time_limit;
    
    // Creazione istanze
    AI_Instance* ai1 = engine1->create(&cfg1);
    AI_Instance* ai2 = engine2->create(&cfg2);
    
    if (!ai1 || !ai2) return -2;
    
    bool is_player_turn = true; // 1 (Bianco) inizia
    int move_count = 0;
    
    // Limite massimo mosse per evitare loop infiniti
    int max_moves = 60; 
    
    while (move_count < max_moves) {
        Bitboard bb;
        board_to_bitboard(board, &bb);
        
        // Calcolo mossa
        Move move;
        if (is_player_turn) {
            move = engine1->get_move(ai1, &bb, time_limit);
        } else {
            move = engine2->get_move(ai2, &bb, time_limit);
        }
        
        // Validazione minima mossa
        if (move.from == 255 || move.to == 255) {
            // IA ha passato/abbandonato -> perde
            return is_player_turn ? -1 : 1; 
        }
        
        // Applica mossa (Assumendo che apply_ai_move sia disponibile o implementala qui)
        apply_ai_move(board, move.from/8, move.from%8, move.to/8, move.to%8);
        check_promotion(board, move.to/8, move.to%8);
        
        // Controllo fine partita (pedine finite)
        if (!has_pieces(board, 1)) return -1; // Nero vince
        if (!has_pieces(board, 2)) return 1;  // Bianco vince
        
        // Cambio turno
        is_player_turn = !is_player_turn;
        move_count++;
    }
    
    return 0; // Patta per limite mosse
}

// ============================================================================
// TORNEO
// ============================================================================

void run_tournament(const char* config_file) {
    params_load(config_file);
    float time_limit = params_get_float("time_limit_per_move", 0.1f);
    int num_games = params_get_int("num_games_per_pair", 20);
    
    int total_engines = ai_count();
    if (total_engines < 2) {
        fprintf(stderr, "Errore: Servono almeno 2 IA registrate.\n");
        return;
    }
    
    // Inizializza risultati
    TournamentResult results[16]; // Assumiamo max 16 IA
    for(int i=0; i<total_engines; i++) {
        results[i].engine_name = ai_list_all()[i]->name;
        results[i].wins = results[i].losses = results[i].draws = 0;
    }
    
    printf("\n=== AVVIO TORNEO (%d IA) ===\n", total_engines);
    
    // Round Robin
    for (int i = 0; i < total_engines; i++) {
        for (int j = i + 1; j < total_engines; j++) {
            int wins_i = 0, wins_j = 0, draws = 0;
            
            printf("Match: %s vs %s ... ", ai_list_all()[i]->name, ai_list_all()[j]->name);
            
            for (int k = 0; k < num_games; k++) {
                int res = play_game(ai_list_all()[i], ai_list_all()[j], time_limit);
                if (res == 1) wins_i++;
                else if (res == -1) wins_j++;
                else draws++;
            }
            
            results[i].wins += wins_i;
            results[i].losses += wins_j;
            results[i].draws += draws;
            
            results[j].wins += wins_j;
            results[j].losses += wins_i;
            results[j].draws += draws;
            
            printf("%d - %d (%d patta)\n", wins_i, wins_j, draws);
        }
    }
    
    // Classifica
    printf("\n=== CLASSIFICA ===\n");
    for (int i = 0; i < total_engines; i++) {
        int total = results[i].wins + results[i].losses + results[i].draws;
        float pct = total > 0 ? (float)results[i].wins / total * 100 : 0;
        printf("%-20s | V: %2d | P: %2d | S: %2d | WinRate: %.1f%%\n", 
               results[i].engine_name,
               results[i].wins, results[i].draws, results[i].losses, pct);
    }
}