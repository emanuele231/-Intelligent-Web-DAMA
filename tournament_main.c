#define NOGDI
#define WIN32_LEAN_AND_MEAN

#include "raylib.h"
#include "variants.h"       
#include "tournament.h"
#include "ai_engine.h"
#include "mcts_core.h"
#include "params.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 750
#define MAX_MATCHES 8

typedef struct {
    int id;
    const char* round_name;
    const AIEngineDef* p1;
    const AIEngineDef* p2;
    const AIEngineDef* winner;
    bool completed;
    char result_text[64];
} TournamentMatch;

static TournamentMatch matches[MAX_MATCHES];
static int current_match = 0;
static bool tournament_running = false;
static bool tournament_finished = false;
static float match_time_limit = 0.5f; // Tempo per mossa
static int max_moves_limit = 40; 

static const char* short_names[] = {
    "UCB1-Classic", "UCB1-Delta", "UCB1-Alpha", "UCB1-Fast",
    "PUCT-Std", "PUCT-Expl", "PUCT-Heur", "PUCT-Bal"
};


// ============================================================================
// SETUP TABELLONE
// ============================================================================
static void setup_bracket(const AIEngineDef** engines) {
    for (int i = 0; i < 4; i++) {
        matches[i].id = i;
        matches[i].round_name = "Quarti";
        matches[i].p1 = engines[i * 2];
        matches[i].p2 = engines[i * 2 + 1];
        matches[i].winner = NULL;
        matches[i].completed = false;
        sprintf(matches[i].result_text, "In attesa...");
    }
}

static void advance_bracket() {
    // Semifinali (2 match)
    matches[4].id = 4; matches[4].round_name = "Semifinale 1";
    matches[4].p1 = matches[0].winner; matches[4].p2 = matches[1].winner;
    matches[4].winner = NULL; matches[4].completed = false;
    sprintf(matches[4].result_text, "In attesa...");

    matches[5].id = 5; matches[5].round_name = "Semifinale 2";
    matches[5].p1 = matches[2].winner; matches[5].p2 = matches[3].winner;
    matches[5].winner = NULL; matches[5].completed = false;
    sprintf(matches[5].result_text, "In attesa...");

    // Finale e 3° posto (2 match)
    matches[6].id = 6; matches[6].round_name = "FINALE";
    matches[6].p1 = matches[4].winner; matches[6].p2 = matches[5].winner;
    matches[6].winner = NULL; matches[6].completed = false;
    sprintf(matches[6].result_text, "In attesa...");

    matches[7].id = 7; matches[7].round_name = "Finale 3° Posto";
    // Loser SF1
    matches[7].p1 = (matches[4].p1 == matches[4].winner) ? matches[4].p2 : matches[4].p1;
    // Loser SF2
    matches[7].p2 = (matches[5].p1 == matches[5].winner) ? matches[5].p2 : matches[5].p1;
    matches[7].winner = NULL; matches[7].completed = false;
    sprintf(matches[7].result_text, "In attesa...");
}

// ============================================================================
// UI DRAWING
// ============================================================================
static void draw_header() {
    DrawRectangle(0, 0, SCREEN_WIDTH, 60, (Color){25, 25, 40, 255});
    DrawText("TORNEO DAMA ITALIANA - ELIMINAZIONE DIRETTA", 200, 18, 22, (Color){100, 200, 255, 255});
    
    char status[64];
    if (tournament_finished) sprintf(status, "Completato! Vincitore: %s", matches[6].winner ? matches[6].winner->name : "N/A");
    else if (tournament_running) sprintf(status, "In corso... Match %d/%d", current_match + 1, MAX_MATCHES);
    else sprintf(status, "Pronto per l'avvio");
    DrawText(status, 300, 42, 14, LIGHTGRAY);
}

static void draw_match(int idx, float x, float y, float w, float h) {
    TournamentMatch* m = &matches[idx];
    
    // Sfondo box
    Color bg_col = tournament_running && current_match == idx ? 
                   (Color){50, 60, 80, 255} : (Color){40, 40, 55, 255};
    DrawRectangle(x, y, w, h, bg_col);
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 1, (Color){100, 100, 120, 255});
    
    // Titolo round (più piccolo)
    DrawText(m->round_name, x + 8, y + 4, 10, (Color){150, 150, 180, 255});
    
    // Player 1 (sinistra, in alto)
    const char* name1 = m->p1 ? m->p1->name : "TBD";
    Color c1 = (m->winner == m->p1) ? GOLD : WHITE;
    DrawText(name1, x + 10, y + 20, 11, c1);
    
    // Player 2 (sinistra, in basso)
    const char* name2 = m->p2 ? m->p2->name : "TBD";
    Color c2 = (m->winner == m->p2) ? GOLD : WHITE;
    DrawText(name2, x + 10, y + 38, 11, c2);
    
    // VS (centro, piccolo)
    DrawText("VS", x + w/2 - 8, y + 28, 9, DARKGRAY);
    
    // Risultato (destra)
    DrawText(m->result_text, x + w - 110, y + 28, 9, LIGHTGRAY);
}

static void draw_bracket_ui() {
    // Quarti (sinistra)
    draw_match(0, 50, 100, 200, 70);
    draw_match(1, 50, 190, 200, 70);
    draw_match(2, 50, 300, 200, 70);
    draw_match(3, 50, 390, 200, 70);
    
    // Semifinali (centro)
    draw_match(4, 300, 145, 200, 70);
    draw_match(5, 300, 345, 200, 70);
    
    // Finali (destra)
    draw_match(6, 550, 145, 250, 70);
    draw_match(7, 550, 345, 250, 70);
    
    // Linee di connessione (semplificate)
    Color line_col = (Color){100, 100, 120, 150};
    // QF1+QF2 -> SF1
    DrawLine(250, 135, 300, 180, line_col); DrawLine(250, 225, 300, 180, line_col);
    // QF3+QF4 -> SF2
    DrawLine(250, 335, 300, 380, line_col); DrawLine(250, 425, 300, 380, line_col);
    // SF1+SF2 -> Final
    DrawLine(500, 180, 550, 180, line_col); DrawLine(500, 380, 550, 180, line_col);
}

static void draw_buttons(bool* start, bool* exit) {
    Rectangle btn_start = {350, 550, 200, 50};
    Rectangle btn_exit = {350, 620, 200, 50};
    
    Color col_start = tournament_running ? (Color){80, 80, 80, 200} : (Color){0, 180, 80, 220};
    DrawRectangleRec(btn_start, col_start);
    DrawRectangleLinesEx(btn_start, 2, WHITE);
    DrawText(tournament_running ? "IN CORSO..." : "AVVIA TORNEO", btn_start.x + 35, btn_start.y + 16, 18, WHITE);
    
    DrawRectangleRec(btn_exit, (Color){180, 50, 50, 220});
    DrawRectangleLinesEx(btn_exit, 2, WHITE);
    DrawText("TORNA AL GIOCO", btn_exit.x + 30, btn_exit.y + 16, 18, WHITE);
    
    if (!tournament_running && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn_start)) {
        *start = true;
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn_exit)) {
        *exit = true;
    }
}

// ============================================================================
// MAIN
// ============================================================================
int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama Tournament System");
    SetTargetFPS(60);
    
    params_load("config.cfg");
    register_all_variants();
    
    if (ai_count() < 8) {
        printf("Errore: Servono almeno 8 IA per il torneo. Ne trovate %d.\n", ai_count());
        return 1;
    }
    
    const AIEngineDef** all_engines = ai_list_all();
    setup_bracket(all_engines);
    
    bool start_req = false;
    bool exit_req = false;
    
    while (!WindowShouldClose()) {
        // Gestione avvio torneo
        if (start_req && !tournament_running) {
            tournament_running = true;
            tournament_finished = false;
        }
        
        // Esecuzione match step-by-step (NON blocca la UI)
        if (tournament_running && current_match < MAX_MATCHES) {
            TournamentMatch* m = &matches[current_match];
            if (m->p1 && m->p2 && !m->completed) {
                double res = play_tournament_game(m->p1, m->p2, match_time_limit, max_moves_limit);
                
                if (res >= 0.9) { m->winner = m->p1; sprintf(m->result_text, "Vittoria %s", m->p1->name); }
                else if (res <= 0.1) { m->winner = m->p2; sprintf(m->result_text, "Vittoria %s", m->p2->name); }
                else { m->winner = m->p1; sprintf(m->result_text, "Patta (Vince %s)", m->p1->name); }
                
                m->completed = true;
                current_match++;
                
                // Se finiti i quarti, genera semifinali e finali
                if (current_match == 4) advance_bracket();
                if (current_match == MAX_MATCHES) {
                    tournament_running = false;
                    tournament_finished = true;
                }
            }
        }
        
        // === RENDERING ===
        BeginDrawing();
        ClearBackground((Color){15, 15, 25, 255});
        
        draw_header();
        draw_bracket_ui();
        draw_buttons(&start_req, &exit_req);
        
        if (exit_req) {
            #ifdef _WIN32
                system("start main.exe");
            #else
                system("./main.exe &");
            #endif
            exit(0);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}