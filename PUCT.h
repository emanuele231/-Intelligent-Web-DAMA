#ifndef AI_PUCT_H
#define AI_PUCT_H

#include "ai_engine.h"
#include "mcts_core.h"
#include "bitboard.h"
#include "moves.h"

/*
 * PUCT AI Engine Module
 * ---------------------
 * MCTS basato su PUCT (Polynomial Upper Confidence Trees).
 * Utilizza una costante di esplorazione (cpuct) per bilanciare 
 * valore medio della mossa e frequanza di visita.
 *
 * ID motore: "puct_base"
 * Nome menu: "PUCT-0.2 (Base)"
 */

// Registrazione esplicita (compatibile MSVC/GCC)
void register_puct_base(void);

#endif // AI_PUCT_H