#include "snake.h"

static unsigned short snake_len, MAX_SNAKE_LEN, LOGICAL_WIDTH, LOGICAL_HEIGHT, WIN_WIDTH, WIN_HEIGHT;
static Position *snake;
static Direction dir, last_dir;
static Position food;
static unsigned int score;
static bool game_over;

void initGame(void) {
    snake_len = 3;
    int start_x = LOGICAL_WIDTH / 2;
    int start_y = LOGICAL_HEIGHT / 2;
    
    // Position initial snake moving rightwards
    for (int i = 0; i < snake_len; i++) {
        snake[i].x = start_x - i;
        snake[i].y = start_y;
    }
    
    dir = DIR_RIGHT;
    last_dir = DIR_RIGHT;
    score = 0;
    game_over = false;
 
    spawnFood();
}

void Snake(WINDOW *parent_win) {
    LOGICAL_WIDTH = 30;
    LOGICAL_HEIGHT = 18;
    WIN_WIDTH = (LOGICAL_WIDTH * 2) + 2;        // TODO: Make this adjustable at runtime through menu/settings
    WIN_HEIGHT = LOGICAL_HEIGHT + 2;
    MAX_SNAKE_LEN = LOGICAL_WIDTH * LOGICAL_HEIGHT;
    snake = malloc(sizeof(Position) * MAX_SNAKE_LEN);

    wclear(parent_win);
    wrefresh(parent_win);

    int max_y, max_x;
    getmaxyx(parent_win, max_y, max_x);

    int start_y = (max_y - WIN_HEIGHT) / 2;
    int start_x = (max_x - WIN_WIDTH) / 2;

    WINDOW *game_win = newwin(WIN_HEIGHT, WIN_WIDTH, start_y, start_x);
    keypad(game_win, TRUE);
    nodelay(game_win, TRUE); // Setup non-blocking inputs
    curs_set(0);

    if (has_colors()) {
        if (COLORS >= 256) {
            init_pair(1, 46, COLOR_BLACK);   // Body: Bright Green text
            init_pair(2, 22, COLOR_BLACK);   // Head: Dark Green text
            init_pair(3, 196, COLOR_BLACK);  // Food: Bright Red text
        } else {
            // Standard 8-color terminal fallback
            init_pair(1, COLOR_GREEN, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_RED, COLOR_BLACK);
        }
    }

    initGame();

    bool running = true;
    clock_t lastUpdate = clock();
    clock_t tick_rate = (clock_t)(0.12 * CLOCKS_PER_SEC);

    while (running) {
        int ch = wgetch(game_win);
        switch (ch) {
            case KEY_UP:    if (last_dir != DIR_DOWN)   dir = DIR_UP;    break;
            case KEY_DOWN:  if (last_dir != DIR_UP)     dir = DIR_DOWN;  break;
            case KEY_LEFT:  if (last_dir != DIR_RIGHT)  dir = DIR_LEFT;  break;
            case KEY_RIGHT: if (last_dir != DIR_LEFT)   dir = DIR_RIGHT; break;
            case 27:        running = false;                             break; // ESC key
        }

        if (!running) break;

        if (game_over) {
            nodelay(game_win, FALSE);
            wattron(game_win, COLOR_PAIR(3));
            mvwprintw(game_win, WIN_HEIGHT / 2, (WIN_WIDTH - 26) / 2, "GAME OVER! Final Score: %d", score);
            wattroff(game_win, COLOR_PAIR(3));
            mvwprintw(game_win, (WIN_HEIGHT / 2) + 1, (WIN_WIDTH - 28) / 2, "Press any key to return...");
            wrefresh(game_win);
            wgetch(game_win); 
            running = false;
        } else {
            clock_t current_time = clock();
            if (current_time - lastUpdate >= tick_rate) {
                processGame(game_win);
                draw(game_win);       
                lastUpdate = current_time;
            }
        }
        napms(10); // Throttle thread loops to prevent high CPU utilization
    }

    free(snake);
    delwin(game_win);
    wclear(parent_win);
    wrefresh(parent_win);
}

void processGame(WINDOW *win) {
    Position next_head = snake[0];
    switch (dir) {
        case DIR_UP:    
            next_head.y--;
            last_dir = DIR_UP;
            break;
        case DIR_DOWN:  
            next_head.y++;
            last_dir = DIR_DOWN;
            break;
        case DIR_LEFT:  
            next_head.x--;
            last_dir = DIR_LEFT;
            break;
        case DIR_RIGHT: 
            next_head.x++;
            last_dir = DIR_RIGHT;
            break;
    }

    // Border Collision Check against logical boundary lines
    if (next_head.x < 0 || next_head.x >= LOGICAL_WIDTH ||
        next_head.y < 0 || next_head.y >= LOGICAL_HEIGHT) {
        game_over = true;
        return;
    }

    // Self Collision Check
    for (int i = 0; i < snake_len; i++) {
        if (next_head.x == snake[i].x && next_head.y == snake[i].y) {
            game_over = true;
            return;
        }
    }

    bool eating = (next_head.x == food.x && next_head.y == food.y);
    int segments_to_move = eating ? snake_len : snake_len - 1;

    for (int i = segments_to_move; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = next_head;

    if (eating) {
        if (snake_len < MAX_SNAKE_LEN) snake_len++;
        score += 10;
        
        spawnFood();
    }
}

void spawnFood() {
    bool on_snake;
    do {
        on_snake = false;
        food.x = rand() % LOGICAL_WIDTH;
        food.y = rand() % LOGICAL_HEIGHT;
        for (int i = 0; i < snake_len; i++) {
            if (food.x == snake[i].x && food.y == snake[i].y) {
                on_snake = true;
                break;
            }
        }
    } while (on_snake);
}

void draw(WINDOW *win) {
    werase(win);  
    box(win, 0, 0);

    mvwprintw(win, 0, 3, " Score: %04d ", score);
    mvwprintw(win, 0, WIN_WIDTH - 15, " esc to quit ");

    // Food Block
    wattron(win, COLOR_PAIR(3) | A_REVERSE);
    mvwaddstr(win, food.y + 1, (food.x * 2) + 1, "  ");
    wattroff(win, COLOR_PAIR(3) | A_REVERSE);

    // Snake Body Segments
    wattron(win, COLOR_PAIR(1) | A_REVERSE);
    for (int i = 1; i < snake_len; i++) {
        mvwaddstr(win, snake[i].y + 1, (snake[i].x * 2) + 1, "  ");
    }
    wattroff(win, COLOR_PAIR(1) | A_REVERSE);
    
    // Head Block
    wattron(win, COLOR_PAIR(2) | A_REVERSE);
    mvwaddstr(win, snake[0].y + 1, (snake[0].x * 2) + 1, "  ");
    wattroff(win, COLOR_PAIR(2) | A_REVERSE);

    wrefresh(win);
}