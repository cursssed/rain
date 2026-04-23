#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int    frame_delay_ms;
    double density;
    char   quit_key;
} Config;

extern Config cfg;

void config_load(const char *explicit_path);

#endif
