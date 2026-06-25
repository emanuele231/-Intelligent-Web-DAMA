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

// Statistiche aggregate per le 8 IA
int engine_captures[8] = {0};
int engine_wins[8] = {0};
bool show_report = false;

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
static float match_time_limit = 0.5f;
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

    for (int i = 4; i < MAX_MATCHES; i++) {
        matches[i].id = i;
        matches[i].round_name = "TBD";
        matches[i].p1 = NULL;
        matches[i].p2 = NULL;
        matches[i].winner = NULL;
        matches[i].completed = false;
        sprintf(matches[i].result_text, "In attesa...");
    }
}

// ============================================================================
// UI DRAWING
// ============================================================================
static void draw_header() {
    DrawRectangle(0, 0, SCREEN_WIDTH, 60, (Color){25, 25, 40, 255});
    DrawText("TORNEO DAMA ITALIANA - ELIMINAZIONE DIRETTA", 200, 18, 22, (Color){100, 200, 255, 255});

    char status[128];
    if (tournament_finished) {
        sprintf(status, "Completato! Vincitore: %s", matches[6].winner ? matches[6].winner->name : "N/A");
    }
    else if (tournament_running) {
        int display_match = (current_match < MAX_MATCHES) ? current_match + 1 : MAX_MATCHES;
        sprintf(status, "In corso... Match %d/%d", display_match, MAX_MATCHES);
    }
    else {
        sprintf(status, "Pronto per l'avvio - 8 IA in gara");
    }
    DrawText(status, 280, 42, 14, LIGHTGRAY);
}

static void draw_match(int idx, float x, float y, float w, float h) {
    TournamentMatch* m = &matches[idx];

    Color bg_col = tournament_running && current_match == idx ?
                   (Color){50, 60, 80, 255} : (Color){40, 40, 55, 255};
    DrawRectangle(x, y, w, h, bg_col);
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 1, (Color){100, 100, 120, 255});

    DrawText(m->round_name, x + 8, y + 4, 10, (Color){150, 150, 180, 255});

    const char* name1 = m->p1 ? m->p1->name : "TBD";
    Color c1 = (m->winner == m->p1) ? GOLD : WHITE;
    DrawText(name1, x + 10, y + 20, 11, c1);

    const char* name2 = m->p2 ? m->p2->name : "TBD";
    Color c2 = (m->winner == m->p2) ? GOLD : WHITE;
    DrawText(name2, x + 10, y + 38, 11, c2);

    DrawText("VS", x + w/2 - 8, y + 28, 9, DARKGRAY);
    DrawText(m->result_text, x + w - 110, y + 28, 9, LIGHTGRAY);
}

static void draw_bracket_ui() {
    draw_match(0, 50, 100, 200, 70);
    draw_match(1, 50, 190, 200, 70);
    draw_match(2, 50, 300, 200, 70);
    draw_match(3, 50, 390, 200, 70);

    draw_match(4, 300, 145, 200, 70);
    draw_match(5, 300, 345, 200, 70);

    draw_match(6, 550, 145, 250, 70);
    draw_match(7, 550, 345, 250, 70);

    Color line_col = (Color){100, 100, 120, 150};
    DrawLine(250, 135, 300, 180, line_col); DrawLine(250, 225, 300, 180, line_col);
    DrawLine(250, 335, 300, 380, line_col); DrawLine(250, 425, 300, 380, line_col);
    DrawLine(500, 180, 550, 180, line_col); DrawLine(500, 380, 550, 180, line_col);
}

static void draw_report() {
    int sorted[8];
    for (int i = 0; i < 8; i++) sorted[i] = i;

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7 - i; j++) {
            if (engine_captures[sorted[j]] < engine_captures[sorted[j + 1]]) {
                int tmp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = tmp;
            }
        }
    }

    float x = 100, y = 100, w = 700, h = 500;
    DrawRectangle(x, y, w, h, (Color){25, 25, 40, 255});
    DrawRectangleLinesEx((Rectangle){x, y, w, h}, 2, (Color){100, 200, 255, 255});
    DrawText("CLASSIFICA TORNEO - PUNTI CAPTURE", 230, y + 15, 20, GOLD);

    DrawLine(x + 20, y + 45, x + w - 20, y + 45, (Color){150, 150, 150, 100});
    DrawText("POS", x + 25, y + 30, 14, WHITE);
    DrawText("IA", x + 70, y + 30, 14, WHITE);
    DrawText("VITTORIE", x + 320, y + 30, 14, WHITE);
    DrawText("CATTURE", x + 480, y + 30, 14, WHITE);
    DrawText("PUNTI", x + 620, y + 30, 14, WHITE);

    for (int i = 0; i < 8; i++) {
        int idx = sorted[i];
        float row_y = y + 65 + i * 45;

        char pos[6]; sprintf(pos, "#%d", i + 1);
        DrawText(pos, x + 30, row_y, 16, i < 3 ? GOLD : WHITE);

        DrawText(ai_list_all()[idx]->name, x + 70, row_y, 16, WHITE);

        char win_str[8]; sprintf(win_str, "%d", engine_wins[idx]);
        DrawText(win_str, x + 330, row_y, 16, LIGHTGRAY);

        char cap_str[12]; sprintf(cap_str, "%d", engine_captures[idx]);
        DrawText(cap_str, x + 490, row_y, 16, (Color){0, 255, 100, 255});

        DrawText(cap_str, x + 630, row_y, 18, GOLD);

        DrawLine(x + 20, row_y + 35, x + w - 20, row_y + 35, (Color){80, 80, 100, 80});
    }
}

static void draw_buttons(bool* start, bool* exit, bool* toggle_report) {
    Rectangle btn_start = {300, 620, 160, 40};
    Rectangle btn_report = {470, 620, 160, 40};
    Rectangle btn_exit = {640, 620, 160, 40};

    const char* btn_text;
    Color col_start;

    if (tournament_finished) {
        btn_text = "COMPLETATO";
        col_start = (Color){80, 80, 80, 200};
    }
    else if (tournament_running) {
        btn_text = "IN CORSO...";
        col_start = (Color){80, 80, 80, 200};
    }
    else {
        btn_text = "AVVIA TORNEO";
        col_start = (Color){0, 180, 80, 220};
    }

    DrawRectangleRec(btn_start, col_start);
    DrawRectangleLinesEx(btn_start, 2, WHITE);
    DrawText(btn_text, btn_start.x + 40, btn_start.y + 12, 16, WHITE);

    Color col_rep = show_report ? (Color){0, 150, 200, 220} : (Color){60, 60, 80, 200};
    DrawRectangleRec(btn_report, col_rep);
    DrawRectangleLinesEx(btn_report, 2, WHITE);
    DrawText(show_report ? "TABELLONE" : "REPORT", btn_report.x + 35, btn_report.y + 12, 16, WHITE);

    DrawRectangleRec(btn_exit, (Color){180, 50, 50, 220});
    DrawRectangleLinesEx(btn_exit, 2, WHITE);
    DrawText("ESCI", btn_exit.x + 55, btn_exit.y + 12, 16, WHITE);

    if (!tournament_running && !tournament_finished &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn_start)) {
        *start = true;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn_report)) {
        *toggle_report = !(*toggle_report);
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
        if (start_req && !tournament_running) {
            tournament_running = true;
            tournament_finished = false;
            show_report = false;

            memset(engine_captures, 0, sizeof(engine_captures));
            memset(engine_wins, 0, sizeof(engine_wins));
        }

        if (tournament_running && current_match < MAX_MATCHES) {
            TournamentMatch* m = &matches[current_match];

            if (!m->p1 || !m->p2) {
                current_match++;
            }
            else if (!m->completed) {
                int caps1 = 0, caps2 = 0;
                double res = play_tournament_game(m->p1, m->p2, match_time_limit, max_moves_limit, &caps1, &caps2);

                if (res >= 0.9) { m->winner = m->p1; sprintf(m->result_text, "Vittoria %s", m->p1->name); }
                else if (res <= 0.1) { m->winner = m->p2; sprintf(m->result_text, "Vittoria %s", m->p2->name); }
                else { m->winner = m->p1; sprintf(m->result_text, "Patta (Vince %s)", m->p1->name); }

                // Trova indici delle IA per aggiornare statistiche
                int idx1 = -1, idx2 = -1;
                for (int i = 0; i < 8; i++) {
                    if (all_engines[i] == m->p1) idx1 = i;
                    if (all_engines[i] == m->p2) idx2 = i;
                }
                if (idx1 != -1) { engine_captures[idx1] += caps1; if (res >= 0.9) engine_wins[idx1]++; }
                if (idx2 != -1) { engine_captures[idx2] += caps2; if (res <= 0.1) engine_wins[idx2]++; }

                m->completed = true;
                current_match++;

                if (current_match == 4) {
                    matches[4].round_name = "Semifinale 1";
                    matches[4].p1 = matches[0].winner;
                    matches[4].p2 = matches[1].winner;

                    matches[5].round_name = "Semifinale 2";
                    matches[5].p1 = matches[2].winner;
                    matches[5].p2 = matches[3].winner;
                }

                if (current_match == 6) {
                    matches[6].round_name = "FINALE";
                    matches[6].p1 = matches[4].winner;
                    matches[6].p2 = matches[5].winner;

                    matches[7].round_name = "Finale 3° Posto";
                    matches[7].p1 = (matches[4].winner == matches[4].p1) ? matches[4].p2 : matches[4].p1;
                    matches[7].p2 = (matches[5].winner == matches[5].p1) ? matches[5].p2 : matches[5].p1;
                }

                if (current_match == MAX_MATCHES) {
                    tournament_running = false;
                    tournament_finished = true;
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){15, 15, 25, 255});

        draw_header();

        if (show_report) draw_report();
        else draw_bracket_ui();

        draw_buttons(&start_req, &exit_req, &show_report);

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