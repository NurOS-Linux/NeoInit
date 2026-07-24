// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *toml_full =
    "name = \"testsvc\"\n"
    "description = \"Test service\"\n"
    "exec = \"/usr/bin/echo hello world\"\n"
    "working_dir = \"/tmp\"\n"
    "restart = true\n"
    "restart_delay_ms = 250\n"
    "restart_max = 5\n"
    "memory_max = \"128M\"\n"
    "cpu_weight = 200\n"
    "tty = \"tty2\"\n"
    "type = \"oneshot\"\n"
    "env = [\n"
    "  \"FOO=bar\",\n"
    "  \"BAZ=qux\",\n"
    "]\n"
    "after = [\"network\"]\n"
    "requires = [\"database\", \"cache\"]\n";

static const char *toml_minimal =
    "exec = \"/usr/bin/true\"\n";

static const char *toml_no_exec =
    "name = \"broken\"\n"
    "description = \"No exec\"\n";

static char *write_tmp(const char *content) {
    char path[] = "/tmp/raesir_test_XXXXXX.toml";
    int fd = mkstemps(path, 5);
    if (fd < 0) return NULL;
    write(fd, content, strlen(content));
    close(fd);
    return strdup(path);
}

int main(void) {
    char *path = write_tmp(toml_full);
    CHECK(path != NULL);

    service_def_t *def = service_parse_toml(path);
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
    CHECK(def->restart_delay_ms == 250);
    CHECK(def->restart_max == 5);
    CHECK(def->memory_max != NULL && strcmp(def->memory_max, "128M") == 0);
    CHECK(def->cpu_weight == 200);
    CHECK(def->tty != NULL && strcmp(def->tty, "tty2") == 0);
    CHECK(def->type == SERVICE_TYPE_ONESHOT);
    CHECK(def->env != NULL);
    CHECK(strcmp(def->env[0], "FOO=bar") == 0);
    CHECK(strcmp(def->env[1], "BAZ=qux") == 0);
    CHECK(def->env[2] == NULL);
    CHECK(def->after != NULL && strcmp(def->after[0], "network") == 0);
    CHECK(def->after[1] == NULL);
    CHECK(def->requires != NULL && strcmp(def->requires[0], "database") == 0);
    CHECK(def->requires[1] != NULL && strcmp(def->requires[1], "cache") == 0);
    CHECK(def->requires[2] == NULL);
    CHECK(def->wants == NULL);
    service_free(def);

    path = write_tmp(toml_minimal);
    CHECK(path != NULL);
    def = service_parse_toml(path);
    unlink(path); free(path);
    CHECK(def != NULL);
    CHECK(def->name != NULL);
    CHECK(def->restart == 0);
    CHECK(def->restart_delay_ms == 100);
    CHECK(def->restart_max == 0);
    CHECK(def->memory_max == NULL);
    CHECK(def->cpu_weight == 0);
    CHECK(def->tty == NULL);
    CHECK(def->type == SERVICE_TYPE_SIMPLE);
    CHECK(def->after == NULL);
    CHECK(def->requires == NULL);
    service_free(def);

    path = write_tmp(toml_no_exec);
    CHECK(path != NULL);
    def = service_parse_toml(path);
    unlink(path); free(path);
    CHECK(def == NULL);

    CHECK(service_name_valid("web") == 1);
    CHECK(service_name_valid("a-b_c.1") == 1);
    CHECK(service_name_valid(NULL) == 0);
    CHECK(service_name_valid("") == 0);
    CHECK(service_name_valid("a/b") == 0);
    CHECK(service_name_valid("../evil") == 0);
    CHECK(service_name_valid("..") == 0);
    CHECK(service_name_valid("has space") == 0);
    CHECK(service_name_valid("this-name-is-way-too-long-to-be-used-as-a-service-identifier-here") == 0);

    TEST_DONE();
}
