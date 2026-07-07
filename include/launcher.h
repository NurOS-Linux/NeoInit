// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#pragma once

#include "service.h"
#include <sys/types.h>

pid_t service_launch(const service_def_t *def);
void  services_launch_all(service_def_t **defs, int count);
