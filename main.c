#include <ncurses/curses.h>
#include "collection.h"

typedef struct
{
    const char *title;          // Selected game title/name
    void (*function)(WINDOW *); // Function of the selected game in the list

} GAME;

void init(int, char **); // initialized main curses window
void displayMenu(int);   // display main menu with all available item whenever the screen need to refresh

GAME game[] = {
    {"Snake", Snake},
    {"Tetris", Tetris}
}; // list available game titles and function name

static int game_amount; // number of game in the list

int main(int args, char *argv[])
{
    int selection = 0;
    bool running = true;
    game_amount = sizeof(game) / sizeof(GAME);

    init(args, argv);
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    raw();
    displayMenu(selection);

    while (running)
    {
        switch (getch())
        {
        case KEY_UP:
            if (--selection < 0)
                selection = game_amount - 1;
            displayMenu(selection);
            break;

        case KEY_DOWN:
            if (++selection >= game_amount)
                selection = 0;
            displayMenu(selection);
            break;

        case 13:
        case 10:
        case KEY_ENTER:
            clear();
            game[selection].function(stdscr);
            displayMenu(selection);
            break;

        case KEY_RESIZE:
            erase();
            resize_term(0, 0);
            displayMenu(selection);
            break;

        case 'q':
        case 'Q':
        case 27:
            running = false;
            break;
        }
    }

    endwin();
    return 0;
}

void init(int argc, char **argv)
{
#ifdef XCURSES
    Xinit(argc, argv);
#else
    initscr();
#endif
#ifdef A_COLOR
    if (has_colors())
        start_color();
#endif
}

void displayMenu(int menum)
{
    if (LINES < 20 || COLS < 70)
        printw("WARNING : Terminal Size Must Be 70x20 minimum");
    else
    {
        WINDOW *win;
        win = newwin(20, 70, (LINES - 20) / 2, (COLS - 70) / 2);

        box(win, 0, 0);
        mvwaddstr(win, 0, (70 - 32) / 2, " CONSOLAS SUPER GAME COLLECTION ");
        for (int i = 0; i < game_amount; i++)
        {
            mvwaddstr(win, 5 + i, 3, game[i].title);
        }
        wattron(win, A_REVERSE);
        mvwaddstr(win, 5 + menum, 3, game[menum].title);
        wattron(win, A_NORMAL);

        refresh();
        wrefresh(win);
    }
}
