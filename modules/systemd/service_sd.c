// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

/*
 * Compatibility parser for systemd .service unit files.
 * Only the subset relevant to raesir is recognised:
 *
 *   [Unit]
 *   Description=
 *   After=       (space-separated service names, ordering only)
 *   Requires=    (space-separated service names, hard dependency)
 *   Wants=       (space-separated service names, weak dependency)
 *
 *   [Service]
 *   ExecStart=
 *   WorkingDirectory=
 *   Environment=KEY=VALUE
 *   Restart=       (no → 0, anything else → 1)
 *   Type=          (simple | oneshot)
 *
 * All other keys and sections are silently ignored.
 */

#include "service.h"
#include "log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 1024

typedef enum { SEC_NONE, SEC_UNIT, SEC_SERVICE, SEC_INSTALL } section_t;

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) *--end = '\0';
    return s;
}

static char **append_tokens(char **arr, const char *val) {
    char buf[LINE_MAX_LEN];
    strncpy(buf, val, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    for (char *tok = strtok(buf, " \t"); tok; tok = strtok(NULL, " \t"))
        arr = service_strv_append(arr, tok);

    return arr;
}

service_def_t *service_parse_sd(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_warn("cannot open %s", path);
        return NULL;
    }

    service_def_t *def = service_alloc();
    char           line[LINE_MAX_LEN];
    section_t      sec = SEC_NONE;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *s = trim(line);

        if (*s == '#' || *s == ';' || *s == '\0') continue;

        /* section header */
        if (*s == '[') {
            char *end = strchr(s, ']');
            if (!end) continue;
            *end = '\0';
            char *name = s + 1;
            if (strcmp(name, "Unit") == 0)         sec = SEC_UNIT;
            else if (strcmp(name, "Service") == 0) sec = SEC_SERVICE;
            else if (strcmp(name, "Install") == 0) sec = SEC_INSTALL;
            else                                   sec = SEC_NONE;
            continue;
        }

        char *eq = strchr(s, '=');
        if (!eq) continue;
        *eq      = '\0';
        char *key = trim(s);
        char *val = trim(eq + 1);

        switch (sec) {
        case SEC_UNIT:
            if (strcmp(key, "Description") == 0) {
                free(def->description);
                def->description = strdup(val);
            } else if (strcmp(key, "After") == 0) {
                def->after = append_tokens(def->after, val);
            } else if (strcmp(key, "Requires") == 0) {
                def->requires = append_tokens(def->requires, val);
            } else if (strcmp(key, "Wants") == 0) {
                def->wants = append_tokens(def->wants, val);
            }
            break;

        case SEC_SERVICE:
            if (strcmp(key, "ExecStart") == 0) {
                if (def->argv) {
                    for (int i = 0; def->argv[i]; i++) free(def->argv[i]);
                    free(def->argv);
                }
                def->argv = service_parse_argv(val);
            } else if (strcmp(key, "WorkingDirectory") == 0) {
                free(def->working_dir);
                def->working_dir = strdup(val);
            } else if (strcmp(key, "Environment") == 0) {
                def->env = service_strv_append(def->env, val);
            } else if (strcmp(key, "MemoryMax") == 0) {
                free(def->memory_max);
                def->memory_max = strdup(val);
            } else if (strcmp(key, "TTYPath") == 0) {
                free(def->tty);
                def->tty = strdup(val);
            } else if (strcmp(key, "CPUWeight") == 0) {
                def->cpu_weight = atoi(val);
            } else if (strcmp(key, "Restart") == 0) {
                def->restart = (strcmp(val, "no") != 0);
            } else if (strcmp(key, "RestartSec") == 0) {
                def->restart_delay_ms = (int)(atof(val) * 1000.0);
            } else if (strcmp(key, "StartLimitBurst") == 0) {
                def->restart_max = atoi(val);
            } else if (strcmp(key, "Type") == 0) {
                if (strcmp(val, "oneshot") == 0)
                    def->type = SERVICE_TYPE_ONESHOT;
                else
                    def->type = SERVICE_TYPE_SIMPLE;
            }
            break;

        default:
            break;
        }
    }

    fclose(f);

    if (!def->argv) {
        log_warn("%s: missing ExecStart", path);
        service_free(def);
        return NULL;
    }

    if (!def->name) {
        const char *base = strrchr(path, '/');
        base = base ? base + 1 : path;
        char tmp[256];
        strncpy(tmp, base, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        char *dot = strrchr(tmp, '.');
        if (dot) *dot = '\0';
        def->name = strdup(tmp);
    }

    return def;
}
