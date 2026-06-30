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

double play_tournament_game(const AIEngineDef* e1, const AIEngineDef* e2,
                            float time_limit, int max_moves,
                            int* caps1, int* caps2);;
#endif // TOURNAMENT_H