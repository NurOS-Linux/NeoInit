// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "control.h"
#include "launcher.h"
#include "log.h"
#include "mount.h"
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
    log_info("restarting service: %s", ent->def->name);
    usleep(100000); /* 100ms throttle */
    pid_t pid = service_launch(ent->def);
    if (pid > 0) {
        log_info("restarted %s [pid %d]", ent->def->name, (int)pid);
        ent->pid = pid;
        ent->running = 1;
    } else {
        log_err("failed to restart %s", ent->def->name);
    }
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

    service_def_t **defs = NULL;
    int count = service_load_dir(SERVICES_DIR, &defs);
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            registry_add(defs[i]);
            pid_t pid = service_launch(defs[i]);
            service_entry_t *ent = NULL;
            if (pid > 0) {
                ent = registry_find(defs[i]->name);
                if (ent) {
                    ent->pid = pid;
                    ent->running = 1;
                }
            }

            if (defs[i]->type == SERVICE_TYPE_ONESHOT && pid > 0) {
                int status;
                waitpid_t:
                if (waitpid(pid, &status, 0) < 0) {
                    if (errno == EINTR) goto waitpid_t;
                    log_err("waitpid %s: %s", defs[i]->name, strerror(errno));
                }
                if (WIFEXITED(status))
                    log_info("%s exited with %d", defs[i]->name, WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    log_warn("%s killed by signal %d", defs[i]->name, WTERMSIG(status));
                if (ent) {
                    ent->running = 0;
                    ent->pid = -1;
                }
            }
        }
        free(defs);
    }

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
