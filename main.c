#include "raylib.h"
#include "Piece.h"
#include <stdio.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE    8
#define CELL_SIZE     (SCREEN_WIDTH / BOARD_SIZE)

// Array di pedine (massimo 24 pedine per giocatore)
#define MAX_PIECES 24
Piece black_pieces[MAX_PIECES];
Piece white_pieces[MAX_PIECES];
int black_count = 0;
int white_count = 0;

// Pedina selezionata
Piece* selected_piece = NULL;

void init_pieces(void) {
    // Inizializza pedine nere (in alto, righe 0-2)
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if ((r + c) % 2 != 0) {
                piece_init(&black_pieces[black_count], PIECE_BLACK, r, c);
                black_count++;
            }
        }
    }
    
    // Inizializza pedine bianche (in basso, righe 5-7)
    for (int r = 5; r < 8; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if ((r + c) % 2 != 0) {
                piece_init(&white_pieces[white_count], PIECE_WHITE, r, c);
                white_count++;
            }
        }
    }
}

void draw_board(void) {
    // Disegna scacchiera
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Color cell = ((r + c) % 2 == 0) 
                ? (Color){240, 217, 181, 255} 
                : (Color){181, 136, 99, 255};
            DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cell);
        }
    }
    
    // Disegna tutte le pedine nere
    for (int i = 0; i < black_count; i++) {
        if (black_pieces[i].active) {
            piece_draw(&black_pieces[i], CELL_SIZE);
        }
    }
    
    // Disegna tutte le pedine bianche
    for (int i = 0; i < white_count; i++) {
        if (white_pieces[i].active) {
            piece_draw(&white_pieces[i], CELL_SIZE);
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama - Intelligent Web");
    SetTargetFPS(60);
    
    init_pieces();
    
    printf("Pedine nere: %d\n", black_count);
    printf("Pedine bianche: %d\n", white_count);

    while (!WindowShouldClose()) {
        // Gestione mouse (preparazione per il prossimo step)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse_pos = GetMousePosition();
            int clicked_col = mouse_pos.x / CELL_SIZE;
            int clicked_row = mouse_pos.y / CELL_SIZE;
            
            printf("Click su: riga=%d, col=%d\n", clicked_row, clicked_col);
            
            // Qui aggiungeremo la logica di selezione
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_board();
        
        // Istruzioni
        DrawText("Dama - Clicca su una pedina per selezionarla", 10, SCREEN_HEIGHT - 30, 20, DARKGRAY);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}