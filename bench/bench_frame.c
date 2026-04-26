#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long bench_mvaddch_count = 0;

#define main rain_main
#include "../rain.c"
#include "../config.c"
#undef main

#define BENCH_FRAMES 100000

int main(void)
{
    COLS = 80; LINES = 24;
    srand(42);
    config_load(NULL);

    short max_pair = 5;
    int n = getNumOfDrops();
    d_Vector drops;
    v_init(&drops, n);
    for (int i = 0; i < n; i++)
        v_add(&drops, d_create(max_pair));

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int f = 0; f < BENCH_FRAMES; f++)
    {
        for (int i = 0; i < drops.size; i++)
        {
            Drop *d = &drops.drops[i];
            d_fall(d);
            d_show(d);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double ms = (t1.tv_sec  - t0.tv_sec)  * 1000.0
              + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("frames:        %d\n",   BENCH_FRAMES);
    printf("drops:         %d\n",   n);
    printf("time:          %.2f ms\n", ms);
    printf("mvaddch calls: %ld\n",  bench_mvaddch_count);
    printf("calls/frame:   %.1f\n", (double)bench_mvaddch_count / BENCH_FRAMES);

    v_free(&drops);
    return 0;
}
