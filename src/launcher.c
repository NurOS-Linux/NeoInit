// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "launcher.h"
#include "cgroup.h"
#include "log.h"
#include "reap.h"
#include "tty.h"

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVICE_LOG_DIR "/var/log/raesir"

static void redirect_output(const service_def_t *def) {
    char path[512];

    mkdir("/var/log", 0755);
    mkdir(SERVICE_LOG_DIR, 0755);

    snprintf(path, sizeof(path), SERVICE_LOG_DIR "/%s.log", def->name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0640);
    if (fd < 0) return;

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO) close(fd);
}

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
        /* Set a basic PATH if not present in env */
        if (!getenv("PATH")) {
#ifdef DEFAULT_PATH
            setenv("PATH", DEFAULT_PATH, 1);
#else
            setenv("PATH", _PATH_STDPATH, 1);
#endif
        }

        if (def->working_dir && chdir(def->working_dir) < 0)
            log_warn("chdir %s: %s", def->working_dir, strerror(errno));

        if (def->env) {
            for (int i = 0; def->env[i]; i++)
                putenv(def->env[i]);
        }

        if (def->tty) {
            if (tty_setup(def->tty) < 0)
                _exit(126);
        } else {
            redirect_output(def);
        }

        execv(def->argv[0], def->argv);
        _exit(127);
    }

    cgroup_setup(def, pid);
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
