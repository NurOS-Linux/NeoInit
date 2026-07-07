// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "reap.h"
#include "log.h"
#include "registry.h"

#include <string.h>
#include <sys/wait.h>

#define TRACK_MAX 64

typedef struct {
    pid_t pid;
    char  name[64];
} tracked_t;

static tracked_t tracked[TRACK_MAX];
static int tracked_count = 0;

void reap_track(pid_t pid, const char *name) {
    if (tracked_count >= TRACK_MAX)
        return;
    tracked[tracked_count].pid = pid;
    strncpy(tracked[tracked_count].name, name, sizeof(tracked[0].name) - 1);
    tracked[tracked_count].name[sizeof(tracked[0].name) - 1] = '\0';
    tracked_count++;
}

void reap_untrack(pid_t pid) {
    for (int i = 0; i < tracked_count; i++) {
        if (tracked[i].pid == pid) {
            tracked[i] = tracked[--tracked_count];
            return;
        }
    }
}

int reap_is_tracked(pid_t pid) {
    for (int i = 0; i < tracked_count; i++) {
        if (tracked[i].pid == pid)
            return 1;
    }
    return 0;
}

static const char *find_name(pid_t pid) {
    for (int i = 0; i < tracked_count; i++) {
        if (tracked[i].pid == pid)
            return tracked[i].name;
    }
    return "(unknown)";
}

int reap_zombies(void) {
    int status;
    pid_t pid;
    int reaped = 0;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        const char *name = find_name(pid);
        
        if (WIFEXITED(status))
            log_info("reaped %s [%d] exit=%d", name, (int)pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            log_warn("reaped %s [%d] signal=%d", name, (int)pid, WTERMSIG(status));
        
        registry_update_pid(pid, 0);
        
        reap_untrack(pid);
        reaped++;
    }

    return reaped;
}
