#include "Piece.h"
#include <stdio.h>

// Inizializza una pedina
void piece_init(Piece* p, PieceColor color, int row, int col) {
    p->color = color;
    p->type = PIECE_NORMAL;
    p->row = row;
    p->col = col;
    p->active = true;
}

// Promuove a dama
void piece_promote_to_king(Piece* p) {
    if (p->active) {
        p->type = PIECE_KING;
        printf("Pedina promossa a DAMA!\n");
    }
}

// Disegna la pedina
void piece_draw(Piece* p, int cell_size) {
    if (!p->active) return;
    
    float center_x = p->col * cell_size + cell_size / 2.0f;
    float center_y = p->row * cell_size + cell_size / 2.0f;
    float radius = cell_size / 3.0f;
    
    // Colore base
    Color base_color = (p->color == PIECE_BLACK) ? BLACK : WHITE;
    Color border_color = (p->color == PIECE_BLACK) ? DARKGRAY : LIGHTGRAY;
    
    // Disegna cerchio principale
    DrawCircle(center_x, center_y, radius, base_color);
    
    // Bordo
    DrawCircleLines(center_x, center_y, radius, border_color);
    
    // Se è una dama, disegna una corona o un cerchio interno
    if (p->type == PIECE_KING) {
        DrawCircle(center_x, center_y, radius * 0.5f, YELLOW);
        const char* label = (p->color == PIECE_BLACK) ? "♚" : "♔";
        DrawText(label, center_x - MeasureText(label, 20) / 2, center_y - 10, 20, (p->color == PIECE_BLACK) ? WHITE : BLACK);
    }
}

// Evidenzia la pedina selezionata
void piece_draw_highlight(Piece* p, int cell_size, Color highlight_color) {
    if (!p->active) return;
    
    float x = p->col * cell_size;
    float y = p->row * cell_size;
    
    // Cerchio evidenziato intorno alla pedina
    DrawCircleLines(x + cell_size / 2.0f, y + cell_size / 2.0f, cell_size / 2.5f, highlight_color);
}

// Controlla se è una dama
bool piece_is_king(Piece* p) {
    return p->type == PIECE_KING;
}

// Ottiene il colore
PieceColor piece_get_color(Piece* p) {
    return p->color;
}