#include "bitboard.h"
#include "UCB1vers1.h"
#include <stdio.h>

static inline int to_bit(int r, int c) { return r * 8 + c; }

void board_to_bitboard(int board[8][8], Bitboard *bb) {
    bb->white = bb->black = bb->white_k = bb->black_k = 0ULL;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int bit = to_bit(r, c);
            if ((r + c) % 2 == 0) continue; //salta le caselle chiare
            switch (board[r][c]) {
                case 1: bb->white |= (1ULL << bit); break;
                case 2: bb->black |= (1ULL << bit); break;
                case 3: bb->white_k |= (1ULL << bit); break;
                case 4: bb->black_k |= (1ULL << bit); break;
            }
        }
    }
    bb->turn = 1; //tocca al nero
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