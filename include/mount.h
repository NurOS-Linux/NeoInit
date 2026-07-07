// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include <stddef.h>

typedef struct {
    const char   *source;
    const char   *target;
    const char   *fstype;
    unsigned long flags;
} mount_entry_t;

void mount_essential(void);
void unmount_all(void);

const mount_entry_t *mount_table_get(void);
size_t mount_table_count(void);
