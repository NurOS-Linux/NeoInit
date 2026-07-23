// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include "service.h"
#include <sys/types.h>

typedef struct {
    service_def_t *def;
    pid_t          pid;
    int            running;
    int            exit_ok;
    int            enabled;
    int            restart_count;
    int            stop_requested;
} service_entry_t;

void             registry_init(void);
int              registry_add(service_def_t *def);
service_entry_t *registry_find(const char *name);
service_entry_t *registry_find_by_pid(pid_t pid);
int              registry_count(void);
service_entry_t *registry_get(int index);
void             registry_update_pid(pid_t pid, int running);
void             registry_check_restarts(void (*restart_cb)(service_entry_t *));
