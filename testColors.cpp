#include <ncurses.h>
int main() {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, COLOR_YELLOW, -1);
    attron(COLOR_PAIR(1));
    printw("White text\n");
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    printw("Yellow text\n");
    attroff(COLOR_PAIR(2));
    refresh();
    getch();
    endwin();
    return 0;
}