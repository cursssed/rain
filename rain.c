#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>

#include "config.h"

#ifndef RAIN_VERSION
#define RAIN_VERSION "unknown"
#endif


typedef struct
{
    int  w;
    int  h;
    int  speed;
    int  color;
    char shape;
} Drop;

typedef struct
{
    Drop *drops;
    int size;
    int capacity;

} d_Vector;


int pRand(int min, int max)
{
    max -= 1;
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void exitCurses()
{
    static int done = 0;
    if (done) return;
    done = 1;
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
}

static void on_signal(int sig) { (void)sig; exit(0); }

void exitErr(const char *err) __attribute__((noreturn));
void exitErr(const char *err)
{
    exitCurses();
    fprintf(stderr, "%s", err);
    exit(EXIT_FAILURE);
}

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

int getNumOfDrops()
{
    int nDrops = (int) (COLS * cfg.density);

    if (nDrops < 1)
        nDrops = 1;

    return nDrops;
}

static void usage(FILE *out)
{
    fprintf(out, "Usage: rain [--config <path>] [--init-config [--force]] [--help] [--version]\n");
    fprintf(out, "  --config <path>   load configuration from <path>\n");
    fprintf(out, "  --init-config     write a starter config to the standard\n");
    fprintf(out, "                    location (or <path> if --config given)\n");
    fprintf(out, "  --force           overwrite an existing config file\n");
    fprintf(out, "  --help, -h        show this help and exit\n");
    fprintf(out, "  --version, -V     show version and exit\n");
    fprintf(out, "\n");
    fprintf(out, "Config search order:\n");
    fprintf(out, "  --config <path>\n");
    fprintf(out, "  $XDG_CONFIG_HOME/rain/config\n");
    fprintf(out, "  $HOME/.config/rain/config\n");
    fprintf(out, "\n");
    fprintf(out, "Press 'q' to quit (configurable via quit_key in config).\n");
}

Drop d_create(short max_pair)
{
    Drop d;

    d.w = pRand(0, COLS);
    d.h = pRand(0, LINES);

    d.speed = pRand(cfg.speed_min, cfg.speed_max + 1);
    d.shape = (d.speed < 3) ? '|' : ':';

    int color = d.speed;

    if (color < 1)
        color = 1;
    if (max_pair > 0 && color > max_pair)
        color = max_pair;

    d.color = color;
    return d;
}

void d_fall(Drop *d)
{
    d->h += d->speed;

    if (d->h >= LINES)
        d->h = -pRand(0, LINES);
}

static void d_show(Drop *d)
{
    if (d->h < 0)
        return;
    mvaddch(d->h, d->w, (chtype)d->shape | COLOR_PAIR(d->color));
}


void v_init(d_Vector *v, int cap)
{
    v->size     = 0;
    v->capacity = 0;
    v->drops    = NULL;

    if (cap > 0)
    {
        v->drops = malloc(sizeof(*v->drops) * (size_t)cap);
        if (!v->drops)
            exitErr("\n*alloc failed*\n");
        v->capacity = cap;
    }
}

void v_free(d_Vector *v)
{
    if (v->drops != NULL)
    {
        free(v->drops);
        v->drops = NULL;
    }

    v->size = 0;
    v->capacity = 0;
}

void v_add(d_Vector *v, Drop d)
{
    if (v->size >= v->capacity)
    {
        int newCap = (v->capacity > 0) ? v->capacity * 2 : 1;
        Drop *newDrops = realloc(v->drops, sizeof(*v->drops) * (size_t)newCap);

        if (newDrops == NULL)
            exitErr("\n*REALLOC FAILED*\n");

        v->drops    = newDrops;
        v->capacity = newCap;
    }

    v->drops[v->size] = d;
    v->size++;
}


static short nearest_xterm256(RgbColor c)
{
    static const int cube_levels[6] = { 0, 95, 135, 175, 215, 255 };

    int best_r = 0, best_g = 0, best_b = 0;
    int min_dr = 1 << 30, min_dg = 1 << 30, min_db = 1 << 30;

    for (int i = 0; i < 6; i++)
    {
        int dr = (c.r - cube_levels[i]) * (c.r - cube_levels[i]);
        int dg = (c.g - cube_levels[i]) * (c.g - cube_levels[i]);
        int db = (c.b - cube_levels[i]) * (c.b - cube_levels[i]);

        if (dr < min_dr) { min_dr = dr; best_r = i; }
        if (dg < min_dg) { min_dg = dg; best_g = i; }
        if (db < min_db) { min_db = db; best_b = i; }
    }

    short cube_idx  = (short)(16 + best_r * 36 + best_g * 6 + best_b);
    int   cube_dist = min_dr + min_dg + min_db;

    int gray_avg  = (c.r + c.g + c.b) / 3;
    int gray_step = (gray_avg - 8 + 5) / 10;
    if (gray_step < 0)  gray_step = 0;
    if (gray_step > 23) gray_step = 23;

    int gray_value = 8 + gray_step * 10;
    int gray_dist  = (c.r - gray_value) * (c.r - gray_value)
                   + (c.g - gray_value) * (c.g - gray_value)
                   + (c.b - gray_value) * (c.b - gray_value);
    short gray_idx = (short)(232 + gray_step);

    return (gray_dist < cube_dist) ? gray_idx : cube_idx;
}

static void build_palette(RgbColor *out, int count)
{
    if (cfg.color_mode == COLOR_MODE_MANUAL && cfg.colors_count >= count)
    {
        for (int i = 0; i < count; i++)
            out[i] = cfg.colors[i];
        return;
    }

    for (int i = 0; i < count; i++)
    {
        double m = (count <= 1) ? 1.0 : 1.0 - 0.25 * i / (double)(count - 1);
        out[i].r = (unsigned char)(cfg.color_base.r * m);
        out[i].g = (unsigned char)(cfg.color_base.g * m);
        out[i].b = (unsigned char)(cfg.color_base.b * m);
    }
}

static short apply_palette(void)
{
    int count = cfg.speed_max;
    if (count < 1) count = 1;
    if (count > MAX_COLORS) count = MAX_COLORS;

    short limit = (short)((count < COLOR_PAIRS - 1) ? count : COLOR_PAIRS - 1);

    RgbColor palette[MAX_COLORS];
    build_palette(palette, limit);

    for (short i = 0; i < limit; i++)
    {
        if (cfg.use_xterm256)
        {
            init_pair((short)(i + 1), nearest_xterm256(palette[i]), -1);
        }
        else
        {
            short idx = (short)(16 + i);
            init_color(idx,
                       (short)(palette[i].r * 1000 / 255),
                       (short)(palette[i].g * 1000 / 255),
                       (short)(palette[i].b * 1000 / 255));
            init_pair((short)(i + 1), idx, -1);
        }
    }

    return limit;
}

short initCurses()
{
    initscr();
    atexit(exitCurses);
    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);
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
        return apply_palette();
    }

    exitErr("\n*Terminal emulator lacks capabilities.\n(Can't have colors).\n*");
}


static void handle_resize(d_Vector *drops, int *dropsTotal, int *lastLines, int *lastCols, short max_pair)
{
    if (LINES == *lastLines && COLS == *lastCols)
        return;

    if (COLS < *lastCols || LINES < *lastLines)
    {
        int w = 0;
        for (int r = 0; r < drops->size; r++)
        {
            Drop *d = &drops->drops[r];
            if (d->w < COLS && d->h < LINES)
                drops->drops[w++] = *d;
        }
        drops->size = w;
    }

    int newTotal = getNumOfDrops();

    if (newTotal > drops->size)
    {
        int wStart = (COLS  > *lastCols)  ? *lastCols  : 0;
        int hStart = (LINES > *lastLines) ? *lastLines : 0;

        for (int i = drops->size; i < newTotal; i++)
        {
            Drop d = d_create(max_pair);
            d.w = pRand(wStart, COLS);
            d.h = pRand(hStart, LINES);
            v_add(drops, d);
        }
    }
    else if (newTotal < drops->size)
    {
        drops->size = newTotal;
    }

    *dropsTotal = newTotal;
    *lastLines  = LINES;
    *lastCols   = COLS;

    clearok(stdscr, 1);
}

int main(int argc, char **argv)
{
    const char *config_path = NULL;
    int init_config = 0;
    int force       = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc)
        {
            config_path = argv[++i];
        }
        else if (strcmp(argv[i], "--init-config") == 0)
        {
            init_config = 1;
        }
        else if (strcmp(argv[i], "--force") == 0)
        {
            force = 1;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            usage(stdout);
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0)
        {
            printf("rain %s\n", RAIN_VERSION);
            exit(EXIT_SUCCESS);
        }
        else
        {
            usage(stderr);
            exit(EXIT_FAILURE);
        }
    }

    if (force && !init_config)
    {
        fprintf(stderr, "rain: --force only makes sense with --init-config\n");
        exit(EXIT_FAILURE);
    }

    if (init_config)
        exit(config_init(config_path, force) == 0 ? EXIT_SUCCESS : EXIT_FAILURE);

    config_load(config_path);

    srand((unsigned int) (time(NULL) ^ getpid()));
    short max_pair = initCurses();

    int dropsTotal = getNumOfDrops();
    d_Vector drops;
    v_init(&drops, dropsTotal);

    for (int i = 0; i < dropsTotal; i++)
        v_add(&drops, d_create(max_pair));

    int lastLines = 0;
    int lastCols  = 0;

    while (1)
    {
        for (int i = 0; i < drops.size; i++)
        {
            Drop *d = &drops.drops[i];
            d_fall(d);
            d_show(d);
        }

        refresh();

        mssleep(cfg.frame_delay_ms);

        int ch = wgetch(stdscr);
        if (ch == cfg.quit_key)
            break;
        if (ch == KEY_RESIZE)
            handle_resize(&drops, &dropsTotal, &lastLines, &lastCols, max_pair);

        erase();
    }

    v_free(&drops);

    return 0;
}
