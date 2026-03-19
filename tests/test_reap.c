// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "framework.h"
#include "reap.h"

#include <unistd.h>

int main(void) {
    reap_track(9999, "fake");
    CHECK(reap_is_tracked(9999) == 1);
    CHECK(reap_is_tracked(1234) == 0);

    reap_untrack(9999);
    CHECK(reap_is_tracked(9999) == 0);

    reap_untrack(9999);
    CHECK(1);

    pid_t pid = fork();
    if (pid == 0)
        _exit(0);

    CHECK(pid > 0);
    reap_track(pid, "test-child");
    CHECK(reap_is_tracked(pid) == 1);

    int reaped = 0;
    for (int i = 0; i < 100 && reaped == 0; i++) {
        usleep(5000);
        reaped = reap_zombies();
    }
    CHECK(reaped > 0);
    CHECK(reap_is_tracked(pid) == 0);
    CHECK(reap_zombies() == 0);

    TEST_DONE();
}
