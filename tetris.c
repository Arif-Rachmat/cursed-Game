#include "tetris.h"

static const int tetris_pieces[7][4][4][4] = {
    // 0: I (Cyan)
    {
        { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0} },
        { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0} }
    },
    // 1: O (Yellow)
    {
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }
    },
    // 2: T (Magenta)
    {
        { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    },
    // 3: S (Green)
    {
        { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
        { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
        { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    },
    // 4: Z (Red)
    {
        { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0} },
        { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0} }
    },
    // 5: J (Blue)
    {
        { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0} }
    },
    // 6: L (Orange/White fallback)
    {
        { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0} },
        { {1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    }
};

static unsigned short BOARD_WIDTH, BOARD_HEIGHT, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, PREV_WIN_WIDTH, PREV_WIN_HEIGHT;
static int** board;
static int current_type, current_rot, current_x, current_y;
static int next_type;
static unsigned int score;
static bool game_over;

static bool checkCollision(int type, int rot, int bx, int by);
static void spawnPiece(void);
static void lockPiece(void);
static void clearLines(void);
static void drawGame(WINDOW *m_win, WINDOW *p_win);

static bool checkCollision(int type, int rot, int bx, int by) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (tetris_pieces[type][rot][r][c]) {
                int target_x = bx + c;
                int target_y = by + r;

                if (target_x < 0 || target_x >= BOARD_WIDTH || target_y >= BOARD_HEIGHT) {
                    return true;
                }
                if (target_y >= 0 && board[target_y][target_x] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

static void spawnPiece(void) {
    current_type = next_type;
    next_type = rand() % 7;
    current_rot = 0;
    current_x = BOARD_WIDTH / 2 - 2;
    current_y = 0;

    if (checkCollision(current_type, current_rot, current_x, current_y)) {
        game_over = true;
    }
}

static void lockPiece(void) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (tetris_pieces[current_type][current_rot][r][c]) {
                int target_x = current_x + c;
                int target_y = current_y + r;
                if (target_y >= 0 && target_y < BOARD_HEIGHT && target_x >= 0 && target_x < BOARD_WIDTH) {
                    board[target_y][target_x] = current_type + 4; // Map color pairs starting at 4
                }
            }
        }
    }
}

static void clearLines(void) {
    int consecutive_clears = 0;
    for (int r = BOARD_HEIGHT - 1; r >= 0; r--) {
        bool is_full = true;
        for (int c = 0; c < BOARD_WIDTH; c++) {
            if (board[r][c] == 0) {
                is_full = false;
                break;
            }
        }
        if (is_full) {
            consecutive_clears++;
            for (int m = r; m > 0; m--) {
                for (int c = 0; c < BOARD_WIDTH; c++) {
                    board[m][c] = board[m - 1][c];
                }
            }
            for (int c = 0; c < BOARD_WIDTH; c++) {
                board[0][c] = 0;
            }
            r++; // Re-evaluate the current row line step
        }
    }
    if (consecutive_clears > 0) {
        score += consecutive_clears * 100 * consecutive_clears; // Exponential score rewards
    }
}

void Tetris(WINDOW *parent_win) {
    BOARD_HEIGHT = 20;
    BOARD_WIDTH = 10;
    MAIN_WIN_HEIGHT = (BOARD_HEIGHT + 2);   //TODO : Make this adjustable at runtime through menu/settings
    MAIN_WIN_WIDTH = (BOARD_WIDTH * 2 + 2);
    PREV_WIN_HEIGHT = 6;
    PREV_WIN_WIDTH = 14;

    board = malloc(BOARD_HEIGHT * sizeof(int*));
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        board[i] = malloc(BOARD_WIDTH * sizeof(int));
    }

    wclear(parent_win);
    wrefresh(parent_win);

    int max_y, max_x;
    getmaxyx(parent_win, max_y, max_x);

    int total_width = MAIN_WIN_WIDTH + 2 + PREV_WIN_WIDTH;
    int start_y = (max_y - MAIN_WIN_HEIGHT) / 2;
    int start_x_main = (max_x - total_width) / 2;
    int start_x_prev = start_x_main + MAIN_WIN_WIDTH + 2;

    WINDOW *game_win = newwin(MAIN_WIN_HEIGHT, MAIN_WIN_WIDTH, start_y, start_x_main);
    WINDOW *preview_win = newwin(PREV_WIN_HEIGHT, PREV_WIN_WIDTH, start_y, start_x_prev);

    keypad(game_win, TRUE);
    nodelay(game_win, TRUE);
    curs_set(0);

    if (has_colors()) {
        if (COLORS >= 256) {
            init_pair(4, 51,  COLOR_BLACK);  // I: Cyan
            init_pair(5, 226, COLOR_BLACK);  // O: Yellow
            init_pair(6, 201, COLOR_BLACK);  // T: Magenta
            init_pair(7, 46,  COLOR_BLACK);  // S: Green
            init_pair(8, 196, COLOR_BLACK);  // Z: Red
            init_pair(9, 21,  COLOR_BLACK);  // J: Blue
            init_pair(10, 208, COLOR_BLACK); // L: Orange
        } else {
            init_pair(4, COLOR_CYAN,    COLOR_BLACK);
            init_pair(5, COLOR_YELLOW,  COLOR_BLACK);
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(7, COLOR_GREEN,   COLOR_BLACK);
            init_pair(8, COLOR_RED,     COLOR_BLACK);
            init_pair(9, COLOR_BLUE,    COLOR_BLACK);
            init_pair(10, COLOR_WHITE,  COLOR_BLACK);
        }
    }

    // Initialize board state to clear structures
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }

    srand(time(NULL));
    score = 0;
    game_over = false;
    next_type = rand() % 7;
    spawnPiece();

    bool running = true;
    clock_t lastUpdate = clock();
    clock_t drop_rate = (clock_t)(0.45 * CLOCKS_PER_SEC); // Standard gravity drop interval speed

    while (running) {
        int ch = wgetch(game_win);
        switch (ch) {
            case KEY_LEFT:
                if (!checkCollision(current_type, current_rot, current_x - 1, current_y)) {
                    current_x--;
                }
                break;
            case KEY_RIGHT:
                if (!checkCollision(current_type, current_rot, current_x + 1, current_y)) {
                    current_x++;
                }
                break;
            case KEY_UP: { // Rotate Block
                int next_rot = (current_rot + 1) % 4;
                if (!checkCollision(current_type, next_rot, current_x, current_y)) {
                    current_rot = next_rot;
                }
                break;
            }
            case KEY_DOWN: // Soft Drop acceleration
                if (!checkCollision(current_type, current_rot, current_x, current_y + 1)) {
                    current_y++;
                }
                break;
            case ' ': { // Hard Drop shortcut
                while (!checkCollision(current_type, current_rot, current_x, current_y + 1)) {
                    current_y++;
                }
                lockPiece();
                clearLines();
                spawnPiece();
                lastUpdate = clock();
                break;
            }
            case 27: // ESC key
                running = false;
                break;
        }

        if (!running) break;

        if (game_over) {
            nodelay(game_win, FALSE);
            wattron(game_win, COLOR_PAIR(8));
            mvwprintw(game_win, MAIN_WIN_HEIGHT / 2, (MAIN_WIN_WIDTH - 11) / 2, "GAME OVER!");
            wattroff(game_win, COLOR_PAIR(8));
            mvwprintw(game_win, (MAIN_WIN_HEIGHT / 2) + 1, (MAIN_WIN_WIDTH - 16) / 2, "Score: %05d", score);
            wrefresh(game_win);
            wgetch(game_win); 
            running = false;
        } else {
            clock_t current_time = clock();
            if (current_time - lastUpdate >= drop_rate) {
                if (!checkCollision(current_type, current_rot, current_x, current_y + 1)) {
                    current_y++;
                } else {
                    lockPiece();
                    clearLines();
                    spawnPiece();
                }
                lastUpdate = current_time;
            }
            drawGame(game_win, preview_win);
        }
        napms(10); // Throttle performance thread cycles
    }

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        free(board[i]);
    }
    free(board);

    delwin(game_win);
    delwin(preview_win);
    wclear(parent_win);
    wrefresh(parent_win);
}

static void drawGame(WINDOW *m_win, WINDOW *p_win) {
    werase(m_win);
    box(m_win, 0, 0);
    mvwprintw(m_win, 0, 2, " Score: %05d ", score);

    // Render anchored static board elements
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] != 0) {
                wattron(m_win, COLOR_PAIR(board[y][x]) | A_REVERSE);
                mvwaddstr(m_win, y + 1, (x * 2) + 1, "  ");
                wattroff(m_win, COLOR_PAIR(board[y][x]) | A_REVERSE);
            }
        }
    }

    if (!game_over) {
        wattron(m_win, COLOR_PAIR(current_type + 4) | A_REVERSE);
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (tetris_pieces[current_type][current_rot][r][c]) {
                    int draw_y = current_y + r;
                    int draw_x = current_x + c;
                    if (draw_y >= 0 && draw_y < BOARD_HEIGHT && draw_x >= 0 && draw_x < BOARD_WIDTH) {
                        mvwaddstr(m_win, draw_y + 1, (draw_x * 2) + 1, "  ");
                    }
                }
            }
        }
        wattroff(m_win, COLOR_PAIR(current_type + 4) | A_REVERSE);
    }
    wrefresh(m_win);

    werase(p_win);
    box(p_win, 0, 0);
    mvwprintw(p_win, 0, 2, " NEXT ");

    wattron(p_win, COLOR_PAIR(next_type + 4) | A_REVERSE);
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (tetris_pieces[next_type][0][r][c]) {
                // Offset calculation to center blocks gracefully inside the 14x6 side-box frame
                int render_x = (c * 2) + 3;
                if (next_type == 0) render_x = (c * 2) + 2; // Extra visual shift adjustment for long 'I' piece
                mvwaddstr(p_win, r + 1, render_x, "  ");
            }
        }
    }
    wattroff(p_win, COLOR_PAIR(next_type + 4) | A_REVERSE);
    wrefresh(p_win);
}