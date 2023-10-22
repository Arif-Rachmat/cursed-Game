#include <curses.h>


typedef struct
{
    const char* title;
    void (*function)(WINDOW*);
}GAME;

void init(int, char**);
void displayMenu(int);

GAME game[] ={
};
static int game_amount;

int main(int args, char *argv[]){
    int key,selection=0;
    bool running = true;
    game_amount = sizeof(game)/sizeof(GAME);

    init(args, argv);
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    raw();
    displayMenu(selection);
    
    while(running){
        key = getch();
        switch(key){
            case KEY_UP:
                if(--selection<0) selection = game_amount-1;
                displayMenu(selection);
                break;
            case KEY_DOWN:
                if(++selection>=game_amount) selection = 0;
                displayMenu(selection);
                break;
            case KEY_RESIZE:
                erase();
                resize_term(0,0);
                if(LINES<20 || COLS<70)
                    printw("WARNING : Terminal Size Must Be 70x20 minimum");
                else
                    displayMenu(selection);
                break;
            case 13:
            case 10:
            case KEY_ENTER:
                game[selection].function(stdscr);
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
        }
    }

    endwin();
    return 0;
}


void init(int argc, char** argv){
#ifdef XCURSES
    Xinit(argc, argv);
#else
    initscr();
#endif
#ifdef A_COLOR
    if(has_colors()) start_color();
#endif
}


void displayMenu(int menum){
    WINDOW* win;
    win = newwin(20,70,(LINES-20)/2, (COLS-70)/2);
    
    wborder(win,186,186,205,205,201,187,200,188);
    mvwaddstr(win,0,(70-32)/2," CONSOLAS SUPER GAME COLLECTION ");
    for(int i=0; i<game_amount; i++){
        mvwaddstr(win, 5+i, 3, game[i].title);
    }
    wattron(win,A_REVERSE);
    mvwaddstr(win,5+menum,3,game[menum].title);
    wattron(win,A_NORMAL);

    refresh();
    wrefresh(win);
}