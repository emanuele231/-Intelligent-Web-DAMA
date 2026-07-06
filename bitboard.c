#include "bitboard.h"
#include "mcts_core.h"
#include <stdio.h>

static inline int to_bit(int r, int c) { return r * 8 + c; }

void board_to_bitboard(int board[8][8], Bitboard *bb) {
    bb->white = 0;
    bb->black = 0;
    bb->white_k = 0;
    bb->black_k = 0;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int bit = r * 8 + c;
            int cell = board[r][c];

            if (cell == 1) bb->white   |= (1ULL << bit);  // Pedina bianca
            if (cell == 2) bb->black   |= (1ULL << bit);  // Pedina nera
            if (cell == 3) bb->white_k |= (1ULL << bit);  // Dama bianca
            if (cell == 4) bb->black_k |= (1ULL << bit);  // Dama nera
        }
    }
}

void bitboard_to_board(Bitboard *bb, int board[8][8]) {
    // Reset board
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if ((r + c) % 2 != 0) board[r][c] = 0;

    for (int bit = 0; bit < 64; bit++) {
        int r = bit / 8, c = bit % 8;
        if ((r + c) % 2 == 0) continue;
        if (bb->white   & (1ULL << bit)) board[r][c] = 1;
        if (bb->black   & (1ULL << bit)) board[r][c] = 2;
        if (bb->white_k & (1ULL << bit)) board[r][c] = 3;
        if (bb->black_k & (1ULL << bit)) board[r][c] = 4;
    }
}