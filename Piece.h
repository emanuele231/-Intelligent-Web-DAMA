#ifndef PIECES_H
#define PIECES_H

#include "raylib.h"

// Enum per il colore della pedina
typedef enum {
    PIECE_BLACK = 0,
    PIECE_WHITE = 1
} PieceColor;

// Enum per il tipo (pedina o dama)
typedef enum {
    PIECE_NORMAL = 0,
    PIECE_KING = 1
} PieceType;

// Struct che rappresenta una pedina
typedef struct {
    PieceColor color;      // Nero o Bianco
    PieceType type;        // Pedina o Dama
    int row;               // Posizione riga (0-7)
    int col;               // Posizione colonna (0-7)
    bool active;           // true = in gioco, false = catturata
} Piece;

// Funzioni di inizializzazione
void piece_init(Piece* p, PieceColor color, int row, int col);
void piece_promote_to_king(Piece* p);

// Funzioni di rendering
void piece_draw(Piece* p, int cell_size);
void piece_draw_highlight(Piece* p, int cell_size, Color highlight_color);

// Funzioni di utilità
bool piece_is_king(Piece* p);
PieceColor piece_get_color(Piece* p);

#endif // PIECES_H