// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "framework.h"
#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *yaml_full =
    "name: testsvc\n"
    "description: Test service\n"
    "exec: /usr/bin/echo hello world\n"
    "working_dir: /tmp\n"
    "restart: true\n"
    "type: oneshot\n"
    "env:\n"
    "  - FOO=bar\n"
    "  - BAZ=qux\n";

static const char *yaml_minimal =
    "exec: /usr/bin/true\n";

static const char *yaml_no_exec =
    "name: broken\n"
    "description: No exec\n";

static char *write_tmp(const char *content) {
    char path[] = "/tmp/neoinit_test_XXXXXX.yaml";
    int fd = mkstemps(path, 5);
    if (fd < 0) return NULL;
    write(fd, content, strlen(content));
    close(fd);
    return strdup(path);
}

int main(void) {
    /* full definition */
    char *path = write_tmp(yaml_full);
    CHECK(path != NULL);

    service_def_t *def = service_parse_yaml(path);
    unlink(path); free(path);

    CHECK(def != NULL);
    CHECK(def->name != NULL && strcmp(def->name, "testsvc") == 0);
    CHECK(def->description != NULL && strcmp(def->description, "Test service") == 0);
    CHECK(def->argv != NULL && strcmp(def->argv[0], "/usr/bin/echo") == 0);
    CHECK(def->argv[1] != NULL && strcmp(def->argv[1], "hello") == 0);
    CHECK(def->argv[2] != NULL && strcmp(def->argv[2], "world") == 0);
    CHECK(def->argv[3] == NULL);
    CHECK(def->working_dir != NULL && strcmp(def->working_dir, "/tmp") == 0);
    CHECK(def->restart == 1);
    CHECK(def->type == SERVICE_TYPE_ONESHOT);
    CHECK(def->env != NULL);
    CHECK(strcmp(def->env[0], "FOO=bar") == 0);
    CHECK(strcmp(def->env[1], "BAZ=qux") == 0);
    CHECK(def->env[2] == NULL);
    service_free(def);

    /* minimal: name derived from filename */
    path = write_tmp(yaml_minimal);
    CHECK(path != NULL);
    def = service_parse_yaml(path);
    unlink(path); free(path);
    CHECK(def != NULL);
    CHECK(def->name != NULL);
    CHECK(def->restart == 0);
    CHECK(def->type == SERVICE_TYPE_SIMPLE);
    service_free(def);

    /* missing exec → NULL */
    path = write_tmp(yaml_no_exec);
    CHECK(path != NULL);
    def = service_parse_yaml(path);
    unlink(path); free(path);
    CHECK(def == NULL);

    TEST_DONE();
}
