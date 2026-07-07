// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERR,
} log_level_t;

void log_init(void);
void log_close(void);
void log_msg(log_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#define log_info(...) log_msg(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_warn(...) log_msg(LOG_LEVEL_WARN, __VA_ARGS__)
#define log_err(...)  log_msg(LOG_LEVEL_ERR,  __VA_ARGS__)
