#include <ncurses.h>
#include <sys/ioctl.h>

enum colours {
	COLOUR_RED = 1,
	COLOUR_GREEN = 2,
	COLOUR_BLUE = 3,
	COLOUR_CYAN = 4,
	COLOUR_MAGENTA = 5,
	COLOUR_YELLOW = 6,
	COLOUR_WHITE = 7,
	COLOUR_BLACK = 8
};

typedef struct _panel {
	WINDOW* instruction_window;
    WINDOW* debug_window;
    WINDOW* register_window;
    WINDOW* info_window;
	int width;
	int height;
} PANEL;


void draw_borders(WINDOW *screen) {
    int x, y, i; getmaxyx(screen, y, x);
    mvwprintw(screen, 0, 0, "+");
    mvwprintw(screen, 0, x - 1, "+");
    for (i = 1; i < (x - 1); i++) {
        mvwprintw(screen, 0, i, "-");
    }
    wrefresh(screen);
}

void print_window_titles(PANEL* panel) {
    int iwx = 0, iwy = 0;
    int idx = 0, idy = 0;
    int irox = 0, iroy = 0;
    int irex = 0, irey = 0;

    getyx(panel->instruction_window, iwy, iwx);
    getyx(panel->debug_window, idx, idy);
    getyx(panel->info_window, irox, iroy);
    getyx(panel->register_window, irex, irey);

    draw_borders(panel->instruction_window);
    wattron(panel->instruction_window, COLOR_PAIR(4));
    mvwprintw(panel->instruction_window, 0, 1, "instructions");
    wattroff(panel->instruction_window, COLOR_PAIR(4));
    wrefresh(panel->instruction_window);

    draw_borders(panel->debug_window);
    wattron(panel->debug_window, COLOR_PAIR(4));
    mvwprintw(panel->debug_window, 0, 1, "debug");
    wattroff(panel->debug_window, COLOR_PAIR(4));
    wrefresh(panel->debug_window);

    draw_borders(panel->register_window);
    wattron(panel->register_window, COLOR_PAIR(4));
    mvwprintw(panel->register_window, 0, 1, "registers");
    wattroff(panel->register_window, COLOR_PAIR(4));
    wrefresh(panel->register_window);

    draw_borders(panel->info_window);
    wattron(panel->info_window, COLOR_PAIR(4));
    mvwprintw(panel->info_window, 0, 1, "information");
    wattroff(panel->info_window, COLOR_PAIR(4));
    wrefresh(panel->info_window);

    wmove(panel->instruction_window, iwy, iwx);
    wmove(panel->debug_window, idx, idy); 
    wmove(panel->info_window, irox, iroy);
    wmove(panel->register_window, irex, irey);

    return;
}

void update_info_window(WINDOW* window) {
    wprintw(window, "general information\n");
    wattron(window, COLOR_PAIR(2));
    wprintw(window, "8 bit processor architecture\n");
    wprintw(window, "3 general purpose registers: A, B, C\n");
    wprintw(window, "32 byte stack starting at address 0xE0\n");
    wprintw(window, "status register flags:\n");
    wprintw(window, "+------------------------------------------+\n");
    wprintw(window, "|0: compare|1: overflow|2: unused|3: unused|\n");
    wprintw(window, "|4: unused |5: unused  |6: unused|7: unused|\n");
    wprintw(window, "+------------------------------------------+\n");
    wattroff(window, COLOR_PAIR(2));

    wprintw(window, "operation information\n");
    wattron(window, COLOR_PAIR(6));
    wprintw(window, "program is stored in program.bin\n");
    wprintw(window, "press a key to step through the program\n");
    wprintw(window, "q to quit\n");
    wprintw(window, "emulator will exit on halt\n");
    wattroff(window, COLOR_PAIR(6));

    wprintw(window, "important instructions\n");
    wattron(window, COLOR_PAIR(1));
    wprintw(window, "halt code: 0xEA\n");
    wattroff(window, COLOR_PAIR(1));

    return;
}

int setup(PANEL* panel) {
	struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    panel->height = w.ws_row * 2; //x2 because font aspect ratio is 2
	panel->width = w.ws_col;

	panel->instruction_window = newwin(w.ws_row/2, w.ws_col/2, 0, 0);
    panel->debug_window = newwin(w.ws_row/2, w.ws_col/2, 0, w.ws_col/2);
    panel->info_window = newwin(w.ws_row/2, w.ws_col/2, w.ws_row/2, 0);
    panel->register_window = newwin(w.ws_row/2, w.ws_col/2, w.ws_row/2, w.ws_col/2);

    scrollok(panel->instruction_window,TRUE);
    scrollok(panel->debug_window,TRUE);
    scrollok(panel->info_window,TRUE);
    scrollok(panel->register_window,TRUE);

	if(!has_colors()) return 1;
	if(panel->instruction_window == NULL || panel->debug_window == NULL
       || panel->info_window == NULL || panel->register_window == NULL) return 2;

	curs_set(0); //hide cursor
	noecho();

	start_color();
        use_default_colors();
	init_pair(COLOUR_RED, COLOR_RED, -1);
	init_pair(COLOUR_GREEN, COLOR_GREEN, -1);
	init_pair(COLOUR_BLUE, COLOR_BLUE, -1);
	init_pair(COLOUR_CYAN, COLOR_CYAN, -1);
	init_pair(COLOUR_MAGENTA, COLOR_MAGENTA, -1);
	init_pair(COLOUR_YELLOW, COLOR_YELLOW, -1);
	init_pair(COLOUR_WHITE, COLOR_WHITE, -1);
	init_pair(COLOUR_BLACK, COLOR_BLACK, -1);

    refresh();

    print_window_titles(panel);

    wmove(panel->instruction_window, 1, 0);
    wmove(panel->debug_window, 1, 0);
    wmove(panel->info_window, 1, 0);
    wmove(panel->register_window, 1, 0);

    update_info_window(panel->info_window);

	return 0;
}
