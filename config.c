#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Config cfg = {
    .frame_delay_ms = 30,
    .density        = 1.5,
    .slow_density   = 0.75,
    .speed_min      = 1,
    .speed_max      = 5,
    .slow_speed_max = 2,
    .quit_key       = 'q',
};

static char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;

    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1)))
        end--;
    *end = '\0';

    return s;
}

static void apply_option(const char *key, const char *value)
{
    if (strcmp(key, "frame_delay_ms") == 0)
    {
        int v = atoi(value);
        if (v > 0)
            cfg.frame_delay_ms = v;
    }
    else if (strcmp(key, "density") == 0)
    {
        double v = atof(value);
        if (v > 0.0)
            cfg.density = v;
    }
    else if (strcmp(key, "slow_density") == 0)
    {
        double v = atof(value);
        if (v > 0.0)
            cfg.slow_density = v;
    }
    else if (strcmp(key, "speed_min") == 0)
    {
        int v = atoi(value);
        if (v > 0)
            cfg.speed_min = v;
    }
    else if (strcmp(key, "speed_max") == 0)
    {
        int v = atoi(value);
        if (v > 0)
            cfg.speed_max = v;
    }
    else if (strcmp(key, "slow_speed_max") == 0)
    {
        int v = atoi(value);
        if (v > 0)
            cfg.slow_speed_max = v;
    }
    else if (strcmp(key, "quit_key") == 0)
    {
        if (value[0])
            cfg.quit_key = value[0];
    }
    else
    {
        fprintf(stderr, "rain: unknown config option '%s'\n", key);
    }
}

static void parse_line(char *line)
{
    char *trimmed = trim(line);

    if (!*trimmed || *trimmed == '#')
        return;

    char *eq = strchr(trimmed, '=');
    if (!eq)
    {
        fprintf(stderr, "rain: invalid config line: '%s'\n", trimmed);
        return;
    }

    *eq = '\0';
    char *key   = trim(trimmed);
    char *value = trim(eq + 1);

    if (!*key)
        return;

    apply_option(key, value);
}

static void resolve_default_path(char *buf, size_t buf_size)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg && xdg[0])
    {
        snprintf(buf, buf_size, "%s/rain/config", xdg);
        return;
    }

    const char *home = getenv("HOME");
    if (home && home[0])
    {
        snprintf(buf, buf_size, "%s/.config/rain/config", home);
        return;
    }

    buf[0] = '\0';
}

void config_load(const char *explicit_path)
{
    char path[1024];
    const char *target;

    if (explicit_path && explicit_path[0])
    {
        target = explicit_path;
    }
    else
    {
        resolve_default_path(path, sizeof(path));
        if (!path[0])
            return;
        target = path;
    }

    FILE *f = fopen(target, "r");
    if (!f)
        return;

    char line[512];
    while (fgets(line, sizeof(line), f))
        parse_line(line);

    fclose(f);
}
