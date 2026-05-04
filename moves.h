#ifndef MOVES_H
#define MOVES_H

#include "Piece.h"


bool move(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool eat(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
bool dama(int board[8][8], int fromrow, int fromcol, int torow, int tocol);
/*
void catch(Piece p, int row, int col);
void king_move(Piece p, int row, int col);
void multiple_catch(Piece p, int row, int col);
*/

#endif