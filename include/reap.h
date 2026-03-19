// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#pragma once

#include <sys/types.h>

int reap_zombies(void);
void reap_track(pid_t pid, const char *name);
void reap_untrack(pid_t pid);
int reap_is_tracked(pid_t pid);
