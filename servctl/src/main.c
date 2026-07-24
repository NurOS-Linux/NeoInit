// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAESIR_SOCK_PATH "/run/raesir.sock"

static void usage(const char *progname) {
    fprintf(stderr, "Usage: %s <command> [service]\n\n", progname);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  status         Check if raesir is alive\n");
    fprintf(stderr, "  list           List all services and their state\n");
    fprintf(stderr, "  start <name>   Launch a service\n");
    fprintf(stderr, "  stop <name>    Stop a running service\n");
    fprintf(stderr, "  restart <name> Restart a service\n");
    fprintf(stderr, "  reload <name>  Send SIGHUP to a running service\n");
    fprintf(stderr, "  enable <name>  Allow a service to be started and auto-restarted\n");
    fprintf(stderr, "  disable <name> Block a service from starting or auto-restarting\n");
    fprintf(stderr, "  rescan         Load new service definitions from disk\n");
    fprintf(stderr, "  loglevel <lvl> Set daemon log level (info, warn, err)\n");
    fprintf(stderr, "  protocol       Report control socket protocol version\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    char cmd_line[512];
    if (argc > 2)
        snprintf(cmd_line, sizeof(cmd_line), "%s %s", argv[1], argv[2]);
    else
        snprintf(cmd_line, sizeof(cmd_line), "%s", argv[1]);

    char buf[4096];
    if (servctl_run_command(RAESIR_SOCK_PATH, cmd_line, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "Error: Could not communicate with raesir at %s\n", RAESIR_SOCK_PATH);
        return 1;
    }

    printf("%s", buf);
    return 0;
}
