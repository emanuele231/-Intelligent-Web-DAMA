#include "raylib.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_WIDTH / BOARD_SIZE)

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dama - Intelligent Web");
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Disegna la scacchiera
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                Color cellColor = ((row + col) % 2 == 0) ? 
                    (Color){240, 217, 181, 255} :  // Beige chiaro
                    (Color){181, 136, 99, 255};   // Marrone
                
                DrawRectangle(col * CELL_SIZE, row * CELL_SIZE, 
                            CELL_SIZE, CELL_SIZE, cellColor);
            }
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}