// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#pragma once

#define NEOINIT_SOCK_PATH "/run/neoinit.sock"

int control_init(void);
void control_handle_data(int listen_fd);
