// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "signal_handler.h"

#include <signal.h>

volatile sig_atomic_t g_do_reap     = 0;
volatile sig_atomic_t g_do_shutdown = 0;
volatile sig_atomic_t g_do_reboot   = 0;

static void on_sigchld(int sig) {
    (void)sig;
    g_do_reap = 1;
}

static void on_sigterm(int sig) {
    (void)sig;
    g_do_shutdown = 1;
}

static void on_sigint(int sig) {
    (void)sig;
    g_do_reboot = 1;
}

void signals_setup(void) {
    struct sigaction sa;

    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = on_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

    sa.sa_handler = on_sigterm;
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}
