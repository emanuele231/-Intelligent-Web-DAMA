#ifndef MOVES_H
#define MOVES_H
#include <stdbool.h>
#include <stdint.h>

#include "Piece.h"


typedef struct {
    uint8_t from;
    uint8_t to;
    uint8_t capture;
} Move;


bool move(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool eat(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool dama(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool check_promotion(int board[8][8], int row, int col);
void apply_ai_move(int board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool has_any_capture(int board[8][8], int player_color);

#endif