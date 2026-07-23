// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

/*
 * Minimal TOML parser for raesir service definitions.
 *
 * Supported syntax:
 *
 *   name = "myservice"
 *   description = "Human readable text"
 *   exec = "/usr/bin/cmd --arg \"quoted arg\""
 *   working_dir = "/var/lib/myservice"
 *   restart = true
 *   type = "simple"          # or: "oneshot"
 *   env = [
 *     "FOO=bar",
 *     "BAZ=qux",
 *   ]
 *   after = ["network"]
 *   requires = ["database"]
 *
 * Basic ("...") and literal ('...') strings, booleans and string arrays
 * (single-line or spanning multiple lines) are supported. Comments start
 * with '#' at the beginning of a line.
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

static char *toml_unquote(const char *p, const char **end) {
    char quote = *p;
    if (quote != '"' && quote != '\'') return NULL;
    p++;

    char  buf[LINE_MAX_LEN];
    int   len = 0;

    while (*p && *p != quote) {
        if (quote == '"' && *p == '\\' && *(p + 1)) {
            p++;
            char c;
            switch (*p) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '"': c = '"'; break;
                case '\\': c = '\\'; break;
                default: c = *p; break;
            }
            if (len < (int)sizeof(buf) - 1) buf[len++] = c;
            p++;
        } else {
            if (len < (int)sizeof(buf) - 1) buf[len++] = *p;
            p++;
        }
    }

    if (*p != quote) return NULL;
    buf[len] = '\0';
    *end = p + 1;
    return strdup(buf);
}

static void array_append(service_def_t *def, const char *key, const char *val) {
    if (strcmp(key, "env") == 0)
        def->env = service_strv_append(def->env, val);
    else if (strcmp(key, "after") == 0)
        def->after = service_strv_append(def->after, val);
    else if (strcmp(key, "requires") == 0)
        def->requires = service_strv_append(def->requires, val);
    else if (strcmp(key, "wants") == 0)
        def->wants = service_strv_append(def->wants, val);
}

static void parse_array_body(service_def_t *def, const char *key, const char *text, int *in_array) {
    const char *p = text;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (*p == '\0' || *p == '#') return;
        if (*p == ']') {
            *in_array = 0;
            return;
        }
        if (*p == '"' || *p == '\'') {
            const char *after;
            char *val = toml_unquote(p, &after);
            if (!val) return;
            array_append(def, key, val);
            free(val);
            p = after;
        } else {
            p++;
        }
    }
}

service_def_t *service_parse_toml(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_warn("cannot open %s", path);
        return NULL;
    }

    service_def_t *def          = service_alloc();
    char           line[LINE_MAX_LEN];
    char           array_key[64] = "";
    int            in_array      = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (in_array) {
            parse_array_body(def, array_key, line, &in_array);
            continue;
        }

        char *trimmed = trim(line);
        if (*trimmed == '#' || *trimmed == '\0') continue;

        char *eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq       = '\0';
        char *key = trim(trimmed);
        char *val = trim(eq + 1);

        if (*val == '[') {
            strncpy(array_key, key, sizeof(array_key) - 1);
            array_key[sizeof(array_key) - 1] = '\0';
            in_array = 1;
            parse_array_body(def, array_key, val + 1, &in_array);
            continue;
        }

        char *str_val = NULL;
        if (*val == '"' || *val == '\'') {
            const char *after;
            str_val = toml_unquote(val, &after);
        }

        if (strcmp(key, "name") == 0) {
            if (str_val) {
                free(def->name);
                def->name = str_val;
                str_val   = NULL;
            }
        } else if (strcmp(key, "description") == 0) {
            if (str_val) {
                free(def->description);
                def->description = str_val;
                str_val           = NULL;
            }
        } else if (strcmp(key, "exec") == 0) {
            if (str_val) {
                if (def->argv) {
                    for (int i = 0; def->argv[i]; i++) free(def->argv[i]);
                    free(def->argv);
                }
                def->argv = service_parse_argv(str_val);
            }
        } else if (strcmp(key, "working_dir") == 0) {
            if (str_val) {
                free(def->working_dir);
                def->working_dir = str_val;
                str_val           = NULL;
            }
        } else if (strcmp(key, "memory_max") == 0) {
            if (str_val) {
                free(def->memory_max);
                def->memory_max = str_val;
                str_val         = NULL;
            }
        } else if (strcmp(key, "tty") == 0) {
            if (str_val) {
                free(def->tty);
                def->tty = str_val;
                str_val  = NULL;
            }
        } else if (strcmp(key, "cpu_weight") == 0) {
            def->cpu_weight = atoi(val);
        } else if (strcmp(key, "restart") == 0) {
            def->restart = (strcmp(val, "true") == 0);
        } else if (strcmp(key, "restart_delay_ms") == 0) {
            def->restart_delay_ms = atoi(val);
        } else if (strcmp(key, "restart_max") == 0) {
            def->restart_max = atoi(val);
        } else if (strcmp(key, "type") == 0) {
            if (str_val && strcmp(str_val, "oneshot") == 0)
                def->type = SERVICE_TYPE_ONESHOT;
            else
                def->type = SERVICE_TYPE_SIMPLE;
        }

        free(str_val);
    }

    fclose(f);

    if (!def->argv) {
        log_warn("%s: missing 'exec' field", path);
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
