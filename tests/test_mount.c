// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "mount.h"

#include <string.h>

int main(void) {
    size_t n = mount_table_count();
    CHECK(n > 0);

    const mount_entry_t *t = mount_table_get();
    CHECK(t != NULL);

    int has_proc  = 0;
    int has_sysfs = 0;
    int has_dev   = 0;
    int has_run   = 0;

    for (size_t i = 0; i < n; i++) {
        CHECK(t[i].source != NULL);
        CHECK(t[i].target != NULL);
        CHECK(t[i].fstype != NULL);

        if (strcmp(t[i].target, "/proc") == 0) has_proc  = 1;
        if (strcmp(t[i].target, "/sys")  == 0) has_sysfs = 1;
        if (strcmp(t[i].target, "/dev")  == 0) has_dev   = 1;
        if (strcmp(t[i].target, "/run")  == 0) has_run   = 1;
    }

    CHECK(has_proc);
    CHECK(has_sysfs);
    CHECK(has_dev);
    CHECK(has_run);

    TEST_DONE();
}
