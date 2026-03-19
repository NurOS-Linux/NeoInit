// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "launcher.h"
#include "log.h"
#include "reap.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t service_launch(const service_def_t *def) {
    if (!def || !def->argv || !def->argv[0]) {
        log_err("service_launch: invalid definition");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        log_err("fork %s: %s", def->name, strerror(errno));
        return -1;
    }

    if (pid == 0) {
        if (def->working_dir && chdir(def->working_dir) < 0) {
            /* non-fatal: log and continue */
        }

        if (def->env) {
            for (int i = 0; def->env[i]; i++)
                putenv(def->env[i]);
        }

        execv(def->argv[0], def->argv);
        _exit(127);
    }

    log_info("launched %s [%d]", def->name, (int)pid);
    reap_track(pid, def->name);
    return pid;
}

void services_launch_all(service_def_t **defs, int count) {
    for (int i = 0; i < count; i++) {
        service_def_t *def = defs[i];

        pid_t pid = service_launch(def);
        if (pid < 0) continue;

        /* oneshot: wait for completion before proceeding */
        if (def->type == SERVICE_TYPE_ONESHOT) {
            int status;
            waitpid_again:
            if (waitpid(pid, &status, 0) < 0) {
                if (errno == EINTR) goto waitpid_again;
                log_err("waitpid %s: %s", def->name, strerror(errno));
            }
            reap_untrack(pid);
        }
    }
}
