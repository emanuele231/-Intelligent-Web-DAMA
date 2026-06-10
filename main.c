#define NOGDI              // Disabilita Rectangle() di Windows GDI (conflitto con Raylib)
#define WIN32_LEAN_AND_MEAN // Include Windows.h leggero

#include "raylib.h"
#include "moves.h"
#include <time.h>
#include "bitboard.h"
#include "ai_engine.h"   // Interfaccia modulare IA
#include "UCB1.h"        // Per register_ucb1_base()
#include "params.h"      // Sistema configurazione generico
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)
#define PIECE_RADIUS (CELL_SIZE * 0.35f)

// === VARIABILI GLOBALI ===
MemoryPool ai_pool;

// Stato gioco
bool isPlayerTurn = true;
bool isIAthinking = false;
bool showHeader = true;
bool firstMoveDone = false;

// Sistema IA modulare
int current_engine_idx = 0;
const AIEngineDef* current_engine = NULL;
AI_Instance* current_ai_instance = NULL;
float aiTimeLimit = 0.2f;
Color headerColor = {0, 0, 0, 150};

// UI Menu
bool showAIMenu = false;
Rectangle aiMenuRect = {0};

// Board: 0=vuota, 1=bianco, 2=nero, 3=dama bianca, 4=dama nera
int board[8][8] = {0};

// Drag & drop
bool isDragging = false;
int dragFromRow = -1, dragFromCol = -1;
int hoverRow = -1, hoverCol = -1;

// ============================================================================
// FUNZIONI DI GIOCO
// ============================================================================

void init_board(void) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 != 0) {
                if (r >= 5) board[r][c] = 1;
                else if (r <= 2) board[r][c] = 2;
                else board[r][c] = 0;
            } else {
                board[r][c] = 0;
            }
        }
    }
}

void reset_game(void) {
    init_board();
    isPlayerTurn = true;
    isIAthinking = false;
    firstMoveDone = false;
    showHeader = true;
    init_pool(&ai_pool);
    printf("🔄 Partita resettata! IA: %s\n", current_engine ? current_engine->name : "N/A");
}

void screen_to_grid(int mouseX, int mouseY, int *row, int *col) {
    *col = mouseX / CELL_SIZE;
    *row = mouseY / CELL_SIZE;
    if (*col < 0) *col = 0; if (*col >= 8) *col = 7;
    if (*row < 0) *row = 0; if (*row >= 8) *row = 7;
}

// Inizializza sistema IA + carica parametri da file
void init_ai_system(void) {
    // 1. Carica parametri generici da config.cfg (fallback su default)
    params_load("config.cfg");
    
    // 2. Registra le IA disponibili (esplicito per compatibilità MSVC)
    register_ucb1_base();
    
    if (ai_count() > 0) {
        const AIEngineDef** all = ai_list_all();
        current_engine = all[0];
        current_engine_idx = 0;
        
        // 3. Costruisci AIConfig usando parametri caricati (con fallback)
        AIConfig cfg = {
            .ucb_c = params_get_float("ucb_c", current_engine->default_cfg.ucb_c),
            .puct_c = params_get_float("puct_c", current_engine->default_cfg.puct_c),
            .time_limit = params_get_float("time_limit", current_engine->default_cfg.time_limit),
            .max_nodes = params_get_int("max_nodes", current_engine->default_cfg.max_nodes),
            .rollout_depth = params_get_int("rollout_depth", current_engine->default_cfg.rollout_depth),
            .use_heuristics = params_get_bool("use_heuristics", current_engine->default_cfg.use_heuristics)
        };
        
        current_ai_instance = current_engine->create(&cfg);
        aiTimeLimit = cfg.time_limit;
        headerColor = current_engine->header_color;
        
        printf("🚀 IA avviata: %s | ucb_c=%.3f | time=%.2fs | nodes=%d\n",
               current_engine->name, cfg.ucb_c, cfg.time_limit, cfg.max_nodes);
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama - Intelligent Web");
    SetTargetFPS(60);
    
    init_board();
    init_pool(&ai_pool);
    init_ai_system();

    while (!WindowShouldClose()) {
        // Toggle header con Q
        if (IsKeyPressed(KEY_Q)) showHeader = !showHeader;

        Vector2 mouse = GetMousePosition();
        screen_to_grid((int)mouse.x, (int)mouse.y, &hoverRow, &hoverCol);

        // Pulsante selezione IA (in alto a destra)
        Rectangle oppBtn = {GetScreenWidth() - 140, 8, 130, 24};

        // === MENU: Click sul pulsante apre menu e PAUSA il gioco ===
        if (showHeader && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, oppBtn)) {
            showAIMenu = true;
            int count = ai_count();
            float item_h = 30;
            float menu_h = count * item_h + 40;
            aiMenuRect = (Rectangle){SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - menu_h/2, 240, menu_h};
        }

        // === GESTIONE MENU (se aperto, il gioco è in pausa) ===
        if (showAIMenu) {
            const AIEngineDef** all = ai_list_all();
            int count = ai_count();
            float item_h = 30;
            
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                bool selected = false;
                for (int i = 0; i < count; i++) {
                    Rectangle opt = {aiMenuRect.x + 20, aiMenuRect.y + 35 + i*item_h, aiMenuRect.width - 40, item_h - 5};
                    if (CheckCollisionPointRec(mouse, opt)) {
                        if (current_ai_instance && current_engine) {
                            current_engine->destroy(current_ai_instance);
                        }
                        current_engine_idx = i;
                        current_engine = all[i];
                        
                        // Ricrea istanza con parametri aggiornati
                        AIConfig cfg = {
                            .ucb_c = params_get_float("ucb_c", current_engine->default_cfg.ucb_c),
                            .puct_c = params_get_float("puct_c", current_engine->default_cfg.puct_c),
                            .time_limit = params_get_float("time_limit", current_engine->default_cfg.time_limit),
                            .max_nodes = params_get_int("max_nodes", current_engine->default_cfg.max_nodes),
                            .rollout_depth = params_get_int("rollout_depth", current_engine->default_cfg.rollout_depth),
                            .use_heuristics = params_get_bool("use_heuristics", current_engine->default_cfg.use_heuristics)
                        };
                        
                        current_ai_instance = current_engine->create(&cfg);
                        aiTimeLimit = cfg.time_limit;
                        headerColor = current_engine->header_color;
                        showAIMenu = false;
                        reset_game();
                        selected = true;
                        break;
                    }
                }
                if (!selected) showAIMenu = false;
            }
        }

        // === LOGICA DI GIOCO (solo se menu CHIUSO) ===
        if (!showAIMenu) {
            // INPUT: Inizio drag
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

                // AUTO-CATTURA: Diagonali verso il BASSO (bianco scende: riga +1)
                int dr = 1;
                int dc_opts[2] = {-1, 1};

                for(int i = 0; i < 2; i++) {
                    int midR = dragFromRow + dr;
                    int midC = dragFromCol + dc_opts[i];
                    int landR = dragFromRow + 2 * dr;
                    int landC = dragFromCol + 2 * dc_opts[i];

                    if(landR >= 0 && landR < 8 && landC >= 0 && landC < 8) {
                        if((board[midR][midC] == 2 || board[midR][midC] == 4) && board[landR][landC] == 0) {
                            board[dragFromRow][dragFromCol] = 0;
                            board[midR][midC] = 0;
                            board[landR][landC] = 1;
                            finalRow = landR; finalCol = landC;
                            successo = true;
                            printf("✅ Cattura: nera in (%d,%d) rimossa!\n", midR, midC);
                            break;
                        }
                    }
                }

                if(!successo) {
                    int destRow = hoverRow; int destCol = hoverCol;
                    if(dragFromRow != destRow || dragFromCol != destCol) {
                        if(dama(board, dragFromRow, dragFromCol, destRow, destCol)) {
                            printf("✅ Mossa Dama!\n"); successo = true; finalRow = destRow; finalCol = destCol;
                        } else if(move(board, dragFromRow, dragFromCol, destRow, destCol)) {
                            printf("✅ Mossa Pedina!\n"); successo = true; finalRow = destRow; finalCol = destCol;
                        } else {
                            printf("❌ Mossa non valida.\n");
                        }
                    }
                }

                if(successo) {
                    check_promotion(board, finalRow, finalCol);
                    if (!firstMoveDone) { firstMoveDone = true; showHeader = false; }
                    isPlayerTurn = false; isIAthinking = true;
                }
                isDragging = false; dragFromRow = -1; dragFromCol = -1;
            }

            // LOGICA IA (Modulare: chiama engine->get_move)
            if (!isPlayerTurn && isIAthinking) {
                static clock_t ai_start = 0;
                static Bitboard ai_state;

                if (ai_start == 0) {
                    ai_start = clock();
                    board_to_bitboard(board, &ai_state);
                }

                if (((clock() - ai_start) / (float)CLOCKS_PER_SEC) >= aiTimeLimit) {
                    Move best = current_engine->get_move(current_ai_instance, &ai_state, aiTimeLimit);
                    apply_ai_move(board, best.from/8, best.from%8, best.to/8, best.to%8);
                    check_promotion(board, best.to/8, best.to%8);
                    isPlayerTurn = true; isIAthinking = false; ai_start = 0;
                }
            }
        }

        // === RENDERING ===
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 1. Scacchiera
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                Color cell = ((r + c) % 2 == 0) ? (Color){240, 217, 181, 255} : (Color){181, 136, 99, 255};
                DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cell);
                if (isDragging && r == hoverRow && c == hoverCol) {
                    DrawRectangle(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color){0, 255, 0, 80});
                }
            }
        }

        // 2. Pedine ferme
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                int piece = board[r][c];
                if (piece == 0 || (isDragging && r == dragFromRow && c == dragFromCol)) continue;
                Vector2 center = {(float)(c*CELL_SIZE + CELL_SIZE/2), (float)(r*CELL_SIZE + CELL_SIZE/2)};
                Color fc, bc;
                if (piece==1){fc=WHITE;bc=DARKGRAY;} 
                else if(piece==3){fc=GOLD;bc=ORANGE;}
                else if(piece==2){fc=BLACK;bc=GRAY;} 
                else if(piece==4){fc=DARKPURPLE;bc=VIOLET;}
                else continue;
                DrawCircleV(center, PIECE_RADIUS, fc);
                DrawCircleLinesV(center, PIECE_RADIUS, bc);
            }
        }

        // 3. Fantasma pedina trascinata
        if (isDragging) {
            DrawCircleV(mouse, PIECE_RADIUS, (Color){255, 255, 255, 180});
            DrawCircleLinesV(mouse, PIECE_RADIUS, RED);
        }

        // 4. HEADER (condizionale)
        if (showHeader) {
            int headerH = 40;
            DrawRectangle(0, 0, SCREEN_WIDTH, headerH, headerColor);

            DrawRectangleRec(oppBtn, CheckCollisionPointRec(mouse, oppBtn) ? (Color){60,60,90,200} : (Color){40,40,60,200});
            DrawRectangleLinesEx(oppBtn, 1, (Color){255,255,255,150});
            DrawText("CAMBIA IA", oppBtn.x + 25, oppBtn.y + 5, 12, WHITE);

            const char* aiName = current_engine ? current_engine->name : "Dama AI";
            Color nameColor = (current_engine_idx == 0) ? (Color){125, 211, 252, 255} : (Color){255, 140, 0, 255};
            DrawText(aiName, (SCREEN_WIDTH - MeasureText(aiName, 24))/2, 10, 24, nameColor);

            int turnX = 15, turnY = headerH + 8;
            if (isPlayerTurn) {
                DrawText("TOCCA A TE!", turnX + 1, turnY + 1, 20, BLACK);
                DrawText("TOCCA A TE!", turnX, turnY, 20, GOLD);
            } else {
                DrawText("IA PENSANDO...", turnX + 1, turnY + 1, 20, BLACK);
                DrawText("IA PENSANDO...", turnX, turnY, 20, LIGHTGRAY);
            }
        }

        // 5. MENU OVERLAY
        if (showAIMenu) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 160});
            DrawRectangleRec(aiMenuRect, (Color){35, 35, 35, 255});
            DrawRectangleLinesEx(aiMenuRect, 2, WHITE);
            DrawText("SELEZIONA IA", aiMenuRect.x + 70, aiMenuRect.y + 18, 14, WHITE);

            const AIEngineDef** all = ai_list_all();
            int count = ai_count();
            float item_h = 30;
            
            for (int i = 0; i < count; i++) {
                Rectangle opt = {aiMenuRect.x + 20, aiMenuRect.y + 35 + i*item_h, aiMenuRect.width - 40, item_h - 5};
                bool hover = CheckCollisionPointRec(mouse, opt);
                DrawRectangleRec(opt, hover ? (Color){70, 70, 110, 255} : (Color){50, 50, 50, 255});
                
                const char* suffix = (i == current_engine_idx) ? " ← ATTIVA" : "";
                char label[64];
                snprintf(label, sizeof(label), "%s%s", all[i]->name, suffix);
                DrawText(label, opt.x + 15, opt.y + 8, 12, (i == current_engine_idx) ? GOLD : WHITE);
            }
        }

        EndDrawing();
    }

    // Pulizia finale
    if (current_ai_instance && current_engine) {
        current_engine->destroy(current_ai_instance);
    }
    params_save("config.cfg"); // Salva eventuali modifiche runtime

    CloseWindow();
    return 0;
}