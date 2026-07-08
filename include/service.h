// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

typedef enum {
    SERVICE_TYPE_SIMPLE  = 0,
    SERVICE_TYPE_ONESHOT,
} service_type_t;

typedef struct {
    char          *name;
    char          *description;
    char         **argv;        /* NULL-terminated, argv[0] is the executable */
    char         **env;         /* NULL-terminated KEY=VALUE pairs            */
    char          *working_dir;
    service_type_t type;
    int            restart;     /* 1 = restart on failure                     */
    char         **after;       /* NULL-terminated service names, ordering only        */
    char         **requires;    /* NULL-terminated service names, hard dependency      */
    char         **wants;       /* NULL-terminated service names, weak dependency      */
} service_def_t;

service_def_t *service_alloc(void);
void           service_free(service_def_t *def);

char **service_strv_append(char **v, const char *val);

/*
 * Scan dir for *.toml / *.service files (and *.yaml / *.yml when built with
 * YAML compatibility), parse them, return an allocated array of pointers
 * (caller frees each element and the array itself).
 * Returns number of loaded definitions, -1 on hard error.
 */
int service_load_dir(const char *dir, service_def_t ***out);

/* Shared argv builder used by all parsers */
char **service_parse_argv(const char *cmd);

/* Native format parser */
service_def_t *service_parse_toml(const char *path);

#ifdef RAESIR_SYSTEMD_COMPAT
service_def_t *service_parse_sd(const char *path);
#endif

#ifdef RAESIR_RUNIT_COMPAT
service_def_t *service_parse_runit(const char *dir);
#endif

#ifdef RAESIR_YAML_COMPAT
service_def_t *service_parse_yaml(const char *path);
#endif
