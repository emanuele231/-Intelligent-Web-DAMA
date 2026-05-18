#include "raylib.h"
#include "moves.h"
#include <time.h>
#include "bitboard.h"
#include "UCB1vers1.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)
#define PIECE_RADIUS (CELL_SIZE * 0.35f)
#define PIECE_RADIUS_QUEEN (CELL_SIZE * 0.20f)
MemoryPool ai_pool;

bool isPlayerTurn = true;
bool isIAthinking = false;

// Stato scacchiera: 0=vuota, 1=pedina bianca, 2=nera, 3=dama bianca, 4=dama nera
int board[8][8] = {0};

// Stato drag & drop
bool isDragging = false;
int dragFromRow = -1, dragFromCol = -1;
int hoverRow = -1, hoverCol = -1;
int destrow = -1, destcol = -1;

// Inizializza pedine bianche (righe 5,6,7) sulle caselle scure
void init_board(void) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) { // Solo caselle scure
                if (r >= 5) board[r][c] = 1;
                if (r <= 2) board[r][c] = 2;
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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isPlayerTurn) {
            int piece = board[hoverRow][hoverCol];
            if (piece == 1 | piece == 3) { // Solo pedine bianche
                isDragging = true;
                dragFromRow = hoverRow;
                dragFromCol = hoverCol;
                destrow = hoverRow;
                destcol = hoverCol;
            }
        }

if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
    bool successo = false;  // Dichiarata e inizializzata QUI
    int destRow = hoverRow; // Usa le variabili corrette
    int destCol = hoverCol;


    if (eat(board, dragFromRow, dragFromCol, destRow, destCol)) {
        printf("Cattura riuscita!\n");
        successo = true;  
    }
    else if (dama(board, dragFromRow, dragFromCol, destRow, destCol)) {
        printf("Mossa Dama!\n");
        successo = true;
    }
    else if (move(board, dragFromRow, dragFromCol, destRow, destCol)) {
        printf("Mossa Pedina!\n");
        successo = true;
    }
    else {
        // Solo se TUTTE le precedenti hanno fallito
        printf("Mossa non valida! La pedina torna indietro.\n");
    }

    // Verifica promozione DOPO qualsiasi mossa riuscita
    if (successo) {
        check_promotion(board, destRow, destCol);
        isPlayerTurn = false;
        isIAthinking = true;
    }

    // Reset stato drag
    isDragging = false;
    dragFromRow = -1;
    dragFromCol = -1;
}

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (!isPlayerTurn && isIAthinking) {
            static clock_t ai_start = 0;
            static Bitboard ai_state;

            if (ai_start == 0) {
                ai_start = clock();
                board_to_bitboard(board, &ai_state);
                printf("l'avversario sta pensando");
            }
            if (((clock() - ai_start) / (float)CLOCKS_PER_SEC) >= 0.2f) {
                mcts_search(&ai_state, 0.2f, &ai_pool);

                // Recupera la mossa migliore basata sulle visite
                Move best = get_best_move(&ai_pool.nodes[0]); // Il root è il primo nodo allocato

                int fromR = best.from / 8;
                int fromC = best.from % 8;
                int toR   = best.to / 8;
                int toC   = best.to % 8;

                printf("🔄 l'avversario sposta: (%d,%d) -> (%d,%d)\n", fromR, fromC, toR, toC);

                apply_ai_move(board, fromR, fromC, toR, toC);

                //bitboard_to_board(&ai_state, board);

                //tocca a noi
                isPlayerTurn = true;
                isIAthinking = false;
                ai_start = 0;
                printf("tocca a te!");
            }
        }

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
                int piece = board[r][c];
                if (piece == 0) continue; 
                    if (isDragging && r == dragFromRow && c == dragFromCol) continue;

                    Vector2 center = {
                        (float)(c * CELL_SIZE + CELL_SIZE/2), 
                        (float)(r * CELL_SIZE + CELL_SIZE/2)
                                    };
                Color fillColor, borderColor;
                if (piece == 1) {           // Pedina Bianca
                    fillColor = WHITE;
                    borderColor = DARKGRAY;
                }
                else if (piece == 3) {      // Dama Bianca (GIALLO!)
                    fillColor = GOLD;
                    borderColor = ORANGE;
                }
                else if (piece == 2) {      // Pedina Nera (IA)
                    fillColor = BLACK;
                    borderColor = GRAY;
                }
                else if (piece == 4) {      // Dama Nera
                    fillColor = DARKPURPLE;
                    borderColor = VIOLET;
                }
                else continue;

                DrawCircleV(center, PIECE_RADIUS, fillColor);
                DrawCircleLinesV(center, PIECE_RADIUS, fillColor);

            }
        }

        // 3. Disegna "fantasma" della pedina trascinata
        if (isDragging) {
            DrawCircleV(mouse, PIECE_RADIUS, (Color){255, 255, 255, 180});
            DrawCircleLinesV(mouse, PIECE_RADIUS, RED);
        }

        if(isPlayerTurn) {
            DrawRectangle(0, 0, 150, 30, (Color){255, 255, 255, 200});
            DrawText("TOCCA A TE!", 10, 5, 20, BLACK);
        } else {
            DrawRectangle(0, 0, 150, 30, (Color){0, 0, 0, 200});
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}