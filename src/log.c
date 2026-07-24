// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "log.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define LOG_FILE_PATH   "/var/log/raesir.log"
#define LOG_FILE_OLD    "/var/log/raesir.log.1"
#define LOG_ROTATE_SIZE (1024 * 1024)

static int         kmsg_fd   = -1;
static int         file_fd   = -1;
static log_level_t min_level = LOG_LEVEL_INFO;
static int         structured = 0;

static const int kmsg_prio[] = { 6, 4, 3 };
static const char *level_tag[] = { "info", "warn", "err" };

void log_init(void) {
    kmsg_fd = open("/dev/kmsg", O_WRONLY | O_CLOEXEC);
    mkdir("/var/log", 0755);
    file_fd = open(LOG_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0640);
}

void log_close(void) {
    if (kmsg_fd >= 0) {
        close(kmsg_fd);
        kmsg_fd = -1;
    }
    if (file_fd >= 0) {
        close(file_fd);
        file_fd = -1;
    }
}

void log_set_level(log_level_t level) {
    min_level = level;
}

log_level_t log_get_level(void) {
    return min_level;
}

void log_set_structured(int on) {
    structured = on;
}

int log_level_from_str(const char *s) {
    if (!s) return -1;
    for (int i = 0; i < 3; i++) {
        if (strcmp(s, level_tag[i]) == 0)
            return i;
    }
    return -1;
}

static void rotate_if_needed(void) {
    struct stat st;

    if (file_fd < 0) return;
    if (fstat(file_fd, &st) < 0) return;
    if (st.st_size < LOG_ROTATE_SIZE) return;

    close(file_fd);
    rename(LOG_FILE_PATH, LOG_FILE_OLD);
    file_fd = open(LOG_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0640);
}

void log_msg(log_level_t level, const char *fmt, ...) {
    if (level < min_level) return;

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
        int kn = snprintf(kmsg, sizeof(kmsg), "<%d>raesir: %s\n",
                          kmsg_prio[level], buf);
        if (kn > 0)
            write(kmsg_fd, kmsg, (size_t)kn);
    }

    if (file_fd >= 0) {
        rotate_if_needed();
        char rec[640];
        int rn;
        if (structured)
            rn = snprintf(rec, sizeof(rec), "ts=%lld level=%s msg=\"%s\"\n",
                          (long long)time(NULL), level_tag[level], buf);
        else
            rn = snprintf(rec, sizeof(rec), "[%lld] [%s] %s\n",
                          (long long)time(NULL), level_tag[level], buf);
        if (rn > 0 && file_fd >= 0)
            write(file_fd, rec, (size_t)rn);
    }

    fprintf(stderr, "raesir [%s]: %s\n", level_tag[level], buf);
}
