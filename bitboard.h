#ifndef BITBOARD_H
#define BITBOARD_H
#include <stdint.h>

typedef struct  Bitboard{
    uint64_t white; uint64_t black;
    uint64_t white_k; uint64_t black_k;
    uint8_t turn;
} Bitboard;

static inline int to_bit(int r, int c);
void board_to_bitboard(int board[8][8], Bitboard *bb);
void bitboard_to_board(Bitboard *bb, int board[8][8]);
#endif