// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "registry.h"
#include <stdlib.h>
#include <string.h>

#define REGISTRY_MAX 128

static service_entry_t registry[REGISTRY_MAX];
static int             registry_size = 0;

void registry_init(void) {
    memset(registry, 0, sizeof(registry));
    registry_size = 0;
}

int registry_add(service_def_t *def) {
    if (registry_size >= REGISTRY_MAX) return -1;
    registry[registry_size].def     = def;
    registry[registry_size].pid     = -1;
    registry[registry_size].running = 0;
    registry_size++;
    return 0;
}

service_entry_t *registry_find(const char *name) {
    for (int i = 0; i < registry_size; i++) {
        if (strcmp(registry[i].def->name, name) == 0)
            return &registry[i];
    }
    return NULL;
}

service_entry_t *registry_find_by_pid(pid_t pid) {
    for (int i = 0; i < registry_size; i++) {
        if (registry[i].pid == pid)
            return &registry[i];
    }
    return NULL;
}

int registry_count(void) {
    return registry_size;
}

service_entry_t *registry_get(int index) {
    if (index < 0 || index >= registry_size) return NULL;
    return &registry[index];
}

void registry_update_pid(pid_t pid, int running) {
    service_entry_t *ent = registry_find_by_pid(pid);
    if (ent) {
        ent->running = running;
        if (!running) ent->pid = -1;
    }
}

void registry_check_restarts(void (*restart_cb)(service_entry_t *)) {
    for (int i = 0; i < registry_size; i++) {
        service_entry_t *ent = &registry[i];
        if (!ent->running && ent->def->restart) {
            restart_cb(ent);
        }
    }
}
