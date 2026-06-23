#define NOGDI
#define WIN32_LEAN_AND_MEAN

#include "raylib.h"
#include "ai_engine.h"
#include "mcts_core.h"
#include "tournament.h"
#include "params.h"
#include "params.h"
#include "variants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 700

//stato torneo
typedef struct {
    bool running;
    bool finished;
    int current_match;
    int total_matches;
    TournamentResult* results;
    int num_results;
    char status_msg[128];
} TournamentState;

static TournamentState t_state = {0};

//FUNZIONI UI

static void draw_header(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, 60, (Color){30, 30, 50, 255});
    DrawText("TORNEO INTELLIGENZE ARTIFICIALI - DAMA", 180, 15, 24, (Color){100, 200, 255, 255});
    DrawText("Round-Robin Tournament", 280, 42, 14, LIGHTGRAY);
}

//sfondo
static void draw_config_panel(void) {
    Rectangle panel = {50, 80, 700, 120};
    DrawRectangleRec(panel, (Color){40, 40, 60, 255});
    DrawRectangleLinesEx(panel, 2, (Color){100, 150, 255, 255});
    
    DrawText("CONFIGURAZIONE TORNEO", panel.x + 20, panel.y + 10, 18, WHITE);
    DrawText("Partite per coppia:", panel.x + 20, panel.y + 45, 14, LIGHTGRAY);
    DrawText("Tempo per mossa:", panel.x + 250, panel.y + 45, 14, LIGHTGRAY);
    DrawText("Max mosse:", panel.x + 450, panel.y + 45, 14, LIGHTGRAY);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", params_get_int("num_games_per_pair", 20));
    DrawText(buf, panel.x + 180, panel.y + 45, 14, WHITE);
    
    snprintf(buf, sizeof(buf), "%.1fs", params_get_float("time_limit_per_move", 0.2f));
    DrawText(buf, panel.x + 380, panel.y + 45, 14, WHITE);
    
    snprintf(buf, sizeof(buf), "%d", params_get_int("max_moves", 60));
    DrawText(buf, panel.x + 560, panel.y + 45, 14, WHITE);
}

//barra progresso partita
static void draw_progress_bar(int current, int total) {
    Rectangle bar_bg = {50, 220, 700, 30};
    DrawRectangleRec(bar_bg, (Color){50, 50, 70, 255});
    DrawRectangleLinesEx(bar_bg, 2, WHITE);
    
    if (total > 0) {
        float pct = (float)current / total;
        Rectangle bar_fill = {52, 222, (bar_bg.width - 4) * pct, bar_bg.height - 4};
        DrawRectangleRec(bar_fill, (Color){0, 180, 100, 255});
    }
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Match %d / %d", current, total);
    DrawText(msg, bar_bg.x + 300, bar_bg.y + 8, 16, WHITE);
}

//risultati partite
static void draw_standings(TournamentResult* results, int count) {
    Rectangle table = {50, 270, 700, 350};
    DrawRectangleRec(table, (Color){35, 35, 50, 255});
    DrawRectangleLinesEx(table, 2, (Color){150, 150, 200, 255});
    
    // Header tabella
    DrawRectangle(52, 272, 696, 28, (Color){50, 50, 70, 255});
    DrawText("POS", 60, 278, 14, WHITE);
    DrawText("IA", 110, 278, 14, WHITE);
    DrawText("VITTORIE", 350, 278, 14, WHITE);
    DrawText("PATTE", 450, 278, 14, WHITE);
    DrawText("SCONFITTE", 530, 278, 14, WHITE);
    DrawText("WIN RATE", 650, 278, 14, WHITE);
    
    // Righe classifica
    if (results && count > 0) {
        // Ordina per vittorie (bubble sort semplice)
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (results[j].wins < results[j+1].wins) {
                    TournamentResult tmp = results[j];
                    results[j] = results[j+1];
                    results[j+1] = tmp;
                }
            }
        }
        
        for (int i = 0; i < count && i < 10; i++) {
            int y = 305 + i * 30;
            Color row_color = (i % 2 == 0) ? (Color){45, 45, 65, 200} : (Color){40, 40, 60, 200};
            DrawRectangle(52, y, 696, 28, row_color);
            
            char buf[32];
            
            // Posizione
            snprintf(buf, sizeof(buf), "#%d", i + 1);
            DrawText(buf, 60, y + 8, 14, i < 3 ? GOLD : WHITE);
            
            // Nome IA
            DrawText(results[i].engine_name, 110, y + 8, 14, WHITE);
            
            // Vittorie
            snprintf(buf, sizeof(buf), "%d", results[i].wins);
            DrawText(buf, 360, y + 8, 14, (Color){0, 255, 100, 255});
            
            // Patte
            snprintf(buf, sizeof(buf), "%d", results[i].draws);
            DrawText(buf, 460, y + 8, 14, LIGHTGRAY);
            
            // Sconfitte
            snprintf(buf, sizeof(buf), "%d", results[i].losses);
            DrawText(buf, 540, y + 8, 14, (Color){255, 100, 100, 255});
            
            // Win rate
            int total = results[i].wins + results[i].losses + results[i].draws;
            float rate = total > 0 ? (float)results[i].wins / total * 100 : 0;
            snprintf(buf, sizeof(buf), "%.1f%%", rate);
            DrawText(buf, 650, y + 8, 14, rate > 50 ? GOLD : WHITE);
        }
    }
}

//PULSANTI

static void draw_buttons(bool* start, bool* export_csv) {
    Rectangle btn_start = {50, 635, 200, 50};
    Rectangle btn_export = {300, 635, 200, 50};
    Rectangle btn_exit = {550, 635, 200, 50};
    
    // Pulsante START
    Color start_color = t_state.running ? (Color){80, 80, 80, 200} : (Color){0, 180, 80, 220};
    DrawRectangleRec(btn_start, start_color);
    DrawRectangleLinesEx(btn_start, 2, WHITE);
    DrawText(t_state.running ? "IN CORSO..." : "AVVIA TORNEO", btn_start.x + 35, btn_start.y + 16, 18, WHITE);
    
    // Pulsante EXPORT
    DrawRectangleRec(btn_export, t_state.finished ? (Color){0, 150, 200, 220} : (Color){80, 80, 80, 200});
    DrawRectangleLinesEx(btn_export, 2, WHITE);
    DrawText("ESPORTA CSV", btn_export.x + 45, btn_export.y + 16, 18, t_state.finished ? WHITE : LIGHTGRAY);
    
    // Pulsante EXIT
    DrawRectangleRec(btn_exit, (Color){180, 50, 50, 220});
    DrawRectangleLinesEx(btn_exit, 2, WHITE);
    DrawText("TORNA AL GIOCO", btn_exit.x + 30, btn_exit.y + 16, 18, WHITE);
    
    // Gestione click
    if (!t_state.running && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        CheckCollisionPointRec(GetMousePosition(), btn_start)) {
        *start = true;
    }
    
    if (t_state.finished && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        CheckCollisionPointRec(GetMousePosition(), btn_export)) {
        *export_csv = true;
    }
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        CheckCollisionPointRec(GetMousePosition(), btn_exit)) {
        // Torna al gioco principale
        #ifdef _WIN32
            system("start dama.exe");
        #else
            system("./dama.exe &");
        #endif
        exit(0);
    }
}

//apertura seconda scena
int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama Tournament System");
    SetTargetFPS(60);
    
    // Carica parametri
    params_load("config.cfg");
    
    // Registra tutte le IA
    register_all_variants();
    
    printf("=== TORNEO IA ===\n");
    printf("IA registrate: %d\n", ai_count());
    
    // Inizializza stato torneo
    t_state.results = calloc(ai_count(), sizeof(TournamentResult));
    t_state.num_results = ai_count();
    
    // Inizializza nomi
    const AIEngineDef** all = ai_list_all();
    for (int i = 0; i < ai_count(); i++) {
        t_state.results[i].engine_name = all[i]->name;
        t_state.results[i].wins = 0;
        t_state.results[i].losses = 0;
        t_state.results[i].draws = 0;
        t_state.results[i].avg_time_per_move = 0;
    }
    
    bool start_tournament = false;
    bool export_csv = false;
    
    while (!WindowShouldClose()) {
        // Avvio torneo
        if (start_tournament && !t_state.running) {
            t_state.running = true;
            t_state.finished = false;
            
            // Esegui torneo
            int num_games = params_get_int("num_games_per_pair", 10);
            float time_limit = params_get_float("time_limit_per_move", 0.2f);
            
            run_tournament_headless(all, ai_count(), num_games, time_limit, t_state.results);
            
            t_state.running = false;
            t_state.finished = true;
            strcpy(t_state.status_msg, "Torneo completato!");
        }
        
        // Export CSV
        if (export_csv && t_state.finished) {
            export_tournament_csv(t_state.results, t_state.num_results, "tournament_results.csv");
            export_csv = false;
            strcpy(t_state.status_msg, "Risultati esportati in tournament_results.csv");
        }
        
        // === RENDERING ===
        BeginDrawing();
        ClearBackground((Color){20, 20, 30, 255});
        
        draw_header();
        draw_config_panel();
        
        if (t_state.running || t_state.finished) {
            draw_progress_bar(t_state.current_match, t_state.total_matches);
            draw_standings(t_state.results, t_state.num_results);
        }
        
        draw_buttons(&start_tournament, &export_csv);
        
        // Status message
        if (t_state.status_msg[0] != '\0') {
            DrawText(t_state.status_msg, 50, 630, 14, (Color){100, 255, 100, 255});
        }
        
        DrawText("Premi ESC per chiudere", 600, 665, 12, DARKGRAY);
        
        EndDrawing();
    }
    
    // Pulizia
    if (t_state.results) free(t_state.results);
    CloseWindow();
    
    return 0;
}