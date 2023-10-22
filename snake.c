#include <curses.h>




void Snake(WINDOW* win){
    int key;
    bool running = true;
    while (running)
    {
        key = getch();
        switch (key)
        {
        case KEY_RESIZE:
            /* code */
            break;
        
        default:
            break;
        }
    }
    
}