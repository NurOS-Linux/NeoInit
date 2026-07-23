#include "framework.h"
#include "launcher.h"
#include "order.h"
#include "registry.h"
#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void write_def(const char *dir, const char *file, const char *content) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, file);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fputs(content, f);
    fclose(f);
}

static int find_idx(service_def_t **defs, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (defs[i]->name && strcmp(defs[i]->name, name) == 0)
            return i;
    }
    return -1;
}

int main(void) {
    char dir[] = "/tmp/raesir_it_XXXXXX";
    CHECK(mkdtemp(dir) != NULL);

    write_def(dir, "base.toml",
        "name = \"base\"\n"
        "exec = \"/usr/bin/true\"\n"
        "type = \"oneshot\"\n");
    write_def(dir, "app.toml",
        "name = \"app\"\n"
        "exec = \"/usr/bin/true\"\n"
        "type = \"oneshot\"\n"
        "requires = [\"base\"]\n"
        "restart_delay_ms = 50\n"
        "restart_max = 3\n"
        "memory_max = \"64M\"\n"
        "cpu_weight = 50\n");
    write_def(dir, "compat.service",
        "[Unit]\n"
        "Description=Compat unit\n"
        "After=app\n"
        "\n"
        "[Service]\n"
        "ExecStart=/usr/bin/true\n"
        "Type=oneshot\n");

    service_def_t **defs = NULL;
    int count = service_load_dir(dir, &defs);
    CHECK(count == 3);

    int base_i = find_idx(defs, count, "base");
    int app_i = find_idx(defs, count, "app");
    int compat_i = find_idx(defs, count, "compat");
    CHECK(base_i >= 0);
    CHECK(app_i >= 0);
    CHECK(compat_i >= 0);

    service_def_t *app = defs[app_i];
    CHECK(app->restart_delay_ms == 50);
    CHECK(app->restart_max == 3);
    CHECK(app->memory_max != NULL && strcmp(app->memory_max, "64M") == 0);
    CHECK(app->cpu_weight == 50);

    int levels[3] = { 0 };
    int n = order_levels(defs, count, levels);
    CHECK(n == 3);
    CHECK(levels[base_i] == 0);
    CHECK(levels[app_i] == 1);
    CHECK(levels[compat_i] == 2);

    registry_init();
    for (int i = 0; i < count; i++)
        CHECK(registry_add(defs[i]) == 0);
    CHECK(registry_count() == 3);

    service_entry_t *ent = registry_find("app");
    CHECK(ent != NULL);
    CHECK(ent->enabled == 1);
    CHECK(ent->stop_requested == 0);
    CHECK(ent->restart_count == 0);

    pid_t pid = service_launch(defs[base_i]);
    CHECK(pid > 0);

    int status = 0;
    CHECK(waitpid(pid, &status, 0) == pid);
    CHECK(WIFEXITED(status));
    CHECK(WEXITSTATUS(status) == 0);

    service_entry_t *found = registry_find_by_pid(-1);
    CHECK(found == NULL || found->pid == -1);

    char path[512];
    const char *names[] = { "base.toml", "app.toml", "compat.service" };
    for (int i = 0; i < 3; i++) {
        snprintf(path, sizeof(path), "%s/%s", dir, names[i]);
        unlink(path);
    }
    rmdir(dir);

    for (int i = 0; i < count; i++)
        service_free(defs[i]);
    free(defs);

    TEST_DONE();
}
