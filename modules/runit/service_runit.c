// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

/*
 * Compatibility parser for runit service directories.
 *
 * A runit service directory under the raesir services path must contain:
 *   run    - executable script that launches the daemon (required)
 *
 * Optional files:
 *   down   - presence marks service as not auto-restarted
 *   env/   - directory whose files each define one environment variable;
 *            filename = variable name, first line of file = value
 *
 * The run script is scanned for the last bare `exec` line.  If found, its
 * arguments become argv.  If not found, the run script itself is invoked
 * directly.
 */

#include "service.h"
#include "log.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LINE_MAX_LEN 1024

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) *--end = '\0';
    return s;
}

static void append_env(service_def_t *def, const char *pair) {
    int n = 0;
    if (def->env)
        while (def->env[n]) n++;
    char **tmp = realloc(def->env, sizeof(char *) * (size_t)(n + 2));
    if (!tmp) return;
    def->env        = tmp;
    def->env[n]     = strdup(pair);
    def->env[n + 1] = NULL;
}

static void load_env_dir(service_def_t *def, const char *env_dir) {
    DIR *d = opendir(env_dir);
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", env_dir, ent->d_name);

        FILE *f = fopen(path, "r");
        if (!f) continue;

        char val[LINE_MAX_LEN];
        val[0] = '\0';
        if (fgets(val, sizeof(val), f))
            val[strcspn(val, "\r\n")] = '\0';
        fclose(f);

        char pair[LINE_MAX_LEN + 256];
        snprintf(pair, sizeof(pair), "%s=%s", ent->d_name, val);
        append_env(def, pair);
    }
    closedir(d);
}

static char **parse_run_script(const char *run_path) {
    FILE *f = fopen(run_path, "r");
    if (!f) return NULL;

    char line[LINE_MAX_LEN];
    char last_exec[LINE_MAX_LEN];
    last_exec[0] = '\0';

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *s = trim(line);

        if (*s == '#' || *s == '\0') continue;

        if (strncmp(s, "exec ", 5) == 0) {
            char *cmd = trim(s + 5);
            strncpy(last_exec, cmd, sizeof(last_exec) - 1);
            last_exec[sizeof(last_exec) - 1] = '\0';
        }
    }
    fclose(f);

    if (last_exec[0] != '\0')
        return service_parse_argv(last_exec);

    return service_parse_argv(run_path);
}

service_def_t *service_parse_runit(const char *dir) {
    char run_path[512];
    snprintf(run_path, sizeof(run_path), "%s/run", dir);

    struct stat st;
    if (stat(run_path, &st) != 0) {
        log_warn("runit: no run script in %s", dir);
        return NULL;
    }

    service_def_t *def = service_alloc();

    def->argv = parse_run_script(run_path);
    if (!def->argv) {
        log_warn("runit: cannot extract command from %s", run_path);
        service_free(def);
        return NULL;
    }

    char down_path[512];
    snprintf(down_path, sizeof(down_path), "%s/down", dir);
    def->restart = (access(down_path, F_OK) != 0) ? 1 : 0;

    char env_dir[512];
    snprintf(env_dir, sizeof(env_dir), "%s/env", dir);
    load_env_dir(def, env_dir);

    const char *base = strrchr(dir, '/');
    def->name = strdup(base ? base + 1 : dir);

    return def;
}
