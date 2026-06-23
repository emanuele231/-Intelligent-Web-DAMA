@echo off
echo Compiling Tournament System...

gcc tournament_main.c tournament.c ai_engine.c mcts_core.c variants.c params.c moves.c bitboard.c -Iinclude .\lib\libraylib.a -lopengl32 -lgdi32 -lwinmm -lm -o tournament.exe

if %errorlevel% equ 0 (
    echo Compilation successful!
    echo Run with: tournament.exe
) else (
    echo Compilation failed!
)