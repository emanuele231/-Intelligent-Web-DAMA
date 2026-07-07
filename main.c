#define NOGDI
#define WIN32_LEAN_AND_MEAN

#include "raylib.h"
#include "moves.h"
#include <time.h>
#include "bitboard.h"
#include "ai_engine.h"
#include "UCB1.h"
#include "params.h"
#include "tournament.h"
#include "mcts_core.h"
#include "variants.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)
#define PIECE_RADIUS (CELL_SIZE * 0.35f)

// Limiti ufficiali della Dama Italiana
#define MAX_TOTAL_MOVES 400
#define MAX_IDLE_MOVES 40

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
bool showConfigMenu = false;

// Selezione temporanea
int temp_engine_idx = 0;
float temp_time = 0.2f;

// Board: 0=vuota, 1=bianco, 2=nero, 3=dama bianca, 4=dama nera
int board[8][8] = {0};

// Drag & drop
bool isDragging = false;
int dragFromRow = -1, dragFromCol = -1;
int hoverRow = -1, hoverCol = -1;

// === TERMINAZIONE PARTITA ===
bool gameOver = false;
int winner = 0;              // 0=nessuno, 1=bianco, 2=nero, 3=patta
char gameOverMessage[128] = "";
int total_moves = 0;
int moves_without_capture = 0;

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
    gameOver = false;
    winner = 0;
    total_moves = 0;
    moves_without_capture = 0;
    gameOverMessage[0] = '\0';
    init_pool(&ai_pool);
    printf("Partita resettata! IA: %s\n", current_engine ? current_engine->name : "N/A");
}

void screen_to_grid(int mouseX, int mouseY, int *row, int *col) {
    *col = mouseX / CELL_SIZE;
    *row = mouseY / CELL_SIZE;
    if (*col < 0) *col = 0; if (*col >= 8) *col = 7;
    if (*row < 0) *row = 0; if (*row >= 8) *row = 7;
}

void init_ai_system(void) {
    params_load("config.cfg");
    register_all_variants();
    
    if (ai_count() > 0) {
        const AIEngineDef** all = ai_list_all();
        current_engine = all[0];
        current_engine_idx = 0;
        
        AIConfig cfg = current_engine->default_cfg;
        current_ai_instance = current_engine->create(&cfg);
        aiTimeLimit = cfg.time_limit;
        headerColor = current_engine->header_color;
        
        printf("IA avviata: %s | ucb_c=%.3f | time=%.2fs | nodes=%d\n",
               current_engine->name, cfg.ucb_c, cfg.time_limit, cfg.max_nodes);
    }
}

// Conta pezzi sulla board
void count_pieces_on_board(int *white_count, int *black_count) {
    *white_count = 0;
    *black_count = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == 1 || board[r][c] == 3) (*white_count)++;
            if (board[r][c] == 2 || board[r][c] == 4) (*black_count)++;
        }
    }
}

// Controlla le condizioni di fine partita
// Restituisce true se la partita è finita, impostando winner e message
bool check_game_over(void) {
    int w_count = 0, b_count = 0;
    count_pieces_on_board(&w_count, &b_count);
    
    // 1. Vittoria per cattura totale
    if (w_count == 0) {
        winner = 2;
        sprintf(gameOverMessage, "NERO VINCE!\n(Bianco senza pezzi)");
        printf("\n=== NERO VINCE! (Bianco senza pezzi) ===\n");
        return true;
    }
    if (b_count == 0) {
        winner = 1;
        sprintf(gameOverMessage, "BIANCO VINCE!\n(Nero senza pezzi)");
        printf("\n=== BIANCO VINCE! (Nero senza pezzi) ===\n");
        return true;
    }
    
    // 2. Vittoria per blocco totale
    // Se tocca al Bianco e non ha mosse legali -> Nero vince
    if (isPlayerTurn && !has_legal_moves(board, 1)) {
        winner = 2;
        sprintf(gameOverMessage, "NERO VINCE!\n(Bianco bloccato)");
        printf("\n=== NERO VINCE! (Bianco bloccato) ===\n");
        return true;
    }
    // Se tocca al Nero e non ha mosse legali -> Bianco vince
    if (!isPlayerTurn && !isIAthinking && !has_legal_moves(board, 2)) {
        winner = 1;
        sprintf(gameOverMessage, "BIANCO VINCE!\n(Nero bloccato)");
        printf("\n=== BIANCO VINCE! (Nero bloccato) ===\n");
        return true;
    }
    
    // 3. Patta per stallo (40 mosse senza catture)
    if (moves_without_capture >= MAX_IDLE_MOVES) {
        winner = 3;
        sprintf(gameOverMessage, "PATTA!\n(%d mosse senza catture)", moves_without_capture);
        printf("\n=== PATTA! (%d mosse senza catture) ===\n", moves_without_capture);
        return true;
    }
    
    // 4. Patta per limite mosse totali (400)
    if (total_moves >= MAX_TOTAL_MOVES) {
        winner = 3;
        sprintf(gameOverMessage, "PATTA!\n(%d mosse totali)", total_moves);
        printf("\n=== PATTA! (%d mosse totali) ===\n", total_moves);
        return true;
    }
    
    return false;
}

// Disegna la schermata di game over
void draw_game_over_screen(void) {
    // Overlay scuro
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    // Box centrale
    int boxW = 400, boxH = 250;
    int boxX = (SCREEN_WIDTH - boxW) / 2;
    int boxY = (SCREEN_HEIGHT - boxH) / 2;
    
    Color boxColor;
    if (winner == 1) boxColor = (Color){255, 215, 0, 240};       // Oro (bianco vince)
    else if (winner == 2) boxColor = (Color){80, 40, 120, 240};  // Viola (nero vince)
    else boxColor = (Color){100, 100, 100, 240};                  // Grigio (patta)
    
    DrawRectangle(boxX, boxY, boxW, boxH, boxColor);
    DrawRectangleLinesEx((Rectangle){boxX, boxY, boxW, boxH}, 3, WHITE);
    
    // Titolo
    const char* title = "";
    if (winner == 1) title = "VITTORIA BIANCO!";
    else if (winner == 2) title = "VITTORIA NERO!";
    else title = "PARTITA PAREGGIATA";
    
    int titleW = MeasureText(title, 28);
    DrawText(title, (SCREEN_WIDTH - titleW) / 2, boxY + 30, 28, WHITE);
    
    // Messaggio dettagliato (su più righe se necessario)
    Color msgColor = (winner == 3) ? LIGHTGRAY : BLACK;
    int msgY = boxY + 90;
    
    // Semplice split su newline
    char* newline = strchr(gameOverMessage, '\n');
    if (newline) {
        *newline = '\0';
        int w1 = MeasureText(gameOverMessage, 20);
        DrawText(gameOverMessage, (SCREEN_WIDTH - w1) / 2, msgY, 20, msgColor);
        int w2 = MeasureText(newline + 1, 16);
        DrawText(newline + 1, (SCREEN_WIDTH - w2) / 2, msgY + 30, 16, msgColor);
        *newline = '\n';
    } else {
        int w = MeasureText(gameOverMessage, 20);
        DrawText(gameOverMessage, (SCREEN_WIDTH - w) / 2, msgY, 20, msgColor);
    }
    
    // Statistiche partita
    char stats[128];
    sprintf(stats, "Mosse totali: %d / %d", total_moves, MAX_TOTAL_MOVES);
    int statsW = MeasureText(stats, 14);
    DrawText(stats, (SCREEN_WIDTH - statsW) / 2, boxY + 160, 14, LIGHTGRAY);
    
    // Pulsante "Nuova Partita"
    Rectangle btnRestart = {boxX + 100, boxY + 190, 200, 45};
    bool hoverBtn = CheckCollisionPointRec(GetMousePosition(), btnRestart);
    DrawRectangleRec(btnRestart, hoverBtn ? (Color){0, 180, 80, 255} : (Color){0, 140, 60, 255});
    DrawRectangleLinesEx(btnRestart, 2, WHITE);
    DrawText("NUOVA PARTITA", btnRestart.x + 40, btnRestart.y + 14, 18, WHITE);
    
    // Click sul pulsante
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverBtn) {
        reset_game();
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
        if (IsKeyPressed(KEY_Q)) showHeader = !showHeader;

        Vector2 mouse = GetMousePosition();
        screen_to_grid((int)mouse.x, (int)mouse.y, &hoverRow, &hoverCol);

        Rectangle oppBtn = {GetScreenWidth() - 140, 8, 130, 24};

        // === MENU: Click sul pulsante apre menu ===
        if (showHeader && !gameOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, oppBtn)) {
            showConfigMenu = true;
            temp_engine_idx = current_engine_idx;
            temp_time = aiTimeLimit;
        }

        // === SCHERMATA CONFIGURAZIONE IA ===
        if (showConfigMenu) {
            BeginDrawing();
            ClearBackground((Color){20, 20, 30, 255});
            
            DrawText("CONFIGURA INTELLIGENZA ARTIFICIALE", SCREEN_WIDTH/2 - 180, 20, 24, (Color){100, 150, 255, 255});
            DrawText("Seleziona variante e tempo di analisi", SCREEN_WIDTH/2 - 140, 50, 16, LIGHTGRAY);
            
            const AIEngineDef** all = ai_list_all();
            int count = ai_count();
            
            // UCB1
            DrawRectangle(60, 90, 240, 320, (Color){40, 50, 70, 255});
            DrawRectangleLines(60, 90, 240, 320, (Color){125, 211, 252, 255});
            DrawText("UCB1 VARIANTS", 110, 95, 18, (Color){125, 211, 252, 255});
            
            const char* ucb_names[] = {"Classic", "Delta", "Alpha", "Fast"};
            Rectangle ucb_btns[4];
            for (int i = 0; i < 4 && i < count; i++) {
                ucb_btns[i] = (Rectangle){70, 125 + i*65, 220, 55};
                bool selected = (temp_engine_idx == i);
                DrawRectangleRec(ucb_btns[i], selected ? (Color){0, 100, 200, 220} : (Color){50, 60, 80, 200});
                DrawRectangleLinesEx(ucb_btns[i], selected ? 2 : 1, selected ? WHITE : (Color){125, 211, 252, 255});
                DrawText(ucb_names[i], ucb_btns[i].x + 10, ucb_btns[i].y + 10, 16, selected ? WHITE : (Color){150, 200, 255, 255});
                
                const char* desc = "";
                if (i == 0) desc = "Standard (C=1.414)";
                else if (i == 1) desc = "Delta (confidenza)";
                else if (i == 2) desc = "Alpha (C variabile)";
                else if (i == 3) desc = "Ottimizzato veloce";
                DrawText(desc, ucb_btns[i].x + 10, ucb_btns[i].y + 32, 11, LIGHTGRAY);
            }
            
            // PUCT
            DrawRectangle(340, 90, 240, 320, (Color){70, 50, 40, 255});
            DrawRectangleLines(340, 90, 240, 320, (Color){255, 140, 0, 255});
            DrawText("PUCT VARIANTS", 390, 95, 18, (Color){255, 140, 0, 255});
            
            const char* puct_names[] = {"Standard", "Explorative", "Heuristic", "Balanced"};
            Rectangle puct_btns[4];
            for (int i = 0; i < 4 && (4+i) < count; i++) {
                puct_btns[i] = (Rectangle){350, 125 + i*65, 220, 55};
                bool selected = (temp_engine_idx == 4+i);
                DrawRectangleRec(puct_btns[i], selected ? (Color){200, 100, 0, 220} : (Color){80, 60, 50, 200});
                DrawRectangleLinesEx(puct_btns[i], selected ? 2 : 1, selected ? WHITE : (Color){255, 140, 0, 255});
                DrawText(puct_names[i], puct_btns[i].x + 10, puct_btns[i].y + 10, 16, selected ? WHITE : (Color){255, 180, 100, 255});
                
                const char* desc = "";
                if (i == 0) desc = "Standard (C=1.2)";
                else if (i == 1) desc = "Alta esplorazione";
                else if (i == 2) desc = "Con euristiche";
                else if (i == 3) desc = "Bilanciato (C=1.2)";
                DrawText(desc, puct_btns[i].x + 10, puct_btns[i].y + 32, 11, LIGHTGRAY);
            }
            
            // Tempo
            DrawRectangle(140, 430, 360, 70, (Color){40, 40, 50, 255});
            DrawRectangleLines(140, 430, 360, 70, WHITE);
            DrawText("TEMPO DI ANALISI", 245, 438, 16, WHITE);
            
            Rectangle btnT02 = {160, 460, 90, 35};
            Rectangle btnT1 = {275, 460, 90, 35};
            Rectangle btnT3 = {390, 460, 90, 35};
            
            bool t02 = (temp_time == 0.2f), t1 = (temp_time == 1.0f), t3 = (temp_time == 3.0f);
            
            DrawRectangleRec(btnT02, t02 ? (Color){0, 180, 100, 220} : (Color){60, 60, 70, 200});
            DrawText("0.2s", btnT02.x + 25, btnT02.y + 10, 16, t02 ? WHITE : LIGHTGRAY);
            
            DrawRectangleRec(btnT1, t1 ? (Color){0, 180, 100, 220} : (Color){60, 60, 70, 200});
            DrawText("1.0s", btnT1.x + 28, btnT1.y + 10, 16, t1 ? WHITE : LIGHTGRAY);
            
            DrawRectangleRec(btnT3, t3 ? (Color){0, 180, 100, 220} : (Color){60, 60, 70, 200});
            DrawText("3.0s", btnT3.x + 28, btnT3.y + 10, 16, t3 ? WHITE : LIGHTGRAY);
            
            // Pulsanti azione
            Rectangle btnApply = {180, 520, 120, 40};
            Rectangle btnCancel = {340, 520, 120, 40};
            
            DrawRectangleRec(btnApply, (Color){0, 180, 80, 220});
            DrawRectangleLinesEx(btnApply, 2, WHITE);
            DrawText("APPLICA", btnApply.x + 25, btnApply.y + 12, 18, WHITE);
            
            DrawRectangleRec(btnCancel, (Color){180, 50, 50, 220});
            DrawRectangleLinesEx(btnCancel, 2, WHITE);
            DrawText("ANNULLA", btnCancel.x + 20, btnCancel.y + 12, 18, WHITE);
            
            // Input menu
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int i = 0; i < 4 && i < count; i++) {
                    if (CheckCollisionPointRec(mouse, ucb_btns[i])) temp_engine_idx = i;
                }
                for (int i = 0; i < 4 && (4+i) < count; i++) {
                    if (CheckCollisionPointRec(mouse, puct_btns[i])) temp_engine_idx = 4 + i;
                }
                
                if (CheckCollisionPointRec(mouse, btnT02)) temp_time = 0.2f;
                if (CheckCollisionPointRec(mouse, btnT1)) temp_time = 1.0f;
                if (CheckCollisionPointRec(mouse, btnT3)) temp_time = 3.0f;
                
                if (CheckCollisionPointRec(mouse, btnApply)) {
                    if (current_ai_instance && current_engine) {
                        current_engine->destroy(current_ai_instance);
                    }
                    
                    AIConfig newCfg = {0};
                    newCfg.time_limit = temp_time;
                    newCfg.max_nodes = (int)(temp_time * 40000);
                    newCfg.use_heuristics = false;
                    
                    if (temp_engine_idx < 4) {
                        newCfg.algo = ALGO_UCB1_CLASSIC;
                        newCfg.ucb_c = 1.414f;
                        if (temp_engine_idx == 1) newCfg.algo = ALGO_UCB_DELTA;
                        if (temp_engine_idx == 2) newCfg.algo = ALGO_UCB_ALPHA;
                        if (temp_engine_idx == 3) newCfg.algo = ALGO_UCB_FAST;
                    } else {
                        newCfg.algo = ALGO_PUCT_STD;
                        newCfg.cpuct = 1.2f;
                        if (temp_engine_idx == 5) newCfg.algo = ALGO_PUCT_EXP;
                        if (temp_engine_idx == 6) {
                            newCfg.algo = ALGO_PUCT_HEUR;
                            newCfg.use_heuristics = true;
                        }
                        if (temp_engine_idx == 7) newCfg.algo = ALGO_PUCT_BAL;
                    }
                    
                    current_ai_instance = mcts_create(&newCfg);
                    current_engine_idx = temp_engine_idx;
                    current_engine = all[temp_engine_idx];
                    aiTimeLimit = temp_time;
                    headerColor = (temp_engine_idx < 4) ? 
                        (Color){0, 100, 200, 150} : (Color){200, 100, 0, 150};
                    
                    printf("IA configurata: %s | Tempo: %.1fs\n", current_engine->name, temp_time);
                    showConfigMenu = false;
                    reset_game();
                }
                
                if (CheckCollisionPointRec(mouse, btnCancel)) {
                    showConfigMenu = false;
                }
            }
            
            EndDrawing();
            continue;
        }

        // === LOGICA DI GIOCO (solo se partita non finita) ===
        if (!showConfigMenu && !gameOver) {
            // INPUT: Inizio drag
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isPlayerTurn) {
                int piece = board[hoverRow][hoverCol];
                if (piece == 1 || piece == 3) { 
                    isDragging = true;
                    dragFromRow = hoverRow;
                    dragFromCol = hoverCol;
                }
            }

            // INPUT: Rilascio mouse
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging && isPlayerTurn) {
                bool successo = false;
                int finalRow = -1, finalCol = -1;
                int piece = board[dragFromRow][dragFromCol];
                bool is_capture = false;
                
                bool must_capture = has_any_capture(board, 1);
                
                if (must_capture) {
                    int dr, dc, land_r, land_c;
                    if (find_best_capture(board, dragFromRow, dragFromCol, &dr, &dc, &land_r, &land_c)) {
                        int total_captured = 0;
                        if (execute_multi_capture(board, dragFromRow, dragFromCol, &finalRow, &finalCol, &total_captured)) {
                            printf("Presa multipla eseguita! %d pezzi catturati.\n", total_captured);
                            successo = true;
                            is_capture = true;
                        }
                    } else {
                        printf("CATTURA OBBLIGATORIA! Questa pedina/dama non puo catturare da qui.\n");
                    }
                } else {
                    int destRow = hoverRow;
                    int destCol = hoverCol;
                    
                    if (dragFromRow != destRow || dragFromCol != destCol) {
                        if (piece == 3) {
                            if (abs(destRow - dragFromRow) == 1 && abs(destCol - dragFromCol) == 1) {
                                if (board[destRow][destCol] == 0) {
                                    board[destRow][destCol] = 3;
                                    board[dragFromRow][dragFromCol] = 0;
                                    successo = true;
                                    finalRow = destRow;
                                    finalCol = destCol;
                                    printf("Mossa Dama!\n");
                                }
                            }
                        }
                        else if (piece == 1) {
                            if (move(board, dragFromRow, dragFromCol, destRow, destCol)) {
                                successo = true;
                                finalRow = destRow;
                                finalCol = destCol;
                                printf("Mossa Pedina!\n");
                            }
                        }
                    }
                }
                
                if (successo) {
                    if (finalRow != -1 && finalCol != -1) {
                        check_promotion(board, finalRow, finalCol);
                    }
                    
                    // Aggiorna contatori
                    total_moves++;
                    if (is_capture) {
                        moves_without_capture = 0;
                    } else {
                        moves_without_capture++;
                    }
                    
                    if (!firstMoveDone) { firstMoveDone = true; showHeader = false; }
                    isPlayerTurn = false;
                    isIAthinking = true;
                    
                    // Controlla fine partita DOPO mossa giocatore
                    if (check_game_over()) {
                        gameOver = true;
                    }
                }
                
                isDragging = false;
                dragFromRow = -1;
                dragFromCol = -1;
            }

            // LOGICA IA
            if (!isPlayerTurn && isIAthinking && !gameOver) {
                static clock_t ai_start = 0;
                static Bitboard ai_state;

                if (ai_start == 0) {
                    ai_start = clock();
                    board_to_bitboard(board, &ai_state);
                    ai_state.turn = 2; // Nero
                }

                if (((clock() - ai_start) / (float)CLOCKS_PER_SEC) >= aiTimeLimit) {
                    Move best = current_engine->get_move(current_ai_instance, &ai_state, aiTimeLimit);
                    
                    if (best.from != 255 && best.to != 255) {
                        int fromR = best.from / 8, fromC = best.from % 8;
                        int toR = best.to / 8, toC = best.to % 8;
                        
                        // Rileva se è cattura
                        bool is_capture = (abs(toR - fromR) == 2 && abs(toC - fromC) == 2);
                        
                        apply_ai_move(board, fromR, fromC, toR, toC);
                        check_promotion(board, toR, toC);
                        
                        // Aggiorna contatori
                        total_moves++;
                        if (is_capture) {
                            moves_without_capture = 0;
                        } else {
                            moves_without_capture++;
                        }
                        
                        isPlayerTurn = true;
                        isIAthinking = false;
                        ai_start = 0;
                        
                        // Controlla fine partita DOPO mossa IA
                        if (check_game_over()) {
                            gameOver = true;
                        }
                    } else {
                        // Mossa invalida -> il giocatore vince
                        printf("IA non ha mosse legali! Bianco vince.\n");
                        winner = 1;
                        sprintf(gameOverMessage, "BIANCO VINCE!\n(IA senza mosse)");
                        gameOver = true;
                        isIAthinking = false;
                        ai_start = 0;
                    }
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
                
                // Simbolo dama (corona)
                if (piece == 3 || piece == 4) {
                    DrawText("D", center.x - 6, center.y - 8, 14, piece == 3 ? MAROON : GOLD);
                }
            }
        }

        // 3. Fantasma pedina trascinata
        if (isDragging) {
            DrawCircleV(mouse, PIECE_RADIUS, (Color){255, 255, 255, 180});
            DrawCircleLinesV(mouse, PIECE_RADIUS, RED);
        }

        // 4. HEADER
        if (showHeader && !gameOver) {
            int headerH = 40;
            DrawRectangle(0, 0, SCREEN_WIDTH, headerH, headerColor);

            DrawRectangleRec(oppBtn, CheckCollisionPointRec(mouse, oppBtn) ? (Color){60,60,90,200} : (Color){40,40,60,200});
            DrawRectangleLinesEx(oppBtn, 1, (Color){255,255,255,150});
            DrawText("CAMBIA IA", oppBtn.x + 25, oppBtn.y + 5, 12, WHITE);

            const char* aiName = current_engine ? current_engine->name : "Dama AI";
            DrawText(aiName, (SCREEN_WIDTH - MeasureText(aiName, 24))/2, 10, 24, headerColor.r < 100 ? (Color){125, 211, 252, 255} : (Color){255, 140, 0, 255});

            int turnX = 15, turnY = headerH + 8;
            if (isPlayerTurn) {
                DrawText("TOCCA A TE!", turnX + 1, turnY + 1, 20, BLACK);
                DrawText("TOCCA A TE!", turnX, turnY, 20, GOLD);
            } else {
                DrawText("IA PENSANDO...", turnX + 1, turnY + 1, 20, BLACK);
                DrawText("IA PENSANDO...", turnX, turnY, 20, LIGHTGRAY);
            }
            
            // Contatori partita (in basso)
            char stats[64];
            sprintf(stats, "Mosse: %d/%d | Senza cattura: %d/%d", 
                    total_moves, MAX_TOTAL_MOVES, moves_without_capture, MAX_IDLE_MOVES);
            DrawRectangle(0, SCREEN_HEIGHT - 25, SCREEN_WIDTH, 25, (Color){0, 0, 0, 150});
            DrawText(stats, 10, SCREEN_HEIGHT - 20, 14, LIGHTGRAY);
        }
        
        // 5. Schermata game over
        if (gameOver) {
            draw_game_over_screen();
        }

        EndDrawing();
    }

    if (current_ai_instance && current_engine) {
        current_engine->destroy(current_ai_instance);
    }
    params_save("config.cfg");

    CloseWindow();
    return 0;
}