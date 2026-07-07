// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

/*
 * Minimal YAML parser for raesir service definitions.
 *
 * Supported syntax:
 *
 *   name: myservice
 *   description: Human readable text
 *   exec: /usr/bin/cmd --arg "quoted arg"
 *   working_dir: /var/lib/myservice
 *   restart: true
 *   type: simple          # or: oneshot
 *   env:
 *     - FOO=bar
 *     - BAZ=qux
 *   after:
 *     - network
 *   requires:
 *     - database
 */

#include "service.h"
#include "log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 1024

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) *--end = '\0';
    return s;
}

service_def_t *service_parse_yaml(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_warn("cannot open %s", path);
        return NULL;
    }

    service_def_t *def         = service_alloc();
    char           line[LINE_MAX_LEN];
    char           list_key[64] = ""; /* copied, not a pointer into line */

    while (fgets(line, sizeof(line), f)) {
        /* strip newline */
        line[strcspn(line, "\r\n")] = '\0';

        const char *raw = line;

        /* list item: leading whitespace + "- " */
        int indent = 0;
        while (raw[indent] == ' ' || raw[indent] == '\t') indent++;

        if (indent > 0 && raw[indent] == '-' && raw[indent + 1] == ' ') {
            const char *val = trim((char *)(raw + indent + 2));
            if (list_key[0] && strcmp(list_key, "env") == 0)
                def->env = service_strv_append(def->env, val);
            else if (list_key[0] && strcmp(list_key, "after") == 0)
                def->after = service_strv_append(def->after, val);
            else if (list_key[0] && strcmp(list_key, "requires") == 0)
                def->requires = service_strv_append(def->requires, val);
            continue;
        }

        /* skip comment and blank lines */
        char *trimmed = trim((char *)raw);
        if (*trimmed == '#' || *trimmed == '\0') continue;

        /* key: [value] */
        char *colon = strchr(trimmed, ':');
        if (!colon) continue;

        *colon       = '\0';
        char *key    = trim(trimmed);
        char *val    = trim(colon + 1);

        /* entering a list block (value is empty) */
        if (*val == '\0') {
            strncpy(list_key, key, sizeof(list_key) - 1);
            list_key[sizeof(list_key) - 1] = '\0';
            continue;
        }
        list_key[0] = '\0';

        if (strcmp(key, "name") == 0) {
            free(def->name);
            def->name = strdup(val);
        } else if (strcmp(key, "description") == 0) {
            free(def->description);
            def->description = strdup(val);
        } else if (strcmp(key, "exec") == 0) {
            if (def->argv) {
                for (int i = 0; def->argv[i]; i++) free(def->argv[i]);
                free(def->argv);
            }
            def->argv = service_parse_argv(val);
        } else if (strcmp(key, "working_dir") == 0) {
            free(def->working_dir);
            def->working_dir = strdup(val);
        } else if (strcmp(key, "restart") == 0) {
            def->restart = (strcmp(val, "true") == 0 || strcmp(val, "yes") == 0 || strcmp(val, "1") == 0);
        } else if (strcmp(key, "type") == 0) {
            if (strcmp(val, "oneshot") == 0)
                def->type = SERVICE_TYPE_ONESHOT;
            else
                def->type = SERVICE_TYPE_SIMPLE;
        }
    }

    fclose(f);

    if (!def->argv) {
        log_warn("%s: missing 'exec' field", path);
        service_free(def);
        return NULL;
    }

    if (!def->name) {
        /* derive name from filename without extension */
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
