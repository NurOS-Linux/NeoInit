// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "control.h"
#include "log.h"
#include "launcher.h"
#include "registry.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int control_init(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, RAESIR_SOCK_PATH, sizeof(addr.sun_path) - 1);
    unlink(RAESIR_SOCK_PATH);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    listen(fd, 5);
    return fd;
}

static void cmd_status(int fd) {
    dprintf(fd, "raesir: active\n");
}

static void cmd_list(int fd) {
    int count = registry_count();
    dprintf(fd, "%-20s %-8s %s\n", "NAME", "PID", "STATUS");
    for (int i = 0; i < count; i++) {
        service_entry_t *ent = registry_get(i);
        dprintf(fd, "%-20s %-8d %s\n", ent->def->name, ent->pid,
                ent->running ? "running" : "stopped");
    }
}

static void cmd_stop(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    if (!ent->running || ent->pid <= 0) {
        dprintf(fd, "err: service '%s' not running\n", name);
        return;
    }
    if (kill(ent->pid, SIGTERM) < 0) {
        dprintf(fd, "err: kill: %s\n", strerror(errno));
    } else {
        dprintf(fd, "ok: stopping %s\n", name);
    }
}

static void cmd_start(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    if (ent->running) {
        dprintf(fd, "err: service '%s' already running\n", name);
        return;
    }
    pid_t pid = service_launch(ent->def);
    if (pid > 0) {
        ent->pid = pid;
        ent->running = 1;
        dprintf(fd, "ok: started %s [pid %d]\n", name, pid);
    } else {
        dprintf(fd, "err: failed to launch %s\n", name);
    }
}

void control_handle_data(int listen_fd) {
    int fd = accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC);
    if (fd < 0) return;

    char buf[512];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        char *cmd = strtok(buf, " \n\r");
        if (!cmd) {
            close(fd);
            return;
        }

        if (strcmp(cmd, "status") == 0) cmd_status(fd);
        else if (strcmp(cmd, "list") == 0) cmd_list(fd);
        else if (strcmp(cmd, "start") == 0) {
            char *arg = strtok(NULL, " \n\r");
            if (arg) cmd_start(fd, arg);
            else dprintf(fd, "err: missing service name\n");
        }
        else if (strcmp(cmd, "stop") == 0) {
            char *arg = strtok(NULL, " \n\r");
            if (arg) cmd_stop(fd, arg);
            else dprintf(fd, "err: missing service name\n");
        }
        else dprintf(fd, "err: unknown command\n");
    }
    close(fd);
}
