// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "service.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char *make_tmpdir(void) {
    char tmpl[] = "/tmp/raesir_runit_XXXXXX";
    return strdup(mkdtemp(tmpl));
}

static void write_file(const char *dir, const char *name, const char *content) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd < 0) return;
    write(fd, content, strlen(content));
    close(fd);
}

static void make_dir(const char *dir, const char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    mkdir(path, 0755);
}

static void write_env_var(const char *svc_dir, const char *var, const char *val) {
    char env_dir[512];
    snprintf(env_dir, sizeof(env_dir), "%s/env", svc_dir);
    mkdir(env_dir, 0755);
    write_file(env_dir, var, val);
}

static void rm_rf(const char *path) {
    DIR *d = opendir(path);
    if (!d) { unlink(path); return; }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char sub[512];
        snprintf(sub, sizeof(sub), "%s/%s", path, ent->d_name);
        rm_rf(sub);
    }
    closedir(d);
    rmdir(path);
}

static int env_has(char **env, const char *pair) {
    if (!env) return 0;
    for (int i = 0; env[i]; i++)
        if (strcmp(env[i], pair) == 0) return 1;
    return 0;
}

int main(void) {
    char *svc_dir;

    /* exec line is picked up correctly, name derived from directory */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run",
        "#!/bin/sh\n"
        "# start the daemon\n"
        "exec /usr/bin/myapp --foreground\n");
    service_def_t *def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(def->argv != NULL);
    CHECK(def->argv[0] != NULL && strcmp(def->argv[0], "/usr/bin/myapp") == 0);
    CHECK(def->argv[1] != NULL && strcmp(def->argv[1], "--foreground") == 0);
    CHECK(def->argv[2] == NULL);
    CHECK(def->restart == 1);
    CHECK(def->name != NULL);
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    /* last exec line wins when multiple exec lines are present */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run",
        "#!/bin/sh\n"
        "exec chpst -u nobody /usr/bin/myapp\n"
        "exec /usr/bin/fallback\n");
    def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(def->argv != NULL && strcmp(def->argv[0], "/usr/bin/fallback") == 0);
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    /* no exec line: run script itself is the command */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run",
        "#!/bin/sh\n"
        "/usr/bin/myapp --no-daemon\n");
    def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(def->argv != NULL);
    char expected_run[512];
    snprintf(expected_run, sizeof(expected_run), "%s/run", svc_dir);
    CHECK(strcmp(def->argv[0], expected_run) == 0);
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    /* down file disables auto-restart */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run", "#!/bin/sh\nexec /usr/bin/myapp\n");
    write_file(svc_dir, "down", "");
    def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(def->restart == 0);
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    /* env/ variables are loaded */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run", "#!/bin/sh\nexec /usr/bin/myapp\n");
    write_env_var(svc_dir, "FOO", "bar");
    write_env_var(svc_dir, "PORT", "8080");
    def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(env_has(def->env, "FOO=bar"));
    CHECK(env_has(def->env, "PORT=8080"));
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    /* missing run script returns NULL */
    svc_dir = make_tmpdir();
    def = service_parse_runit(svc_dir);
    CHECK(def == NULL);
    rm_rf(svc_dir); free(svc_dir);

    /* env/ with newline-terminated value is stripped */
    svc_dir = make_tmpdir();
    write_file(svc_dir, "run", "#!/bin/sh\nexec /usr/bin/myapp\n");
    write_env_var(svc_dir, "KEY", "value\n");
    def = service_parse_runit(svc_dir);
    CHECK(def != NULL);
    CHECK(env_has(def->env, "KEY=value"));
    service_free(def);
    rm_rf(svc_dir); free(svc_dir);

    TEST_DONE();
}
