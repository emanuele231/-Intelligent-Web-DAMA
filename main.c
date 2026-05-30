#include "raylib.h"
#include "moves.h"
#include <time.h>
#include "bitboard.h"
#include "UCB1vers1.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)
#define PIECE_RADIUS (CELL_SIZE * 0.35f)

MemoryPool ai_pool;

bool isPlayerTurn = true;
bool isIAthinking = false;
bool showHeader = true;
bool firstMoveDone = false;

// Stato scacchiera: 0=vuota, 1=pedina bianca, 2=nera, 3=dama bianca, 4=dama nera
int board[8][8] = {0};

// Stato drag & drop
bool isDragging = false;
int dragFromRow = -1, dragFromCol = -1;
int hoverRow = -1, hoverCol = -1;

// Inizializza pedine: Bianco in basso (righe 5-7), Nero in alto (righe 0-2)
void init_board(void) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) { // Solo caselle scure
                if (r >= 5) board[r][c] = 1;  // Bianco (TU) in basso
                if (r <= 2) board[r][c] = 2;  // Nero (IA) in alto
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
        if(IsKeyPressed(KEY_Q)){
        showHeader = !showHeader;
    }
        Vector2 mouse = GetMousePosition();
        screen_to_grid((int)mouse.x, (int)mouse.y, &hoverRow, &hoverCol);

        // INPUT: Pressione mouse (inizio drag)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isPlayerTurn) {
            int piece = board[hoverRow][hoverCol];
            if (piece == 1 || piece == 3) { 
                isDragging = true;
                dragFromRow = hoverRow;
                dragFromCol = hoverCol;
            }
        }

        // INPUT: Rilascio mouse (esecuzione mossa)
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
    bool successo = false;
    int finalRow = -1, finalCol = -1;

    //AUTO-CATTURA: Scansiona le diagonali verso il basso
    // Bianco si muove verso righe maggiori (+1). Controlla le 2 diagonali.
    int dr = -1; 
    int dc_opts[2] = {1, -1};

    for(int i = 0; i < 2; i++) {
        int midR = dragFromRow + dr;
        int midC = dragFromCol + dc_opts[i];
        int landR = dragFromRow + 2 * dr;
        int landC = dragFromCol + 2 * dc_opts[i];

        // Verifica che la terra di atterraggio sia dentro la scacchiera
        if(landR < 8 && landC >= 0 && landC < 8) {
            //Se c'è una nera nel mezzo E la casella di arrivo è vuota
            if((board[midR][midC] == 2 || board[midR][midC] == 4) && board[landR][landC] == 0) {
                // Esegui cattura automatica
                board[dragFromRow][dragFromCol] = 0;      // Libera partenza
                board[midR][midC] = 0;                    // ️ Mangia la nera
                board[landR][landC] = 1;                  // Posiziona bianca
                finalRow = landR;
                finalCol = landC;
                successo = true;
                printf("Cattura automatica: nera in (%d,%d) rimossa!\n", midR, midC);
                break; // Cattura trovata ed eseguita
            }
        }
    }

    // Se non era una cattura, procedi con mosse normali (coordinate mouse)
    if(!successo) {
        int destRow = hoverRow;
        int destCol = hoverCol;
        if(dragFromRow != destRow || dragFromCol != destCol) {
            if(dama(board, dragFromRow, dragFromCol, destRow, destCol)) {
                printf("Mossa Dama!\n"); successo = true; finalRow = destRow; finalCol = destCol;
            } else if(move(board, dragFromRow, dragFromCol, destRow, destCol)) {
                printf("Mossa Pedina!\n"); successo = true; finalRow = destRow; finalCol = destCol;
            } else {
                printf("Mossa non valida.\n");
            }
        }
    }

    //  Gestione post-mossa
    if(successo) {
        check_promotion(board, finalRow, finalCol);
        if (!firstMoveDone) {
            firstMoveDone = true;
            showHeader = false;
        }

        isPlayerTurn = false;
        isIAthinking = true;
    }

    // Reset drag
    isDragging = false;
    dragFromRow = -1; 
    dragFromCol = -1;
}

        // LOGICA IA (UCB1 MCTS - 0.2s)
        if (!isPlayerTurn && isIAthinking) {
            static clock_t ai_start = 0;
            static Bitboard ai_state;

            if (ai_start == 0) {
                ai_start = clock();
                board_to_bitboard(board, &ai_state);
                printf("l'avversario sta pensando...\n");
            }

            if (((clock() - ai_start) / (float)CLOCKS_PER_SEC) >= 0.2f) {
                mcts_search(&ai_state, 0.2f, &ai_pool);
                Move best = get_best_move(&ai_pool.nodes[0], &ai_state);

                int fromR = best.from / 8;
                int fromC = best.from % 8;
                int toR   = best.to / 8;
                int toC   = best.to % 8;

                printf("l'avversario sposta: (%d,%d) -> (%d,%d)\n", fromR, fromC, toR, toC);
                apply_ai_move(board, fromR, fromC, toR, toC);

                isPlayerTurn = true;
                isIAthinking = false;
                ai_start = 0;
                printf("tocca a te!\n");
            }
        }

        // RENDERING======================================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Disegna scacchiera
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Color cell = ((r + c) % 2 == 0) ? (Color){240, 217, 181, 255} 
                                                : (Color){181, 136, 99, 255};
                DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cell);

                if (isDragging && r == hoverRow && c == hoverCol) {
                    DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, 
                                  (Color){0, 255, 0, 80});
                }
            }
        }

        //  Disegna pedine ferme 
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                int piece = board[r][c];
                if (piece == 0) continue; // Salta caselle vuote
                
                // Nascondi pedina originale durante il drag
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
                else if (piece == 3) {      // Dama Bianca
                    fillColor = GOLD;
                    borderColor = ORANGE;
                }
                else if (piece == 2) {      // Pedina Nera
                    fillColor = BLACK;
                    borderColor = GRAY;
                }
                else if (piece == 4) {      // Dama Nera
                    fillColor = DARKPURPLE;
                    borderColor = VIOLET;
                }
                else continue;

                DrawCircleV(center, PIECE_RADIUS, fillColor);
                DrawCircleLinesV(center, PIECE_RADIUS, borderColor); 
            }
        }

        //  Disegna "fantasma" della pedina trascinata
        if (isDragging) {
            DrawCircleV(mouse, PIECE_RADIUS, (Color){255, 255, 255, 180});
            DrawCircleLinesV(mouse, PIECE_RADIUS, RED);
        }

        //header
    if (showHeader) {
        int headerH = 40;
        DrawRectangle(0, 0, SCREEN_WIDTH, headerH, (Color){0, 0, 0, 150});

        Rectangle oppBtn = {GetScreenWidth() - 140, 5, 130, 25};
        bool isHovering = CheckCollisionPointRec(GetMousePosition(), oppBtn);
        DrawRectangleRec(oppBtn, isHovering ? (Color){50, 50, 85, 255} : (Color){35, 35, 60, 255});
        DrawRectangleLinesEx(oppBtn, 1.5f, WHITE);
        DrawText("UCB1-02", oppBtn.x + 8, oppBtn.y + 6, 12, WHITE);

        if (isHovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            //TODO - Menu in cui si visualizzano le ia
        }

        const char* aiName = "UCB1-0.2";
        int nameWidth = MeasureText(aiName, 22);
        DrawText(aiName, (SCREEN_WIDTH - nameWidth) / 2, 7, 22, (Color){125, 211, 252, 255});

        int turnY = headerH + 16;
        if (isPlayerTurn) {
            DrawRectangle(0, turnY, 150, 30, (Color){255, 255, 255, 200});
            DrawText("TOCCA A TE!", 10, 5, 20, WHITE);
        } else {
            DrawRectangle(0, turnY, 150, 30, (Color){0, 0, 0, 200});
            DrawText("IA STA PENSANDO...", 10, 5, 16, WHITE);
        }
    }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}