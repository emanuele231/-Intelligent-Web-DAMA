#ifndef BITBOARD_H
#define BITBOARD_H
#include <stdint.h>

typedef struct  Bitboard{
    uint64_t white; uint64_t black;
    uint64_t white_k; uint64_t black_k;
    uint8_t turn;
} Bitboard;

#endif