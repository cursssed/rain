#ifndef CURSES_H
#define CURSES_H

int LINES  = 24;
int COLS   = 80;
int COLORS = 256;

typedef struct { int _dummy; } WINDOW;
static WINDOW _stdscr_inst;
WINDOW *stdscr = &_stdscr_inst;

#define COLOR_PAIR(n) ((int)(n))

static inline void initscr(void)                                        {}
static inline void noecho(void)                                         {}
static inline void cbreak(void)                                         {}
static inline int  keypad(WINDOW *w, int b)       { (void)w; (void)b; return 0; }
static inline int  curs_set(int v)                { (void)v; return 0; }
static inline void timeout(int d)                 { (void)d; }
static inline int  has_colors(void)               { return 1; }
static inline void use_default_colors(void)       {}
static inline void start_color(void)              {}
static inline void init_pair(short p, short f, short b) { (void)p; (void)f; (void)b; }
static inline void attron(int a)                  { (void)a; }
static inline void mvaddch(int y, int x, char c)  { (void)y; (void)x; (void)c; }
static inline void endwin(void)                   {}
static inline void refresh(void)                  {}
static inline void erase(void)                    {}
static inline void clear(void)                    {}
static inline void resetty(void)                  {}
static inline int  wgetch(WINDOW *w)              { (void)w; return -1; }

#endif
