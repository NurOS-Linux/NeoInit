#include "cgroup.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CG_BASE "/sys/fs/cgroup/raesir"

static int write_string(const char *path, const char *val) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) return -1;

    ssize_t n = write(fd, val, strlen(val));
    close(fd);
    return (n < 0) ? -1 : 0;
}

int cgroup_setup(const service_def_t *def, pid_t pid) {
    char dir[512];
    char path[600];
    char val[64];

    if (mkdir(CG_BASE, 0755) < 0 && errno != EEXIST)
        return -1;

    snprintf(dir, sizeof(dir), CG_BASE "/%s", def->name);
    if (mkdir(dir, 0755) < 0 && errno != EEXIST)
        return -1;

    if (def->memory_max) {
        snprintf(path, sizeof(path), "%s/memory.max", dir);
        if (write_string(path, def->memory_max) < 0)
            log_warn("%s: cannot set memory.max to %s", def->name, def->memory_max);
    }

    if (def->cpu_weight > 0) {
        snprintf(path, sizeof(path), "%s/cpu.weight", dir);
        snprintf(val, sizeof(val), "%d", def->cpu_weight);
        if (write_string(path, val) < 0)
            log_warn("%s: cannot set cpu.weight to %d", def->name, def->cpu_weight);
    }

    snprintf(path, sizeof(path), "%s/cgroup.procs", dir);
    snprintf(val, sizeof(val), "%d", (int)pid);
    if (write_string(path, val) < 0) {
        rmdir(dir);
        return -1;
    }

    return 0;
}

void cgroup_teardown(const char *name) {
    char dir[512];

    snprintf(dir, sizeof(dir), CG_BASE "/%s", name);
    rmdir(dir);
}
