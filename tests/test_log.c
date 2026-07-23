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

    CHECK(log_level_from_str("info") == LOG_LEVEL_INFO);
    CHECK(log_level_from_str("warn") == LOG_LEVEL_WARN);
    CHECK(log_level_from_str("err") == LOG_LEVEL_ERR);
    CHECK(log_level_from_str("bogus") == -1);
    CHECK(log_level_from_str(NULL) == -1);

    log_set_level(LOG_LEVEL_ERR);
    CHECK(log_get_level() == LOG_LEVEL_ERR);
    log_info("filtered, must not appear");
    log_set_level(LOG_LEVEL_INFO);
    CHECK(log_get_level() == LOG_LEVEL_INFO);

    log_set_structured(1);
    log_info("structured record");
    log_set_structured(0);

    TEST_DONE();
}
