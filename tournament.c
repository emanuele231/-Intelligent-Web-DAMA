#include "ai_engine.h"
#include "bitboard.h"
#include "moves.h"
#include "params.h"
#include "tournament.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

// Gioca una singola partita
double play_tournament_game(const AIEngineDef* engine_white, 
                            const AIEngineDef* engine_black, 
                            float time_limit, 
                            int max_moves) {
    int board[8][8];
    init_tournament_board(board);
    
    // Crea istanze
    AIConfig cfg_w = engine_white->default_cfg;
    AIConfig cfg_b = engine_black->default_cfg;
    cfg_w.time_limit = time_limit;
    cfg_b.time_limit = time_limit;
    
    AI_Instance* ai_white = engine_white->create(&cfg_w);
    AI_Instance* ai_black = engine_black->create(&cfg_b);
    
    if (!ai_white || !ai_black) {
        fprintf(stderr, "Errore creazione IA\n");
        return 0.5;
    }
    
    bool white_turn = true;
    int move_count = 0;
    
    while (move_count < max_moves) {
        Bitboard bb;
        board_to_bitboard(board, &bb);
        
        Move move;
        if (white_turn) {
            move = engine_white->get_move(ai_white, &bb, time_limit);
        } else {
            move = engine_black->get_move(ai_black, &bb, time_limit);
        }
        
        // Mossa invalida -> sconfitta
        if (move.from == 255 || move.to == 255) {
            engine_white->destroy(ai_white);
            engine_black->destroy(ai_black);
            return white_turn ? 0.0 : 1.0;
        }
        
        // Applica mossa
        apply_ai_move(board, move.from/8, move.from%8, move.to/8, move.to%8);
        check_promotion(board, move.to/8, move.to%8);
        
        // Controllo fine partita (pedine finite)
        bool white_has = false, black_has = false;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1 || board[r][c] == 3) white_has = true;
                if (board[r][c] == 2 || board[r][c] == 4) black_has = true;
            }
        }
        
        if (!white_has) {
            engine_white->destroy(ai_white);
            engine_black->destroy(ai_black);
            return 0.0;  // Vince nero
        }
        if (!black_has) {
            engine_white->destroy(ai_white);
            engine_black->destroy(ai_black);
            return 1.0;  // Vince bianco
        }
        
        white_turn = !white_turn;
        move_count++;
    }
    
    // Patta per limite mosse
    engine_white->destroy(ai_white);
    engine_black->destroy(ai_black);
    return 0.5;
}

//applico il round robin per eseguire il torneo
void run_tournament_headless(const AIEngineDef** engines, int num_engines, 
                             int games_per_pair, float time_limit,
                             TournamentResult* results) {
    printf("\n=== INIZIO TORNEO ===\n");
    printf("Partite per coppia: %d\n", games_per_pair);
    printf("Tempo per mossa: %.1fs\n\n", time_limit);
    
    int total_matches = (num_engines * (num_engines - 1)) / 2 * games_per_pair;
    int match_count = 0;
    
    // Round-robin
    for (int i = 0; i < num_engines; i++) {
        for (int j = i + 1; j < num_engines; j++) {
            int wins_i = 0, wins_j = 0, draws = 0;
            
            printf("[%s] vs [%s]\n", engines[i]->name, engines[j]->name);
            
            for (int g = 0; g < games_per_pair; g++) {
                double result = play_tournament_game(engines[i], engines[j], time_limit, 60);
                
                if (result > 0.9) wins_i++;
                else if (result < 0.1) wins_j++;
                else draws++;
                
                match_count++;
                
                // Aggiorna stato per UI (se necessario)
                // Questo potrebbe essere passato come callback
            }
            
            results[i].wins += wins_i;
            results[i].losses += wins_j;
            results[i].draws += draws;
            
            results[j].wins += wins_j;
            results[j].losses += wins_i;
            results[j].draws += draws;
            
            printf("  Risultato: %d-%d (%d patte)\n\n", wins_i, wins_j, draws);
        }
    }
    
    // Stampa classifica
    printf("\n=== CLASSIFICA FINALE ===\n");
    printf("%-25s | %6s | %6s | %6s | %8s\n", "IA", "V", "P", "S", "WinRate");
    printf("----------------------------------------------------------\n");
    
    for (int i = 0; i < num_engines; i++) {
        int total = results[i].wins + results[i].losses + results[i].draws;
        float rate = total > 0 ? (float)results[i].wins / total * 100 : 0;
        printf("%-25s | %6d | %6d | %6d | %7.1f%%\n", 
               results[i].engine_name, 
               results[i].wins, 
               results[i].draws, 
               results[i].losses, 
               rate);
    }
}

//esportazione risultati
void export_tournament_csv(TournamentResult* results, int count, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Errore apertura file %s\n", filename);
        return;
    }
    
    fprintf(f, "Rank,IA,Wins,Draws,Losses,Total,WinRate\n");
    
    // Ordina per vittorie
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (results[j].wins < results[j+1].wins) {
                TournamentResult tmp = results[j];
                results[j] = results[j+1];
                results[j+1] = tmp;
            }
        }
    }
    
    for (int i = 0; i < count; i++) {
        int total = results[i].wins + results[i].losses + results[i].draws;
        float rate = total > 0 ? (float)results[i].wins / total * 100 : 0;
        
        fprintf(f, "%d,%s,%d,%d,%d,%d,%.2f\n",
                i + 1,
                results[i].engine_name,
                results[i].wins,
                results[i].draws,
                results[i].losses,
                total,
                rate);
    }
    
    fclose(f);
    printf("Risultati esportati in %s\n", filename);
}