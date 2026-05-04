#include "raylib.h"
#include "moves.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)
#define PIECE_RADIUS (CELL_SIZE * 0.35f)

// Stato scacchiera: 0=vuota, 1=pedina bianca, 2=nera, 3=dama bianca, 4=dama nera
int board[8][8] = {0};

// Stato drag & drop
bool isDragging = false;
int dragFromRow = -1, dragFromCol = -1;
int hoverRow = -1, hoverCol = -1;

// Inizializza pedine bianche (righe 5,6,7) sulle caselle scure
void init_board(void) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) { // Solo caselle scure
                if (r >= 5) board[r][c] = 1;
            }
        }
    }
}

// Converte coordinate schermo -> coordinate scacchiera
void screen_to_grid(int mouseX, int mouseY, int *row, int *col) {
    *col = mouseX / CELL_SIZE;
    *row = mouseY / CELL_SIZE;
    if (*col < 0) *col = 0; if (*col >= 8) *col = 7;
    if (*row < 0) *row = 0; if (*row >= 8) *row = 7;
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama - Intelligent Web");
    SetTargetFPS(60);
    init_board();

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        screen_to_grid((int)mouse.x, (int)mouse.y, &hoverRow, &hoverCol);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (board[hoverRow][hoverCol] == 1) { // Solo pedine bianche
                isDragging = true;
                dragFromRow = hoverRow;
                dragFromCol = hoverCol;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
            if (eat(board, dragFromRow, dragFromCol, hoverRow, hoverCol)){
                printf("catturato: +1");
            } else if (!move(board, dragFromRow, dragFromCol, hoverRow, hoverCol)) {
                printf("Mossa non valida! La pedina torna indietro.\n");
            } else if (dama(board, dragFromRow, dragFromCol, hoverRow, hoverCol)){
                printf("DAMA!");
            }
            isDragging = false;
            dragFromRow = -1; dragFromCol = -1;
        }

        if(dragFromRow == 7){
            printf("DAMA!");
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 1. Disegna scacchiera
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Color cell = ((r + c) % 2 == 0) ? (Color){240, 217, 181, 255} 
                                                : (Color){181, 136, 99, 255};
                DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cell);

                // Evidenzia casella sotto il mouse durante il drag
                if (isDragging && r == hoverRow && c == hoverCol) {
                    DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, 
                                  (Color){0, 255, 0, 80});
                }
            }
        }

        // 2. Disegna pedine ferme
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (board[r][c] == 1) {
                    // Nascondi la pedina originale mentre la trasciniamo
                    if (isDragging && r == dragFromRow && c == dragFromCol) continue;

                    Vector2 center = {(float)(c * CELL_SIZE + CELL_SIZE/2), 
                                      (float)(r * CELL_SIZE + CELL_SIZE/2)};
                    DrawCircleV(center, PIECE_RADIUS, WHITE);
                    DrawCircleLinesV(center, PIECE_RADIUS, BLACK);
                }
            }
        }

        // 3. Disegna "fantasma" della pedina trascinata
        if (isDragging) {
            DrawCircleV(mouse, PIECE_RADIUS, (Color){255, 255, 255, 180});
            DrawCircleLinesV(mouse, PIECE_RADIUS, RED);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}