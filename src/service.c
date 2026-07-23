// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "service.h"
#include "log.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

service_def_t *service_alloc(void) {
    service_def_t *def = calloc(1, sizeof(*def));
    if (def) {
        def->restart_delay_ms = 100;
        def->restart_max      = 0;
    }
    return def;
}

static void free_strv(char **v) {
    if (!v) return;
    for (int i = 0; v[i]; i++)
        free(v[i]);
    free(v);
}

void service_free(service_def_t *def) {
    if (!def) return;
    free(def->name);
    free(def->description);
    free(def->working_dir);
    free_strv(def->argv);
    free_strv(def->env);
    free_strv(def->after);
    free_strv(def->requires);
    free_strv(def->wants);
    free(def);
}

char **service_strv_append(char **v, const char *val) {
    int n = 0;
    if (v) {
        while (v[n]) n++;
    }
    char **tmp = realloc(v, sizeof(char *) * (size_t)(n + 2));
    if (!tmp) return v;
    tmp[n]     = strdup(val);
    tmp[n + 1] = NULL;
    return tmp;
}

char **service_parse_argv(const char *cmd) {
    char **argv = NULL;
    int    argc = 0;
    char   buf[1024];
    const char *p = cmd;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        int   len   = 0;
        char  quote = 0;

        while (*p && (quote || (*p != ' ' && *p != '\t'))) {
            if (!quote && (*p == '\'' || *p == '"')) {
                quote = *p++;
            } else if (quote && *p == quote) {
                quote = 0; p++;
            } else if (!quote && *p == '\\' && *(p + 1)) {
                p++;
                if (len < (int)sizeof(buf) - 1) buf[len++] = *p++;
            } else {
                if (len < (int)sizeof(buf) - 1) buf[len++] = *p++;
                else p++;
            }
        }
        buf[len] = '\0';

        char **tmp = realloc(argv, sizeof(char *) * (size_t)(argc + 2));
        if (!tmp) { free_strv(argv); return NULL; }
        argv = tmp;
        argv[argc++] = strdup(buf);
        argv[argc]   = NULL;
    }

    return argv;
}

static const char *file_ext(const char *name) {
    const char *dot = strrchr(name, '.');
    return dot ? dot + 1 : "";
}

int service_load_dir(const char *dir, service_def_t ***out) {
    DIR *d = opendir(dir);
    if (!d) {
        if (errno != ENOENT)
            log_warn("opendir %s: %s", dir, strerror(errno));
        *out = NULL;
        return 0;
    }

    service_def_t **list  = NULL;
    int             count = 0;
    struct dirent  *ent;

    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);

        const char    *ext = file_ext(ent->d_name);
        service_def_t *def = NULL;

        if (strcmp(ext, "toml") == 0) {
            def = service_parse_toml(path);
#ifdef RAESIR_YAML_COMPAT
        } else if (strcmp(ext, "yaml") == 0 || strcmp(ext, "yml") == 0) {
            def = service_parse_yaml(path);
#endif
#ifdef RAESIR_SYSTEMD_COMPAT
        } else if (strcmp(ext, "service") == 0) {
            def = service_parse_sd(path);
#endif
#ifdef RAESIR_RUNIT_COMPAT
        } else {
            struct stat st;
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
                def = service_parse_runit(path);
            else
                continue;
#else
        } else {
            continue;
#endif
        }

        if (!def) {
            log_warn("failed to parse %s", path);
            continue;
        }

        service_def_t **tmp = realloc(list, sizeof(*list) * (size_t)(count + 1));
        if (!tmp) { service_free(def); continue; }
        list = tmp;
        list[count++] = def;
    }

    closedir(d);
    *out = list;
    return count;
}
