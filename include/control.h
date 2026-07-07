// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#define RAESIR_SOCK_PATH "/run/raesir.sock"

int control_init(void);
void control_handle_data(int listen_fd);
