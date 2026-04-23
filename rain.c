/*

                                 000      00
                           0000000   0000
              0      00  00000000000000000
            0000 0  000000000000000000000000       0
         000000000000000000000000000000000000000 000
        0000000000000000000000000000000000000000000000
    000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000
            C
                O M        |
                    F          |
                 Y                         |         |
            |                R  A
                                  I N
                       I N   |
              |                                    |
        |                            Y O
                |                   U        R
                   |            T E

                                     R   |   |
                         |            M
                               I N
                                        AL


    Although this was a purely fun-motivated project I
    challenged myself to write this code clean & leak-free.

    If you find bugs or leaks feel free to contact me or fork
    this. That would be awesome.

    @ Nik, 07.2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <curses.h>

#include "config.h"


//
//  GLOBALS
//

int userResized = 0;
int slowerDrops = 0;
short maxColorPair = 0;

typedef struct
{
    int  w;
    int  h;
    int  speed;
    int  color;
    char shape;
} Drop;

/**
 * Since we want the total number of drops to
 * be terminal-size dependent we need a dynamic/
 * resizeable data structure containing an array of
 * drops.
 */

typedef struct
{
    Drop *drops;
    int size;
    int capacity;

} d_Vector;


//
//  PROTOTYPES
//

Drop d_create();
void d_fall(Drop *d);
void d_show(Drop *d);

void v_init(d_Vector *v, int cap);
void v_free(d_Vector *v);
void v_delete(d_Vector *v);
void v_resize(d_Vector *v, int newCap);
void v_add(d_Vector *v, Drop d);
Drop *v_getAt(d_Vector *v, int pos);

void initCurses();
void exitCurses();

int pRand(int min, int max);
int getNumOfDrops();
void exitErr(const char *err) __attribute__((noreturn));
void usage();


//
//  FUNCTIONS - DROP
//

Drop d_create()
{
    Drop d;

    d.w = pRand(0, COLS);
    d.h = pRand(0, LINES);

    if (slowerDrops)
    {
        d.speed = pRand(cfg.speed_min, cfg.slow_speed_max + 1);
        (d.speed < 2) ? (d.shape = '|') : (d.shape = ':');
    }

    else
    {
        d.speed = pRand(cfg.speed_min, cfg.speed_max + 1);
        (d.speed < 3) ? (d.shape = '|') : (d.shape = ':');
    }

    int x = d.speed;

    // patented
    int color = (int) ((0.0416 * (x - 4)
                               * (x - 3)
                               * (x - 2) - 4)
                               * (x - 1) + 255);

    if (color < 1)
        color = 1;
    if (maxColorPair > 0 && color > maxColorPair)
        color = maxColorPair;

    d.color = color;
    return d;
}

void d_fall(Drop *d)
{
    d->h += d->speed;

    if (d->h >= LINES-1)
        d->h = pRand(0, 10);
}

void d_show(Drop *d)
{
    attron(COLOR_PAIR(d->color));
    mvaddch(d->h, d->w, d->shape);
    attroff(COLOR_PAIR(d->color));
}


//
//  FUNCTIONS - VECTOR
//

void v_init(d_Vector *v, int cap)
{
    if (cap > 0 && v != 0)
    {
        v->drops = (Drop *) malloc(sizeof(Drop) * cap);

        if (v->drops != 0)
        {
            v->size = 0;
            v->capacity = cap;
        }
        else
            exitErr("\n*DROP ARRAY IS >NULL<*\n");
    }
    else
        exitErr("\n*VECTOR INIT FAILED*\n");
}

void v_free(d_Vector *v)
{
    if(v->drops != 0)
    {
        free(v->drops);
        v->drops = 0;
    }

    v->size = 0;
    v->capacity = 0;
}

void v_delete(d_Vector *v)
{
    v_free(v);
}

void v_resize(d_Vector *v, int newCap)
{
    d_Vector newVec;
    v_init(&newVec, newCap);

    for (int i = 0; i < newCap; i++)
        v_add(&newVec, d_create());

    v_free(v);
    *v = newVec;
}

void v_add(d_Vector *v, Drop d)
{
    if (v->size >= v->capacity)
    {
        int newCap = (v->capacity > 0) ? v->capacity * 2 : 1;
        Drop *newDrops = realloc(v->drops, sizeof(Drop) * newCap);

        if (newDrops == 0)
            exitErr("\n*REALLOC FAILED*\n");

        v->drops    = newDrops;
        v->capacity = newCap;
    }

    v->drops[v->size] = d;
    v->size++;
}

Drop *v_getAt(d_Vector *v, int pos)
{
    if ((pos < v->size) && (pos >= 0))
        return &(v->drops[pos]);

    exitErr("\n*BAD ACCESS*\n");
}


//
//  FUNCTIONS - CURSES
//

void initCurses()
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, 1);

    if (curs_set(0) == ERR)
        exitErr("\n*Terminal emulator lacks capabilities.\n(Can't hide Cursor).*\n");

    timeout(0);

    if (has_colors())
    {
        use_default_colors();
        start_color();

        short limit = (COLORS < COLOR_PAIRS - 1)
                      ? (short) COLORS
                      : (short) (COLOR_PAIRS - 1);

        for (short i = 0; i < limit; i++)
            init_pair(i + 1, i, -1);

        maxColorPair = limit;
    }
    else
        exitErr("\n*Terminal emulator lacks capabilities.\n(Can't have colors).\n*");

}

void exitCurses()
{
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
}


//
//  UTILS
//

int pRand(int min, int max)
{
    max -= 1;
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void exitErr(const char *err)
{
    exitCurses();
    fprintf(stderr, "%s", err);
    exit(EXIT_FAILURE);
}

int getNumOfDrops()
{
    int nDrops = 0;

    if ((LINES < 20 && COLS > 100) || (COLS < 100 && LINES < 40))
    {
        nDrops = (int) (COLS * cfg.slow_density);

        // Watch that state..
        slowerDrops = 1;
    }
    else
    {
        nDrops = (int) (COLS * cfg.density);
        slowerDrops = 0;
    }

    if (nDrops < 1)
        nDrops = 1;

    return nDrops;
}

void usage()
{
    fprintf(stderr, "Usage: rain [--config <path>]\n");
    fprintf(stderr, "Hit 'q' to exit.\n");
}

// wrapper around nanosleep, replacing deprecated usleep func 
int mssleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec  = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


int main(int argc, char **argv)
{
    const char *config_path = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc)
        {
            config_path = argv[++i];
        }
        else
        {
            usage();
            exit(EXIT_FAILURE);
        }
    }

    config_load(config_path);

    srand((unsigned int) (time(NULL) ^ getpid()));
    initCurses();

    int dropsTotal = getNumOfDrops();
    d_Vector drops;
    v_init(&drops, dropsTotal);

    for (int i = 0; i < dropsTotal; i++)
        v_add(&drops, d_create());


    //
    //  DRAW-LOOP
    //

    while (1)
    {

        if (userResized)
        {
            static int lastLines = 0;
            static int lastCols  = 0;

            if (LINES != lastLines || COLS != lastCols)
            {
                dropsTotal = getNumOfDrops();
                v_resize(&drops, dropsTotal);
                lastLines = LINES;
                lastCols  = COLS;
            }

            userResized = 0;
        }

        for (int i = 0; i < dropsTotal; i++)
        {
            Drop *d = v_getAt(&drops, i);
            d_fall(d);
            d_show(d);
        }

        refresh();

        // Frame Delay
        mssleep(cfg.frame_delay_ms);

        int ch = wgetch(stdscr);
        if (ch == cfg.quit_key)
            break;
        if (ch == KEY_RESIZE)
            userResized = 1;

        erase();
    }

    // Free pointers & exit gracefully
    v_delete(&drops);
    exitCurses();

    return 0;
}
