#include <curses.h>
#include <stdlib.h> //noetig fuer atexit()

void quit()
{
  endwin();
}

int main(void)
{
  int x, y;

  initscr();
  atexit(quit);
  curs_set(0);
  
  mvprintw(3, 5, "LINES: %d", LINES);
  mvprintw(4, 5, "COLS:  %d", COLS);

  getyx(stdscr, y, x);
  mvprintw(5, 5, "Momentane Cursorposition:  [%d, %d]", y, x);

  getbegyx(stdscr, y, x);
  mvprintw(6, 5, "Koordinatenursprung:       [%d, %d]", y, x);

  getmaxyx(stdscr, y, x);
  mvprintw(7, 5, "Fenstergröße:              [%d, %d]", y, x);

  mvaddstr(11, 2, "Taste drücken -> Ende");
  refresh();

  getch();
  return(0);
}
