#include <stdio.h>
#include <stdlib.h>

#define main rain_main
#include "../rain.c"
#include "../config.c"
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

static void vector_grow_on_add_tests(void)
{
    COLS = 80; LINES = 24;
    d_Vector v;
    v_init(&v, 2);

    for (int i = 0; i < 20; i++) {
        Drop d = {0};
        d.w     = i;
        d.h     = i * 2;
        d.speed = 1;
        d.color = 1;
        d.shape = '|';
        v_add(&v, d);
    }

    CHECK(v.size     == 20, "v_add (grow): size correct after crossing capacity");
    CHECK(v.capacity >= 20, "v_add (grow): capacity grew to fit");

    int ok = 1;
    for (int i = 0; i < 20; i++) {
        Drop *d = v_getAt(&v, i);
        if (d->w != i || d->h != i * 2) { ok = 0; break; }
    }
    CHECK(ok, "v_add (grow): preserves previously inserted data");

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
    CHECK(d->h <= 0 && d->h > -LINES, "d_fall: respawns above visible area");
    v_delete(&v);
}

static void d_create_fast_tests(void)
{
    COLS = 80; LINES = 24;
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

static void nearest_xterm256_tests(void)
{
    RgbColor black = { 0, 0, 0 };
    CHECK(nearest_xterm256(black) == 16, "nearest_xterm256: pure black -> 16");

    RgbColor white = { 255, 255, 255 };
    CHECK(nearest_xterm256(white) == 231, "nearest_xterm256: pure white -> 231");

    RgbColor red = { 255, 0, 0 };
    CHECK(nearest_xterm256(red) == 196, "nearest_xterm256: pure red -> 196");

    RgbColor gray = { 168, 168, 168 };
    short idx = nearest_xterm256(gray);
    CHECK(idx >= 232 && idx <= 255, "nearest_xterm256: near-neutral maps to grayscale ramp");

    RgbColor off_white = { 220, 220, 230 };
    idx = nearest_xterm256(off_white);
    CHECK(idx >= 232 && idx <= 255, "nearest_xterm256: default color_base maps to grayscale");
}

static void d_create_color_range_tests(void)
{
    COLS = 80; LINES = 24;
    maxColorPair = 0;

    srand(20);
    int ok = 1;
    for (int i = 0; i < 500; i++) {
        Drop d = d_create();
        if (d.color < 1) { ok = 0; break; }
    }
    CHECK(ok, "d_create: color >= 1");
}

static void d_create_color_clamp_tests(void)
{
    COLS = 80; LINES = 24;
    maxColorPair = 16;
    srand(22);
    int ok = 1;
    for (int i = 0; i < 500; i++) {
        Drop d = d_create();
        if (d.color < 1 || d.color > 16) { ok = 0; break; }
    }
    CHECK(ok, "d_create: color clamped to [1, maxColorPair]");
    maxColorPair = 0;
}

static void config_parse_tests(void)
{
    const char *path = "/tmp/rain_test_config";

    FILE *f = fopen(path, "w");
    fprintf(f, "# comment line\n");
    fprintf(f, "\n");
    fprintf(f, "frame_delay_ms = 50\n");
    fprintf(f, "  density =  2.5  \n");
    fprintf(f, "speed_min = 2\n");
    fprintf(f, "speed_max = 8\n");
    fprintf(f, "quit_key=X\n");
    fprintf(f, "bogus_key = ignored\n");
    fprintf(f, "malformed line\n");
    fclose(f);

    Config saved = cfg;
    config_load(path);

    CHECK(cfg.frame_delay_ms == 50, "config: numeric option parsed");
    CHECK(cfg.density == 2.5,       "config: float with surrounding whitespace parsed");
    CHECK(cfg.speed_min == 2,       "config: speed_min parsed");
    CHECK(cfg.speed_max == 8,       "config: speed_max parsed");
    CHECK(cfg.quit_key == 'X',      "config: char option parsed without spaces");

    unlink(path);
    cfg = saved;
}

static void config_sample_file_tests(void)
{
    const char *path = "/tmp/rain_test_sample";

    FILE *f = fopen(path, "w");
    fprintf(f, "frame_delay_ms = 40\n");
    fprintf(f, "density = 1.5\n");
    fprintf(f, "speed_min = 1\n");
    fprintf(f, "speed_max = 5\n");
    fprintf(f, "quit_key = x\n");
    fprintf(f, "color_mode = manual\n");
    fprintf(f, "colors = #CA81D2, #C472CC, #9E5DA6, #7E4982, #603864\n");
    fprintf(f, "use_xterm256 = false\n");
    fclose(f);

    Config saved = cfg;
    cfg.frame_delay_ms = 1;
    cfg.quit_key       = '!';
    cfg.color_mode     = COLOR_MODE_AUTO;
    cfg.colors_count   = 0;

    config_load(path);

    CHECK(cfg.frame_delay_ms == 40,            "config sample: frame_delay_ms loaded");
    CHECK(cfg.quit_key == 'x',                 "config sample: quit_key loaded");
    CHECK(cfg.color_mode == COLOR_MODE_MANUAL, "config sample: color_mode loaded");
    CHECK(cfg.colors_count == 5,               "config sample: colors list count");

    unlink(path);
    cfg = saved;
}

static void config_hex_parser_tests(void)
{
    RgbColor c;

    CHECK(parse_hex_color("#ff0080", &c) == 1
          && c.r == 0xff && c.g == 0x00 && c.b == 0x80,
          "parse_hex_color: #rrggbb");

    CHECK(parse_hex_color("aabbcc", &c) == 1
          && c.r == 0xaa && c.g == 0xbb && c.b == 0xcc,
          "parse_hex_color: rrggbb without hash");

    CHECK(parse_hex_color("  #A1B2C3  ", &c) == 1
          && c.r == 0xa1 && c.g == 0xb2 && c.b == 0xc3,
          "parse_hex_color: surrounding whitespace and uppercase");

    CHECK(parse_hex_color("#zzzzzz", &c) == 0, "parse_hex_color: non-hex rejected");
    CHECK(parse_hex_color("#abc",    &c) == 0, "parse_hex_color: short form rejected");
    CHECK(parse_hex_color("",        &c) == 0, "parse_hex_color: empty rejected");
    CHECK(parse_hex_color("#abcdef00", &c) == 0, "parse_hex_color: trailing data rejected");
}

static void config_color_fields_tests(void)
{
    const char *path = "/tmp/rain_test_config_colors";

    FILE *f = fopen(path, "w");
    fprintf(f, "color_mode = manual\n");
    fprintf(f, "color_base = #112233\n");
    fprintf(f, "colors = #ff0000, #00ff00, #0000ff\n");
    fclose(f);

    Config saved = cfg;
    config_load(path);

    CHECK(cfg.color_mode == COLOR_MODE_MANUAL, "config: color_mode manual parsed");
    CHECK(cfg.color_base.r == 0x11 && cfg.color_base.g == 0x22 && cfg.color_base.b == 0x33,
          "config: color_base parsed");
    CHECK(cfg.colors_count == 3, "config: colors list count");
    CHECK(cfg.colors[0].r == 0xff && cfg.colors[1].g == 0xff && cfg.colors[2].b == 0xff,
          "config: colors list entries");

    unlink(path);
    cfg = saved;
}

static void config_use_xterm256_tests(void)
{
    const char *path = "/tmp/rain_test_config_xterm256";

    FILE *f = fopen(path, "w");
    fprintf(f, "use_xterm256 = true\n");
    fclose(f);

    Config saved = cfg;
    cfg.use_xterm256 = 0;
    config_load(path);
    CHECK(cfg.use_xterm256 == 1, "config: use_xterm256 = true parsed");

    f = fopen(path, "w");
    fprintf(f, "use_xterm256 = false\n");
    fclose(f);

    cfg.use_xterm256 = 1;
    config_load(path);
    CHECK(cfg.use_xterm256 == 0, "config: use_xterm256 = false parsed");

    unlink(path);
    cfg = saved;
}

static void config_color_mode_auto_tests(void)
{
    const char *path = "/tmp/rain_test_config_auto";

    FILE *f = fopen(path, "w");
    fprintf(f, "color_mode = auto\n");
    fclose(f);

    Config saved = cfg;
    cfg.color_mode = COLOR_MODE_MANUAL;
    config_load(path);

    CHECK(cfg.color_mode == COLOR_MODE_AUTO, "config: color_mode auto parsed");

    unlink(path);
    cfg = saved;
}

static void config_rejects_invalid_values_tests(void)
{
    const char *path = "/tmp/rain_test_config_invalid";

    FILE *f = fopen(path, "w");
    fprintf(f, "frame_delay_ms = -5\n");
    fprintf(f, "density = 0\n");
    fprintf(f, "speed_max = 0\n");
    fclose(f);

    Config saved = cfg;
    cfg.frame_delay_ms = 30;
    cfg.density        = 1.5;
    cfg.speed_max      = 5;

    config_load(path);

    CHECK(cfg.frame_delay_ms == 30, "config: non-positive frame_delay_ms rejected");
    CHECK(cfg.density == 1.5,       "config: non-positive density rejected");
    CHECK(cfg.speed_max == 5,       "config: non-positive speed_max rejected");

    unlink(path);
    cfg = saved;
}

static void config_missing_file_tests(void)
{
    Config saved = cfg;
    cfg.frame_delay_ms = 42;

    config_load("/tmp/rain_definitely_nonexistent_xyz123");

    CHECK(cfg.frame_delay_ms == 42, "config: missing file leaves values untouched");

    cfg = saved;
}

static void getNumOfDrops_tests(void)
{
    LINES = 40; COLS = 120;
    int n = getNumOfDrops();
    CHECK(n == (int)(120 * 1.5), "getNumOfDrops: uses density * cols");

    LINES = 15; COLS = 110;
    n = getNumOfDrops();
    CHECK(n == (int)(110 * 1.5), "getNumOfDrops: small terminal still uses density * cols");

    LINES = 24; COLS = 0;
    n = getNumOfDrops();
    CHECK(n >= 1, "getNumOfDrops: clamps to at least 1 when COLS is 0");

    LINES = 0; COLS = 0;
    n = getNumOfDrops();
    CHECK(n >= 1, "getNumOfDrops: clamps to at least 1 when LINES and COLS are 0");
}

int main(void)
{
    pRand_tests();
    mssleep_tests();
    vector_init_tests();
    vector_free_tests();
    vector_add_tests();
    vector_get_tests();
    vector_grow_on_add_tests();
    d_fall_advance_tests();
    d_fall_wrap_tests();
    d_create_fast_tests();
    d_create_position_tests();
    nearest_xterm256_tests();
    d_create_color_range_tests();
    d_create_color_clamp_tests();
    getNumOfDrops_tests();
    config_parse_tests();
    config_sample_file_tests();
    config_missing_file_tests();
    config_rejects_invalid_values_tests();
    config_hex_parser_tests();
    config_color_fields_tests();
    config_use_xterm256_tests();
    config_color_mode_auto_tests();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
