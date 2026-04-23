#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int    frame_delay_ms;
    double density;
    double slow_density;
    int    speed_min;
    int    speed_max;
    int    slow_speed_max;
    char   quit_key;
} Config;

extern Config cfg;

void config_load(const char *explicit_path);

#endif
