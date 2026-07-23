// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "control.h"
#include "launcher.h"
#include "log.h"
#include "mount.h"
#include "order.h"
#include "reap.h"
#include "registry.h"
#include "service.h"
#include "signal_handler.h"

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef ENABLE_CONTAINER_DETECTION
static int is_container(void) {
    return (getenv("container") != NULL);
}
#endif

static void do_halt(int cmd) {
    log_info("shutting down");
    sync();

#ifdef ENABLE_CONTAINER_DETECTION
    if (is_container()) {
        log_info("container environment detected, exiting gracefully");
        log_close();
        exit(0);
    }
#endif

    unmount_all();
    log_close();
    reboot(cmd);
}

static void on_service_restart(service_entry_t *ent) {
    if (!ent->enabled) return;

    if (ent->def->restart_max > 0 &&
        ent->restart_count >= ent->def->restart_max) {
        log_warn("%s: restart limit (%d) reached, giving up",
                 ent->def->name, ent->def->restart_max);
        return;
    }

    log_info("restarting service: %s", ent->def->name);

    if (ent->def->restart_delay_ms > 0)
        usleep((useconds_t)ent->def->restart_delay_ms * 1000);

    pid_t pid = service_launch(ent->def);
    if (pid > 0) {
        log_info("restarted %s [pid %d]", ent->def->name, (int)pid);
        ent->pid = pid;
        ent->running = 1;
        ent->restart_count++;
    } else {
        log_err("failed to restart %s", ent->def->name);
    }
}

static int deps_satisfied(const service_def_t *def) {
    if (!def->requires) return 1;
    for (int i = 0; def->requires[i]; i++) {
        service_entry_t *dep = registry_find(def->requires[i]);
        if (!dep) return 0;
        if (dep->running) continue;
        if (dep->def->type == SERVICE_TYPE_ONESHOT && dep->exit_ok) continue;
        return 0;
    }
    return 1;
}

static void wait_oneshot(service_entry_t *ent) {
    int status = 0;

    for (;;) {
        if (waitpid(ent->pid, &status, 0) < 0) {
            if (errno == EINTR) continue;
            log_err("waitpid %s: %s", ent->def->name, strerror(errno));
            break;
        }
        break;
    }

    if (WIFEXITED(status)) {
        log_info("%s exited with %d", ent->def->name, WEXITSTATUS(status));
        ent->exit_ok = (WEXITSTATUS(status) == 0);
    } else if (WIFSIGNALED(status)) {
        log_warn("%s killed by signal %d", ent->def->name, WTERMSIG(status));
        ent->exit_ok = 0;
    }

    ent->running = 0;
    ent->pid = -1;
}

static void start_all_services(void) {
    service_def_t **defs = NULL;
    int count = service_load_dir(SERVICES_DIR, &defs);
    if (count <= 0) {
        free(defs);
        return;
    }

    int *levels = calloc((size_t)count, sizeof(int));
    if (!levels) {
        free(defs);
        return;
    }

    int nlevels = order_levels(defs, count, levels);
    if (nlevels < 0) {
        free(levels);
        free(defs);
        return;
    }

    for (int i = 0; i < count; i++)
        registry_add(defs[i]);

    for (int lv = 0; lv < nlevels; lv++) {
        for (int i = 0; i < count; i++) {
            if (levels[i] != lv) continue;

            service_entry_t *ent = registry_find(defs[i]->name);
            if (!ent) continue;

            if (!deps_satisfied(defs[i])) {
                log_err("skipping %s: unmet dependencies", defs[i]->name);
                continue;
            }

            pid_t pid = service_launch(defs[i]);
            if (pid > 0) {
                ent->pid = pid;
                ent->running = 1;
            }
        }

        for (int i = 0; i < count; i++) {
            if (levels[i] != lv) continue;
            if (defs[i]->type != SERVICE_TYPE_ONESHOT) continue;

            service_entry_t *ent = registry_find(defs[i]->name);
            if (!ent || !ent->running) continue;

            wait_oneshot(ent);
        }
    }

    free(levels);
    free(defs);
}

int main(void) {
    if (getpid() != 1) {
        fprintf(stderr, "raesir: must run as PID 1\n");
        return 1;
    }

    log_init();
    log_info("starting");

    signals_setup();
    mount_essential();
    registry_init();

    int ctrl_fd = control_init();

    start_all_services();

    log_info("entering main loop");

    struct pollfd fds[1];
    fds[0].fd     = ctrl_fd;
    fds[0].events = POLLIN;

    sigset_t sigmask;
    sigemptyset(&sigmask);

    for (;;) {
        int n = ppoll(fds, 1, NULL, &sigmask);
        if (n < 0) {
            if (errno == EINTR) goto check_signals;
            continue;
        }

        if (fds[0].revents & POLLIN)
            control_handle_data(ctrl_fd);

    check_signals:
        if (g_do_reap) {
            g_do_reap = 0;
            reap_zombies();
            registry_check_restarts(on_service_restart);
        }
        if (g_do_shutdown) do_halt(RB_POWER_OFF);
        if (g_do_reboot)   do_halt(RB_AUTOBOOT);
    }
}
