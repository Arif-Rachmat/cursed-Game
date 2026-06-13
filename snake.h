#pragma once
#include <ncurses/curses.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
    int x;
    int y;
} Position;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

void Snake(WINDOW *);
void draw(WINDOW *);
void processGame(WINDOW *);
void spawnFood();