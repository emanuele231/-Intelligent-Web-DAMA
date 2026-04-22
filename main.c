#include <stdio.h>

// Codici ANSI per colori di sfondo e reset
#define BG_BLACK "\033[40m"
#define BG_RED   "\033[41m"
#define BG_WHITE "\033[47m"
#define RESET    "\033[0m"

// Caratteri per le celle (2 caratteri per mantenere l'allineamento)
#define CELL_DARK "[ ]"
#define CELL_LIGHT "   "

void draw_board(void) {
    printf("\n    A  B  C  D  E  F  G  H\n");
    for (int row = 0; row < 8; row++) {
        printf(" %d ", row + 1);
        for (int col = 0; col < 8; col++) {
            // Le celle scure nella dama sono quelle con (riga + colonna) dispari
            if ((row + col) % 2 != 0) {
                printf("%s%s%s", BG_RED, CELL_DARK, RESET);
            } else {
                printf("%s%s%s", BG_BLACK, CELL_LIGHT, RESET);
            }
        }
        printf(" %d\n", row + 1);
    }
    printf("    A  B  C  D  E  F  G  H\n\n");
}

int main(void) {
    draw_board();
    return 0;
}