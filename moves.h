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
int count_continued_captures(int board[8][8], int row, int col, int piece);
bool dama(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool check_promotion(int board[8][8], int row, int col);
void apply_ai_move(int board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool has_any_capture(int board[8][8], int player_color);
bool find_best_capture(int board[8][8], int from_row, int from_col, int* out_dr, int* out_dc, int* out_land_r, int* out_land_c);
bool execute_multi_capture(int board[8][8], int start_row, int start_col, int* final_row, int* final_col, int* total_captured);

#endif