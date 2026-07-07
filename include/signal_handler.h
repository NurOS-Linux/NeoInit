// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include <signal.h>

extern volatile sig_atomic_t g_do_reap;
extern volatile sig_atomic_t g_do_shutdown;
extern volatile sig_atomic_t g_do_reboot;

void signals_setup(void);
