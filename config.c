#include "config.h"
#include "config_template.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

Config cfg = {
    .frame_delay_ms = 30,
    .density        = 1.5,
    .speed_min      = 1,
    .speed_max      = 5,
    .quit_key       = 'q',
    .color_mode     = COLOR_MODE_AUTO,
    .color_base     = { 220, 220, 230 },
    .colors_count   = 0,
    .use_xterm256   = 0,
};

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int parse_hex_color(const char *s, RgbColor *out)
{
    while (*s && isspace((unsigned char)*s))
        s++;

    if (*s == '#')
        s++;

    int digits[6];
    for (int i = 0; i < 6; i++)
    {
        digits[i] = hex_digit(s[i]);
        if (digits[i] < 0)
            return 0;
    }

    const char *tail = s + 6;
    while (*tail && isspace((unsigned char)*tail))
        tail++;
    if (*tail)
        return 0;

    out->r = (unsigned char)((digits[0] << 4) | digits[1]);
    out->g = (unsigned char)((digits[2] << 4) | digits[3]);
    out->b = (unsigned char)((digits[4] << 4) | digits[5]);
    return 1;
}

static int parse_colors_list(const char *value, RgbColor *out, int max_count)
{
    char buf[512];
    strncpy(buf, value, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int count = 0;
    char *saveptr = NULL;
    for (char *tok = strtok_r(buf, ",", &saveptr);
         tok != NULL && count < max_count;
         tok = strtok_r(NULL, ",", &saveptr))
    {
        RgbColor c;
        if (!parse_hex_color(tok, &c))
            return -1;
        out[count++] = c;
    }

    return count;
}

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
    else if (strcmp(key, "quit_key") == 0)
    {
        if (value[0])
            cfg.quit_key = value[0];
    }
    else if (strcmp(key, "color_mode") == 0)
    {
        if (strcmp(value, "auto") == 0)
            cfg.color_mode = COLOR_MODE_AUTO;
        else if (strcmp(value, "manual") == 0)
            cfg.color_mode = COLOR_MODE_MANUAL;
        else
            fprintf(stderr, "rain: invalid color_mode '%s' (use auto or manual)\n", value);
    }
    else if (strcmp(key, "color_base") == 0)
    {
        RgbColor c;
        if (parse_hex_color(value, &c))
            cfg.color_base = c;
        else
            fprintf(stderr, "rain: invalid hex color for color_base: '%s'\n", value);
    }
    else if (strcmp(key, "colors") == 0)
    {
        int n = parse_colors_list(value, cfg.colors, MAX_COLORS);
        if (n > 0)
            cfg.colors_count = n;
        else
            fprintf(stderr, "rain: invalid colors list: '%s'\n", value);
    }
    else if (strcmp(key, "use_xterm256") == 0)
    {
        if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
            cfg.use_xterm256 = 1;
        else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0)
            cfg.use_xterm256 = 0;
        else
            fprintf(stderr, "rain: invalid use_xterm256 '%s' (use true or false)\n", value);
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

static int mkdir_p(const char *path)
{
    char buf[1024];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(buf))
        return -1;
    memcpy(buf, path, len + 1);

    for (char *p = buf + 1; *p; p++)
    {
        if (*p != '/')
            continue;
        *p = '\0';
        if (mkdir(buf, 0755) != 0 && errno != EEXIST)
            return -1;
        *p = '/';
    }

    if (mkdir(buf, 0755) != 0 && errno != EEXIST)
        return -1;

    return 0;
}

int config_init(const char *explicit_path, int force)
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
        {
            fprintf(stderr, "rain: cannot resolve config path (HOME/XDG_CONFIG_HOME unset)\n");
            return -1;
        }
        target = path;
    }

    if (!force)
    {
        FILE *existing = fopen(target, "r");
        if (existing)
        {
            fclose(existing);
            fprintf(stderr, "rain: config already exists at %s (use --force to overwrite)\n", target);
            return -1;
        }
    }

    char parent[1024];
    strncpy(parent, target, sizeof(parent) - 1);
    parent[sizeof(parent) - 1] = '\0';
    char *slash = strrchr(parent, '/');
    if (slash && slash != parent)
    {
        *slash = '\0';
        if (mkdir_p(parent) != 0)
        {
            fprintf(stderr, "rain: cannot create %s: %s\n", parent, strerror(errno));
            return -1;
        }
    }

    FILE *f = fopen(target, "w");
    if (!f)
    {
        fprintf(stderr, "rain: cannot write %s: %s\n", target, strerror(errno));
        return -1;
    }

    if (fwrite(CONFIG_TEMPLATE, 1, CONFIG_TEMPLATE_LEN, f) != CONFIG_TEMPLATE_LEN)
    {
        fprintf(stderr, "rain: write error on %s: %s\n", target, strerror(errno));
        fclose(f);
        return -1;
    }

    fclose(f);
    printf("rain: config written to %s\n", target);
    return 0;
}
