#include "moves.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Struttura per memorizzare le opzioni di cattura
typedef struct {
    int dr, dc;
    int captured_kings;  // Numero di dame catturate
    int captured_pawns;  // Numero di pedine catturate
    int total_captures;  // Totale pezzi catturati
    int final_row, final_col;
} CaptureOption;

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

bool has_any_capture(int board[8][8], int player_color) {
    int king_val = (player_color == 1) ? 3 : 4;
    int enemy = (player_color == 1) ? 2 : 1;
    int enemy_k = (player_color == 1) ? 4 : 3;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board[r][c];
            // Controlla sia pedine che dame del giocatore
            if (piece != player_color && piece != king_val) continue;

            int dr_list[4] = {-2, -2, 2, 2};
            int dc_list[4] = {-2, 2, -2, 2};

            for (int i = 0; i < 4; i++) {
                // PEDINE: solo avanti, DAME: tutte le direzioni
                if (piece == 1 && dr_list[i] != -2) continue;  // Pedina bianca solo su
                if (piece == 2 && dr_list[i] != 2) continue;   // Pedina nera solo giù
                // Dame (3 e 4) possono catturare in tutte le direzioni - nessun continue

                int nr = r + dr_list[i];
                int nc = c + dc_list[i];
                if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
                if (board[nr][nc] != 0) continue;  // Destinazione deve essere libera

                int mid_r = r + dr_list[i] / 2;
                int mid_c = c + dc_list[i] / 2;
                int mid_piece = board[mid_r][mid_c];
                
                // Pedina bianca mangia solo pedine nere
                if (piece == 1 && mid_piece != 2) continue;
                // Pedina nera mangia solo pedine bianche
                if (piece == 2 && mid_piece != 1) continue;
                // Dama mangia sia pedine che dame avversarie
                if ((piece == 3 || piece == 4) && mid_piece != enemy && mid_piece != enemy_k) continue;

                return true;  // Cattura trovata!
            }
        }
    }
    return false;
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
    
    if (abs(drow) != 2 || abs(dcol) != -2) return false;

    int midRow = fromrow + drow / 2;
    int midCol = fromcol + dcol / 2;

    if (board[midRow][midCol] != 1 && board[midRow][midCol] != 2 && 
        board[midRow][midCol] != 3 && board[midRow][midCol] != 4) return false;

    board[fromrow][fromcol] = 0;
    board[midRow][midCol] = 0;
    board[torow][tocol] = piece;

    return true;
}

void apply_ai_move(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8 ||
        fromRow < 0 || fromRow >= 8 || fromCol < 0 || fromCol >= 8) return;

    int piece = board[fromRow][fromCol];
    if (piece < 1 || piece > 4) return;

    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;

    // Cattura: rimuovi pezzo in mezzo
    if (abs(dRow) == 2 && abs(dCol) == 2) {
        int midRow = (fromRow + toRow) / 2;
        int midCol = (fromCol + toCol) / 2;
        board[midRow][midCol] = 0;
    }

    // Sposta il pezzo
    board[fromRow][fromCol] = 0;
    board[toRow][toCol] = piece;

    // Promozione
    check_promotion(board, toRow, toCol);
}

bool check_promotion(int board[8][8], int row, int col) {
    // Pedina bianca (1) arriva in riga 0 -> diventa Dama bianca (3)
    if (board[row][col] == 1 && row == 0) {
        board[row][col] = 3;
        printf("   Promozione a Dama Bianca! (%d, %d)\n", row, col);
        return true;
    }
    // Pedina nera (2) arriva in riga 7 -> diventa Dama nera (4)
    if (board[row][col] == 2 && row == 7) {
        board[row][col] = 4;
        printf("   Promozione a Dama Nera! (%d, %d)\n", row, col);
        return true;
    }
    return false;
}

// Conta catture ricorsivamente per presa multipla
static int count_captures_recursive(int board[8][8], int row, int col, int piece,
                                    int *kings, int *pawns) {
    int max_kings = 0, max_pawns = 0;
    int dr_list[4] = {-2, -2, 2, 2};
    int dc_list[4] = {-2, 2, -2, 2};
    int is_king = (piece == 3 || piece == 4);
    bool is_white = (piece == 1 || piece == 3);
    
    for (int i = 0; i < 4; i++) {
        if (!is_king && piece == 1 && dr_list[i] != -2) continue;
        if (!is_king && piece == 2 && dr_list[i] != 2) continue;
        
        int mid_r = row + dr_list[i] / 2;
        int mid_c = col + dc_list[i] / 2;
        int land_r = row + dr_list[i];
        int land_c = col + dc_list[i];
        
        if (land_r < 0 || land_r >= 8 || land_c < 0 || land_c >= 8) continue;
        
        int mid = board[mid_r][mid_c];
        
        // CORREZIONE: Verifica che sia un pezzo NEMICO
        bool is_enemy = false;
        if (is_white) {
            is_enemy = (mid == 2 || mid == 4);  // Bianco mangia nero
        } else {
            is_enemy = (mid == 1 || mid == 3);  // Nero mangia bianco
        }
        
        if (!is_enemy) continue;
        
        if (board[land_r][land_c] != 0) continue;
        
        int saved_mid = board[mid_r][mid_c];
        int saved_from = board[row][col];
        
        board[row][col] = 0;
        board[mid_r][mid_c] = 0;
        board[land_r][land_c] = saved_from;
        
        int next_kings = 0, next_pawns = 0;
        count_captures_recursive(board, land_r, land_c, saved_from, &next_kings, &next_pawns);
        
        bool is_enemy_king = (mid == 3 || mid == 4);
        bool is_enemy_pawn = (mid == 1 || mid == 2);
        
        int total_kings = (is_enemy_king ? 1 : 0) + next_kings;
        int total_pawns = (is_enemy_pawn ? 1 : 0) + next_pawns;
        
        if (total_kings > max_kings || (total_kings == max_kings && total_pawns > max_pawns)) {
            max_kings = total_kings;
            max_pawns = total_pawns;
        }
        
        board[row][col] = saved_from;
        board[mid_r][mid_c] = saved_mid;
        board[land_r][land_c] = 0;
    }
    
    *kings = max_kings;
    *pawns = max_pawns;
    return max_kings + max_pawns;
}

bool find_best_capture(int board[8][8], int from_row, int from_col,
                       int *out_dr, int *out_dc, int *out_land_r, int *out_land_c) {
    int piece = board[from_row][from_col];
    CaptureOption best = {0};
    bool found = false;
    
    int dr_list[4] = {-2, -2, 2, 2};
    int dc_list[4] = {-2, 2, -2, 2};
    int is_king = (piece == 3 || piece == 4);
    
    // Determina quali sono i pezzi nemici
    bool is_white = (piece == 1 || piece == 3);
    
    for (int i = 0; i < 4; i++) {
        if (!is_king && piece == 1 && dr_list[i] != -2) continue;
        if (!is_king && piece == 2 && dr_list[i] != 2) continue;
        
        int mid_r = from_row + dr_list[i] / 2;
        int mid_c = from_col + dc_list[i] / 2;
        int land_r = from_row + dr_list[i];
        int land_c = from_col + dc_list[i];
        
        if (land_r < 0 || land_r >= 8 || land_c < 0 || land_c >= 8) continue;
        
        int mid_piece = board[mid_r][mid_c];
        
        // CORREZIONE: Verifica che sia un pezzo NEMICO
        bool is_enemy = false;
        if (is_white) {
            // Bianco mangia solo nero (2 o 4)
            is_enemy = (mid_piece == 2 || mid_piece == 4);
        } else {
            // Nero mangia solo bianco (1 o 3)
            is_enemy = (mid_piece == 1 || mid_piece == 3);
        }
        
        if (!is_enemy) continue;  // Salta se non è un nemico
        
        bool is_enemy_king = (mid_piece == 3 || mid_piece == 4);
        bool is_enemy_pawn = (mid_piece == 1 || mid_piece == 2);
        
        if (board[land_r][land_c] != 0) continue;
        
        // Simula cattura
        int saved_mid = board[mid_r][mid_c];
        int saved_from = board[from_row][from_col];
        
        board[from_row][from_col] = 0;
        board[mid_r][mid_c] = 0;
        board[land_r][land_c] = saved_from;
        
        int future_kings = 0, future_pawns = 0;
        count_captures_recursive(board, land_r, land_c, saved_from, &future_kings, &future_pawns);
        
        int total_kings = (is_enemy_king ? 1 : 0) + future_kings;
        int total_pawns = (is_enemy_pawn ? 1 : 0) + future_pawns;
        int total = total_kings + total_pawns;
        
        board[from_row][from_col] = saved_from;
        board[mid_r][mid_c] = saved_mid;
        board[land_r][land_c] = 0;
        
        if (!found ||
            total > best.total_captures ||
            (total == best.total_captures && total_kings > best.captured_kings)) {
            best.dr = dr_list[i];
            best.dc = dc_list[i];
            best.captured_kings = total_kings;
            best.captured_pawns = total_pawns;
            best.total_captures = total;
            best.final_row = land_r;
            best.final_col = land_c;
            found = true;
        }
    }
    
    if (found) {
        *out_dr = best.dr;
        *out_dc = best.dc;
        *out_land_r = best.final_row;
        *out_land_c = best.final_col;
    }
    
    return found;
}

// Esegue presa multipla obbligatoria
bool execute_multi_capture(int board[8][8], int start_row, int start_col, 
                           int* final_row, int* final_col, int* total_captured) {
    int row = start_row, col = start_col;
    int piece = board[row][col];
    *total_captured = 0;
    bool captured_something = false;
    
    while (1) {
        int dr, dc, land_r, land_c;
        
        if (!find_best_capture(board, row, col, &dr, &dc, &land_r, &land_c)) {
            break;  // Nessuna cattura disponibile
        }
        
        int mid_r = row + dr / 2;
        int mid_c = col + dc / 2;
        
        // Esegui cattura
        board[row][col] = 0;
        board[mid_r][mid_c] = 0;
        board[land_r][land_c] = piece;
        
        (*total_captured)++;
        captured_something = true;
        
        // Verifica promozione
        if (piece == 1 && land_r == 0) {
            board[land_r][land_c] = 3;  // Dama bianca
            piece = 3;
        } else if (piece == 2 && land_r == 7) {
            board[land_r][land_c] = 4;  // Dama nera
            piece = 4;
        }
        
        row = land_r;
        col = land_c;
    }
    
    *final_row = row;
    *final_col = col;
    return captured_something;
}

bool has_legal_moves(int board[8][8], int player_color) {
    // Se ci sono catture, ci sono mosse legali
    if (has_any_capture(board, player_color)) return true;
    
    // Controlla mosse semplici
    int king_val = (player_color == 1) ? 3 : 4;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board[r][c];
            if (piece != player_color && piece != king_val) continue;
            
            int is_king = (piece == 3 || piece == 4);
            int dr_list[4] = {-1, -1, 1, 1};
            int dc_list[4] = {-1, 1, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                // Pedina: solo avanti, Dama: tutte le direzioni
                if (!is_king && piece == 1 && dr_list[i] != -1) continue;
                if (!is_king && piece == 2 && dr_list[i] != 1) continue;
                
                int nr = r + dr_list[i];
                int nc = c + dc_list[i];
                if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) continue;
                if (board[nr][nc] == 0) return true;  // Mossa semplice valida
            }
        }
    }
    return false;  // Nessun movimento legale
}