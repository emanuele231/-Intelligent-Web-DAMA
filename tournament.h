#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "ai_engine.h"
#include "params.h"
#include <stdbool.h>

// Struttura aggregata per i risultati
typedef struct {
    const char* engine_id;
    const char* engine_name;
    int wins;
    int losses;
    int draws;
    double avg_time_per_move; // secondi
} TournamentResult;

/**
 * Avvia un torneo round-robin tra tutte le IA registrate nel sistema.
 * Legge parametri da config_file (num_games, time_limit, max_moves, ecc.)
 * e stampa una classifica ordinata per score (W + 0.5*D).
 */
int run_tournament(const char* config_file, const char* output_csv);

/**
 * Gioca una singola partita headless tra due motori.
 * @return 1.0 se vince engine1 (Bianco), 0.0 se vince engine2 (Nero), 0.5 in caso di patta
 */
double play_tournament_game(const AIEngineDef* engine_white, 
                            const AIEngineDef* engine_black, 
                            float time_limit, 
                            int max_moves);

#endif // TOURNAMENT_H