// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "log.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

static int kmsg_fd = -1;

static const int kmsg_prio[] = { 6, 4, 3 };
static const char *level_tag[] = { "info", "warn", "err" };

void log_init(void) {
    kmsg_fd = open("/dev/kmsg", O_WRONLY | O_CLOEXEC);
}

void log_close(void) {
    if (kmsg_fd >= 0) {
        close(kmsg_fd);
        kmsg_fd = -1;
    }
}

void log_msg(log_level_t level, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0)
        return;

    if (kmsg_fd >= 0) {
        char kmsg[544];
        int kn = snprintf(kmsg, sizeof(kmsg), "<%d>neoinit: %s\n",
                          kmsg_prio[level], buf);
        if (kn > 0)
            write(kmsg_fd, kmsg, (size_t)kn);
    } else {
        fprintf(stderr, "neoinit [%s]: %s\n", level_tag[level], buf);
    }
}
