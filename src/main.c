// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "launcher.h"
#include "log.h"
#include "mount.h"
#include "reap.h"
#include "service.h"
#include "signal_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <unistd.h>

#define SERVICES_DIR "/etc/neoinit/services"

static void do_halt(int cmd) {
    log_info("shutting down");
    sync();
    unmount_all();
    log_close();
    reboot(cmd);
}

int main(void) {
    if (getpid() != 1) {
        fprintf(stderr, "neoinit: must run as PID 1\n");
        return 1;
    }

    log_init();
    log_info("starting");

    signals_setup();
    mount_essential();

    service_def_t **services = NULL;
    int count = service_load_dir(SERVICES_DIR, &services);
    if (count > 0) {
        services_launch_all(services, count);
        for (int i = 0; i < count; i++)
            service_free(services[i]);
        free(services);
    }

    log_info("entering main loop");

    for (;;) {
        pause();

        if (g_do_reap) {
            g_do_reap = 0;
            reap_zombies();
        }
        if (g_do_shutdown)
            do_halt(RB_POWER_OFF);
        if (g_do_reboot)
            do_halt(RB_AUTOBOOT);
    }
}
