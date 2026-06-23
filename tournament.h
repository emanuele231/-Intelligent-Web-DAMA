#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "ai_engine.h"

typedef struct {
    const char* engine_name;
    int wins;
    int losses;
    int draws;
    double avg_time_per_move;
} TournamentResult;

double play_tournament_game(const AIEngineDef* engine_white, 
                            const AIEngineDef* engine_black, 
                            float time_limit, 
                            int max_moves);

void run_tournament_headless(const AIEngineDef** engines, int num_engines, 
                             int games_per_pair, float time_limit,
                             TournamentResult* results);

void export_tournament_csv(TournamentResult* results, int count, const char* filename);

#endif // TOURNAMENT_H