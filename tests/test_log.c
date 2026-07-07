// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "log.h"

int main(void) {
    log_init();
    log_info("test info");
    log_warn("test warn");
    log_err("test err");
    log_close();
    log_close();

    log_msg(LOG_LEVEL_INFO, "after close: %s", "ok");

    TEST_DONE();
}
