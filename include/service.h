// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

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
} service_def_t;

service_def_t *service_alloc(void);
void           service_free(service_def_t *def);

/*
 * Scan dir for *.yaml / *.service files, parse them, return an allocated
 * array of pointers (caller frees each element and the array itself).
 * Returns number of loaded definitions, -1 on hard error.
 */
int service_load_dir(const char *dir, service_def_t ***out);

/* Shared argv builder used by both parsers */
char **service_parse_argv(const char *cmd);

/* Format-specific parsers */
service_def_t *service_parse_yaml(const char *path);

#ifdef NEOINIT_SYSTEMD_COMPAT
service_def_t *service_parse_sd(const char *path);
#endif

#ifdef NEOINIT_RUNIT_COMPAT
service_def_t *service_parse_runit(const char *dir);
#endif
