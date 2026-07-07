// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include <stdio.h>
#include <stdlib.h>

static int _t_count  = 0;
static int _t_passed = 0;

#define CHECK(cond) do {                                              \
    _t_count++;                                                       \
    if (cond) {                                                       \
        _t_passed++;                                                  \
        printf("  pass: %s\n", #cond);                               \
    } else {                                                          \
        printf("  FAIL: %s  (line %d)\n", #cond, __LINE__);          \
    }                                                                 \
} while (0)

#define TEST_DONE() do {                                              \
    printf("\n%d/%d passed\n", _t_passed, _t_count);                 \
    return (_t_passed == _t_count) ? 0 : 1;                          \
} while (0)
