#include <stdio.h>
#include <stdlib.h>

#define main rain_main
#include "../rain.c"
#undef main

static int passed = 0;
static int failed = 0;

#define CHECK(cond, label) \
    do { \
        if (cond) { printf("pass  %s\n", label); passed++; } \
        else      { printf("FAIL  %s\n", label); failed++; } \
    } while (0)

static void pRand_tests(void)
{
    srand(42);
    int ok = 1;
    for (int i = 0; i < 2000; i++) {
        int v = pRand(0, 10);
        if (v < 0 || v > 9) { ok = 0; break; }
    }
    CHECK(ok, "pRand [0,10): all samples within bounds");

    ok = 1;
    for (int i = 0; i < 2000; i++) {
        int v = pRand(5, 15);
        if (v < 5 || v > 14) { ok = 0; break; }
    }
    CHECK(ok, "pRand [5,15): all samples within bounds");
}

static void mssleep_tests(void)
{
    CHECK(mssleep(-1) == -1, "mssleep: negative input returns -1");
    CHECK(mssleep(0)  ==  0, "mssleep: zero input returns 0");
    CHECK(mssleep(1)  ==  0, "mssleep: 1ms returns 0");
}

static void vector_init_tests(void)
{
    d_Vector v;
    v_init(&v, 8);
    CHECK(v.size     == 0,    "v_init: size starts at 0");
    CHECK(v.capacity == 8,    "v_init: capacity matches argument");
    CHECK(v.drops    != NULL, "v_init: drops array allocated");
    v_free(&v);
}

static void vector_free_tests(void)
{
    d_Vector v;
    v_init(&v, 4);
    v_free(&v);
    CHECK(v.size     == 0,    "v_free: size reset to 0");
    CHECK(v.capacity == 0,    "v_free: capacity reset to 0");
    CHECK(v.drops    == NULL, "v_free: drops pointer nulled");
}

static void vector_add_tests(void)
{
    COLS = 80; LINES = 24;
    srand(1);
    d_Vector v;
    v_init(&v, 16);
    for (int i = 0; i < 10; i++)
        v_add(&v, d_create());
    CHECK(v.size == 10, "v_add: size increments on each add");
    v_delete(&v);
}

static void vector_get_tests(void)
{
    COLS = 80; LINES = 24;
    srand(2);
    d_Vector v;
    v_init(&v, 4);
    Drop d = d_create();
    d.speed = 3;
    v_add(&v, d);
    Drop *got = v_getAt(&v, 0);
    CHECK(got != NULL,      "v_getAt: returns non-null for valid index");
    CHECK(got->speed == 3,  "v_getAt: returns the correct element");
    v_delete(&v);
}

static void vector_resize_tests(void)
{
    COLS = 80; LINES = 24;
    srand(3);
    d_Vector v;
    v_init(&v, 8);
    for (int i = 0; i < 8; i++)
        v_add(&v, d_create());
    v_resize(&v, 16);
    CHECK(v.size     == 16, "v_resize: fills to new capacity");
    CHECK(v.capacity == 16, "v_resize: capacity updated");
    v_delete(&v);
}

static void d_fall_advance_tests(void)
{
    COLS = 80; LINES = 40;
    srand(4);
    d_Vector v;
    v_init(&v, 4);
    v_add(&v, d_create());
    Drop *d = v_getAt(&v, 0);
    d->h = 5;
    int speed = d->speed;
    d_fall(d);
    CHECK(d->h == 5 + speed, "d_fall: h advances by speed");
    v_delete(&v);
}

static void d_fall_wrap_tests(void)
{
    COLS = 80; LINES = 24;
    srand(5);
    d_Vector v;
    v_init(&v, 4);
    v_add(&v, d_create());
    Drop *d = v_getAt(&v, 0);
    d->h     = LINES - 1;
    d->speed = 1;
    d_fall(d);
    CHECK(d->h < 10, "d_fall: wraps to top when at screen bottom");
    v_delete(&v);
}

static void d_create_fast_tests(void)
{
    COLS = 80; LINES = 24;
    slowerDrops = 0;
    srand(10);
    int ok_speed = 1, ok_shape = 1;
    for (int i = 0; i < 500; i++) {
        Drop d = d_create();
        if (d.speed < 1 || d.speed > 5)         ok_speed = 0;
        if (d.speed < 3  && d.shape != '|')      ok_shape = 0;
        if (d.speed >= 3 && d.shape != ':')      ok_shape = 0;
    }
    CHECK(ok_speed, "d_create (fast): speed in [1,5]");
    CHECK(ok_shape, "d_create (fast): shape matches speed threshold");
}

static void d_create_slow_tests(void)
{
    COLS = 80; LINES = 24;
    slowerDrops = 1;
    srand(11);
    int ok_speed = 1, ok_shape = 1;
    for (int i = 0; i < 500; i++) {
        Drop d = d_create();
        if (d.speed < 1 || d.speed > 2)         ok_speed = 0;
        if (d.speed < 2  && d.shape != '|')      ok_shape = 0;
        if (d.speed >= 2 && d.shape != ':')      ok_shape = 0;
    }
    CHECK(ok_speed, "d_create (slow): speed in [1,2]");
    CHECK(ok_shape, "d_create (slow): shape matches speed threshold");
    slowerDrops = 0;
}

static void d_create_position_tests(void)
{
    COLS = 80; LINES = 24;
    srand(12);
    int ok = 1;
    for (int i = 0; i < 500; i++) {
        Drop d = d_create();
        if (d.w < 0 || d.w >= COLS)  { ok = 0; break; }
        if (d.h < 0 || d.h >= LINES) { ok = 0; break; }
    }
    CHECK(ok, "d_create: position within terminal bounds");
}

static void handleResize_tests(void)
{
    userResized = 0;
    handleResize(28);
    CHECK(userResized == 1, "handleResize: sets userResized flag");
    userResized = 0;
}

static void getNumOfDrops_tests(void)
{
    LINES = 40; COLS = 120; slowerDrops = 0;
    int n = getNumOfDrops();
    CHECK(n == (int)(120 * 1.5), "getNumOfDrops: large terminal uses 1.5x cols");
    CHECK(slowerDrops == 0,      "getNumOfDrops: slowerDrops off for large terminal");

    LINES = 15; COLS = 110;
    n = getNumOfDrops();
    CHECK(n == (int)(110 * 0.75), "getNumOfDrops: few lines triggers 0.75x cols");
    CHECK(slowerDrops == 1,       "getNumOfDrops: slowerDrops on for few lines");

    LINES = 30; COLS = 60;
    n = getNumOfDrops();
    CHECK(n == (int)(60 * 0.75), "getNumOfDrops: few cols triggers 0.75x cols");
    CHECK(slowerDrops == 1,      "getNumOfDrops: slowerDrops on for few cols");
}

int main(void)
{
    pRand_tests();
    mssleep_tests();
    vector_init_tests();
    vector_free_tests();
    vector_add_tests();
    vector_get_tests();
    vector_resize_tests();
    d_fall_advance_tests();
    d_fall_wrap_tests();
    d_create_fast_tests();
    d_create_slow_tests();
    d_create_position_tests();
    getNumOfDrops_tests();
    handleResize_tests();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
