// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "client.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int servctl_run_command(const char *sock_path, const char *command, char *out_buf, int buf_size) {
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    if (write(fd, command, strlen(command)) < 0) {
        close(fd);
        return -1;
    }

    int total = 0;
    ssize_t n;
    while ((n = read(fd, out_buf + total, (size_t)(buf_size - total - 1))) > 0) {
        total += (int)n;
        if (total >= buf_size - 1) break;
    }
    out_buf[total] = '\0';

    close(fd);
    return 0;
}
