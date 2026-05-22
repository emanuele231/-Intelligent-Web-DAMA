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
    if (torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    if (board[torow][tocol] != 0) return false;
    
    int piece = board[fromrow][fromcol];
    if(piece != 1 && piece != 3) return false;

    int drow = torow - fromrow; 
    int dcol = tocol - fromcol;
    if (abs(drow) != 2 || abs(dcol) != 2) return false;

    if (piece == 1 && drow != 2) return false;


    int midRow = fromrow + drow / 2;
    int midCol = fromcol + dcol / 2;
    if (board[midRow][midCol] != 2 && board[midRow][midCol] != 4) return false;

    //cattura
    board[fromrow][fromcol] = 0;
    board[midRow][midCol] = 0;
    board[torow][tocol] = piece;

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

bool check_promotion(int board[8][8], int row, int col) {
    printf("cella (%d, %d)\n", row, col);
    if (board[row][col] == 1 && row == 0) {
        board[row][col] = 3; // 3 = Dama Bianca
        printf("Promozione a Dama! (%d, %d)\n", row, col);
        return true;
    }
    return false;
}

void apply_ai_move(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8 ||
        fromRow < 0 || fromRow >= 8 || fromCol < 0 || fromCol >= 8){
        printf("Fuori dalla scacchiera\n"); return;
    }


    int piece = board[fromRow][fromCol];
    if (piece != 2 && piece != 4) {
        printf("non è una pedina nera");
        return;
    }

    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    if (abs(dRow) == 2 && abs(dCol) == 2) {
        int midRow = (fromRow + toRow) / 2;
        int midCol = (fromCol + toCol) / 2;
        board[midRow][midCol] = 0; // Rimuovi pedina mangiata
        printf("avversario cattura in (%d, %d)\n", midRow, midCol);
    }

    board[fromRow][fromCol] = 0;
    board[toRow][toCol] = piece;

    if (piece == 2 && toRow == 0) {
        board[toRow][toCol] = 4; // 4 = Dama Nera
        printf("avversario promuove a Dama Nera! (%d, %d)\n", toRow, toCol);
    }
}