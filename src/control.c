// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "control.h"
#include "log.h"
#include "launcher.h"
#include "reap.h"
#include "registry.h"
#include "service.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
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
    if (!ent->enabled) {
        dprintf(fd, "err: service '%s' is disabled\n", name);
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
        ent->restart_count = 0;
        dprintf(fd, "ok: started %s [pid %d]\n", name, pid);
    } else {
        dprintf(fd, "err: failed to launch %s\n", name);
    }
}

static int stop_and_wait(service_entry_t *ent) {
    pid_t pid = ent->pid;

    if (kill(pid, SIGTERM) < 0)
        return -1;

    int waited_ms = 0;
    for (;;) {
        pid_t r = waitpid(pid, NULL, WNOHANG);
        if (r == pid || (r < 0 && errno == ECHILD))
            break;
        if (waited_ms >= 5000) {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            break;
        }
        usleep(100000);
        waited_ms += 100;
    }

    reap_untrack(pid);
    ent->running = 0;
    ent->pid = -1;
    return 0;
}

static void cmd_restart(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    if (!ent->enabled) {
        dprintf(fd, "err: service '%s' is disabled\n", name);
        return;
    }
    if (ent->running && ent->pid > 0) {
        if (stop_and_wait(ent) < 0) {
            dprintf(fd, "err: kill: %s\n", strerror(errno));
            return;
        }
    }
    pid_t pid = service_launch(ent->def);
    if (pid > 0) {
        ent->pid = pid;
        ent->running = 1;
        ent->restart_count = 0;
        dprintf(fd, "ok: restarted %s [pid %d]\n", name, pid);
    } else {
        dprintf(fd, "err: failed to launch %s\n", name);
    }
}

static void cmd_reload(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    if (!ent->running || ent->pid <= 0) {
        dprintf(fd, "err: service '%s' not running\n", name);
        return;
    }
    if (kill(ent->pid, SIGHUP) < 0) {
        dprintf(fd, "err: kill: %s\n", strerror(errno));
    } else {
        dprintf(fd, "ok: reload signal sent to %s\n", name);
    }
}

static void cmd_enable(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    ent->enabled = 1;
    dprintf(fd, "ok: enabled %s\n", name);
}

static void cmd_disable(int fd, const char *name) {
    service_entry_t *ent = registry_find(name);
    if (!ent) {
        dprintf(fd, "err: service '%s' not found\n", name);
        return;
    }
    ent->enabled = 0;
    dprintf(fd, "ok: disabled %s\n", name);
}

static void cmd_rescan(int fd) {
    service_def_t **defs = NULL;
    int count = service_load_dir(SERVICES_DIR, &defs);
    int added = 0;

    for (int i = 0; i < count; i++) {
        if (registry_find(defs[i]->name)) {
            service_free(defs[i]);
        } else if (registry_add(defs[i]) == 0) {
            added++;
        } else {
            service_free(defs[i]);
        }
    }
    free(defs);

    dprintf(fd, "ok: %d new service(s) registered\n", added);
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
        else if (strcmp(cmd, "rescan") == 0) cmd_rescan(fd);
        else if (strcmp(cmd, "start") == 0 || strcmp(cmd, "stop") == 0 ||
                 strcmp(cmd, "restart") == 0 || strcmp(cmd, "reload") == 0 ||
                 strcmp(cmd, "enable") == 0 || strcmp(cmd, "disable") == 0) {
            char *arg = strtok(NULL, " \n\r");
            if (!arg) {
                dprintf(fd, "err: missing service name\n");
            } else if (strcmp(cmd, "start") == 0) {
                cmd_start(fd, arg);
            } else if (strcmp(cmd, "stop") == 0) {
                cmd_stop(fd, arg);
            } else if (strcmp(cmd, "restart") == 0) {
                cmd_restart(fd, arg);
            } else if (strcmp(cmd, "reload") == 0) {
                cmd_reload(fd, arg);
            } else if (strcmp(cmd, "enable") == 0) {
                cmd_enable(fd, arg);
            } else {
                cmd_disable(fd, arg);
            }
        }
        else dprintf(fd, "err: unknown command\n");
    }
    close(fd);
}
