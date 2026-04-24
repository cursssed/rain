#ifndef CONFIG_H
#define CONFIG_H

#define MAX_COLORS 16

typedef struct {
    unsigned char r, g, b;
} RgbColor;

typedef enum {
    COLOR_MODE_AUTO = 0,
    COLOR_MODE_MANUAL
} ColorMode;

typedef struct {
    int       frame_delay_ms;
    double    density;
    int       speed_min;
    int       speed_max;
    char      quit_key;
    ColorMode color_mode;
    RgbColor  color_base;
    RgbColor  colors[MAX_COLORS];
    int       colors_count;
    int       use_xterm256;
} Config;

extern Config cfg;

void config_load(const char *explicit_path);
int  config_init(const char *explicit_path, int force);
int  parse_hex_color(const char *s, RgbColor *out);

#endif
