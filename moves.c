#include "moves.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// 1. MOSSA SEMPLICE (Pedina Bianca)
// Bianco va verso l'ALTO (Righe decrescenti: 7 -> 0)
bool move(int board[8][8], int fromrow, int fromcol, int torow, int tocol){
    if(torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    if(board[fromrow][fromcol] != 1) return false;  // Solo pedina bianca
    if(board[torow][tocol] != 0) return false;  // Destinazione deve essere vuota

    int drow = torow - fromrow;
    int dcol = tocol - fromcol;

    // Pedina bianca va SOLO verso l'ALTO (righe decrescenti)
    if(drow != -1) return false;
    if(abs(dcol) != 1) return false;

    board[fromrow][fromcol] = 0;
    board[torow][tocol] = 1;
    return true;
}
// 2. CATTURA (Pedina Bianca)
bool eat(int board[8][8], int fromrow, int torow, int fromcol, int tocol){
    if (torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    if (board[torow][tocol] != 0) return false;
    
    int piece = board[fromrow][fromcol];
    if(piece != 1) return false;  // Solo pedina bianca

    int drow = torow - fromrow; 
    int dcol = tocol - fromcol;
    
    if (abs(drow) != 2 || abs(dcol) != 2) return false;

    if (drow != -2) return false;

    // Pedina può mangiare sia avanti (-2) che indietro (+2)
    int midRow = fromrow + drow / 2;
    int midCol = fromcol + dcol / 2;

    // Deve esserci una pedina nera da mangiare
    if (board[midRow][midCol] != 2) return false;

    // Esegui cattura
    board[fromrow][fromcol] = 0;
    board[midRow][midCol] = 0;
    board[torow][tocol] = 1;  // Rimane pedina (non diventa dama durante cattura)

    return true;
}

int count_continued_captures(int board[8][8], int row, int col, int piece) {
    int max_additional = 0;
    int dr_list[4] = {-2, -2, 2, 2};
    int dc_list[4] = {-2, 2, -2, 2};
    
    for (int i = 0; i < 4; i++) {
        if (piece == 1 && dr_list[i] != -2) continue; // Pedina solo avanti
        
        int midR = row + dr_list[i] / 2;
        int midC = col + dc_list[i] / 2;
        int landR = row + dr_list[i];
        int landC = col + dc_list[i];
        
        if (landR < 0 || landR >= 8 || landC < 0 || landC >= 8) continue;
        
        int mid_piece = board[midR][midC];
        if (piece == 1 && mid_piece != 2) continue;
        if (piece == 3 && mid_piece != 2 && mid_piece != 4) continue;
        
        if (board[landR][landC] == 0) {
            int additional = 1 + count_continued_captures(board, landR, landC, piece);
            if (additional > max_additional) max_additional = additional;
        }
    }
    return max_additional;
}

// 3. VERIFICA SE ESISTE ALMENO UNA CATTURA
bool has_any_capture(int board[8][8], int player_color) {
    int enemy = (player_color == 1) ? 2 : 1;
    int enemy_k = (player_color == 1) ? 4 : 3;
    int king_val = (player_color == 1) ? 3 : 4;
    bool found = false;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board[r][c];
            if (piece != player_color && piece != king_val) continue;

            int dr_list[4] = {-2, -2, 2, 2};
            int dc_list[4] = {-2, 2, -2, 2};

            for (int i = 0; i < 4; i++) {
                // Bianco cattura SOLO verso l'ALTO (-2)
                if (player_color == 1 && dr_list[i] != -2) continue;

                int nr = r + dr_list[i];
                int nc = c + dc_list[i];
                if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
                if ((nr + nc) % 2 == 0) continue; // Deve essere casella scura
                if (board[nr][nc] != 0) continue;

                int mid_r = r + dr_list[i] / 2;
                int mid_c = c + dc_list[i] / 2;
                
                if (board[mid_r][mid_c] == enemy || board[mid_r][mid_c] == enemy_k) {
                    printf("TROVATA CATTURA DISPONIBILE: Pedina %s in (%d,%d) può mangiare in (%d,%d)\n", 
                           (player_color == 1 ? "BIANCA" : "NERA"), r, c, nr, nc);
                    found = true;
                }
            }
        }
    }
    if (!found) printf(" NESSUNA CATTURA DISPONIBILE sulla scacchiera.\n");
    return found;
}

// 4. MOSSA DAMA
bool dama(int board[8][8], int fromrow, int fromcol, int torow, int tocol){
    if (torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    int piece = board[fromrow][fromcol];
    if (piece != 3 && piece != 4) return false;

    if (board[torow][tocol] != 0) return false;

    int dRow = torow - fromrow;
    int dCol = tocol - fromcol;
    
    // Deve muoversi in diagonale di 1 casella
    if (abs(dRow) != abs(dCol) || dRow == 0) return false;
    if (abs(dRow) != 1) return false;

    board[fromrow][fromcol] = 0;
    board[torow][tocol] = piece;
    return true;
}

// 5. CATTURA DAMA
bool dama_eat(int board[8][8], int fromrow, int torow, int fromcol, int tocol){
    if (torow < 0 || torow >= 8 || tocol < 0 || tocol >= 8) return false;
    if (board[torow][tocol] != 0) return false;
    
    int piece = board[fromrow][fromcol];
    if(piece != 3 && piece != 4) return false;

    int drow = torow - fromrow; 
    int dcol = tocol - fromcol;
    
    if (abs(drow) != 2 || abs(dcol) != 2) return false;

    int midRow = fromrow + drow / 2;
    int midCol = fromcol + dcol / 2;

    if (board[midRow][midCol] != 1 && board[midRow][midCol] != 2 && 
        board[midRow][midCol] != 3 && board[midRow][midCol] != 4) return false;

    board[fromrow][fromcol] = 0;
    board[midRow][midCol] = 0;
    board[torow][tocol] = piece;

    return true;
}

// 6. PROMOZIONE A DAMA
bool check_promotion(int board[8][8], int row, int col) {
    // Bianco diventa Dama a riga 0
    if (board[row][col] == 1 && row == 0) {
        board[row][col] = 3;
        printf("Promozione a Dama Bianca! (%d, %d)\n", row, col);
        return true;
    }
    // Nero diventa Dama a riga 7
    if (board[row][col] == 2 && row == 7) {
        board[row][col] = 4;
        printf("Promozione a Dama Nera! (%d, %d)\n", row, col);
        return true;
    }
    return false;
}

// 7. APPLICA MOSSA IA
void apply_ai_move(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8 ||
        fromRow < 0 || fromRow >= 8 || fromCol < 0 || fromCol >= 8) {
        printf("apply_ai_move: Fuori dalla scacchiera\n");
        return;
    }

    int piece = board[fromRow][fromCol];
    if (piece != 2 && piece != 4) {
        printf("apply_ai_move: non è una pedina nera (trovato %d)\n", piece);
        return;
    }

    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    // ✅ GESTIONE CATTURA: se la distanza è 2, rimuovi il pezzo in mezzo
    if (abs(dRow) == 2 && abs(dCol) == 2) {
        int midRow = (fromRow + toRow) / 2;
        int midCol = (fromCol + toCol) / 2;
        
        int captured_piece = board[midRow][midCol];
        printf("IA NERA cattura pezzo %d in (%d,%d)\n", captured_piece, midRow, midCol);
        
        // ✅ RIMUOVI il pezzo catturato
        board[midRow][midCol] = 0;
    }

    // Sposta il pezzo
    board[fromRow][fromCol] = 0;
    board[toRow][toCol] = piece;
    
    printf("   IA: (%d,%d) -> (%d,%d)\n", fromRow, fromCol, toRow, toCol);
}