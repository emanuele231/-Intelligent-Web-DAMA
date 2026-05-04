#include "moves.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool move(int board[8][8], int fromrow, int fromcol, int torow, int tocol){
    if(torow < 0 || torow >=8 || tocol < 0 || tocol >= 8){
        printf("fuori dalla scacchiera");
        return false;
    }

    if(board[fromrow][fromcol] != 1){
        printf("non ce una pedina bianca nella cella selezionata");
        return false;
    }

    if(board[torow][tocol] != 0){
        printf("casella di destinazione occupata");
        return false;
    }

    int drow = torow - fromrow; //differenza tra casella corrente e di destinazione
    int dcol = tocol - fromcol;

    if(drow != -1){
        printf("non si può andare verso il basso");
        return false;
    }

    if(abs(dcol) != 1){
        printf("puoi muoverti di 1 e solo in diagonale");
        return false;
    }

    board[fromrow][fromcol] = 0;
    board[torow][tocol] = 1;
    return true;
}

bool eat (int board[8][8], int fromrow, int torow, int fromcol, int tocol){
    int drow = torow - fromrow; 
    int dcol = tocol - fromcol;

    int midRow = fromrow + drow / 2;
    int midCol = fromcol + dcol / 2;

    if(board[midRow][midCol] != 2) return false;

    //cattura
    board[fromrow][fromcol] = 0;
    board[midRow][midCol] = 0;
    board[torow][tocol] = 0;

    return true;
}

bool dama(int board[8][8], int fromrow, int fromcol, int torow, int tocol){
if (torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    int piece = board[fromrow][fromcol];
    if (piece != 3 && piece != 4) return false;

    if (board[torow][tocol] != 0) return false;

    int dRow = torow - fromrow;
    int dCol = tocol - fromcol;
    
    if (abs(dRow) != abs(dCol) || dRow == 0) return false;

    if (abs(dRow) != 1) return false;

    if ((torow + tocol) % 2 == 0) return false;

    board[fromrow][fromcol] = 0;  // Svuota partenza
    board[torow][tocol] = piece;  // Piazza la Dama a destinazione
    return true;
}