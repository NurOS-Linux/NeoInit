// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *sd_full =
    "[Unit]\n"
    "Description=Test systemd service\n"
    "After=network database\n"
    "Requires=database\n"
    "Wants=cache\n"
    "Wants=metrics\n"
    "\n"
    "[Service]\n"
    "ExecStart=/usr/bin/echo hello world\n"
    "WorkingDirectory=/tmp\n"
    "Environment=FOO=bar\n"
    "Environment=BAZ=qux\n"
    "Restart=on-failure\n"
    "Type=oneshot\n"
    "\n"
    "[Install]\n"
    "WantedBy=multi-user.target\n";

static const char *sd_no_exec =
    "[Unit]\n"
    "Description=Broken\n"
    "[Service]\n"
    "Type=simple\n";

static char *write_tmp(const char *content) {
    char path[] = "/tmp/raesir_test_XXXXXX.service";
    int fd = mkstemps(path, 8);
    if (fd < 0) return NULL;
    write(fd, content, strlen(content));
    close(fd);
    return strdup(path);
}

int main(void) {
    char *path = write_tmp(sd_full);
    CHECK(path != NULL);

    service_def_t *def = service_parse_sd(path);
    unlink(path); free(path);

    CHECK(def != NULL);
    CHECK(def->description != NULL && strcmp(def->description, "Test systemd service") == 0);
    CHECK(def->argv != NULL && strcmp(def->argv[0], "/usr/bin/echo") == 0);
    CHECK(def->argv[1] != NULL && strcmp(def->argv[1], "hello") == 0);
    CHECK(def->argv[3] == NULL);
    CHECK(def->working_dir != NULL && strcmp(def->working_dir, "/tmp") == 0);
    CHECK(def->env != NULL && strcmp(def->env[0], "FOO=bar") == 0);
    CHECK(def->env[1] != NULL && strcmp(def->env[1], "BAZ=qux") == 0);
    CHECK(def->env[2] == NULL);
    CHECK(def->restart == 1);
    CHECK(def->type == SERVICE_TYPE_ONESHOT);
    CHECK(def->after != NULL && strcmp(def->after[0], "network") == 0);
    CHECK(def->after[1] != NULL && strcmp(def->after[1], "database") == 0);
    CHECK(def->after[2] == NULL);
    CHECK(def->requires != NULL && strcmp(def->requires[0], "database") == 0);
    CHECK(def->requires[1] == NULL);
    CHECK(def->wants != NULL && strcmp(def->wants[0], "cache") == 0);
    CHECK(def->wants[1] != NULL && strcmp(def->wants[1], "metrics") == 0);
    CHECK(def->wants[2] == NULL);
    service_free(def);

    /* missing ExecStart → NULL */
    path = write_tmp(sd_no_exec);
    CHECK(path != NULL);
    def = service_parse_sd(path);
    unlink(path); free(path);
    CHECK(def == NULL);

    TEST_DONE();
}
