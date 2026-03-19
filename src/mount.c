// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "mount.h"
#include "log.h"

#include <errno.h>
#include <string.h>
#include <sys/mount.h>

static const mount_entry_t table[] = {
    { "proc",     "/proc",     "proc",     MS_NOSUID | MS_NOEXEC | MS_NODEV,      NULL              },
    { "sysfs",    "/sys",      "sysfs",    MS_NOSUID | MS_NOEXEC | MS_NODEV,      NULL              },
    { "devtmpfs", "/dev",      "devtmpfs", MS_NOSUID | MS_STRICTATIME,             "mode=0755"       },
    { "devpts",   "/dev/pts",  "devpts",   MS_NOSUID | MS_NOEXEC,                 "mode=0620,gid=5" },
    { "tmpfs",    "/dev/shm",  "tmpfs",    MS_NOSUID | MS_NODEV,                  NULL              },
    { "tmpfs",    "/run",      "tmpfs",    MS_NOSUID | MS_NODEV | MS_STRICTATIME,  "mode=0755"       },
};

const mount_entry_t *mount_table_get(void) {
    return table;
}

size_t mount_table_count(void) {
    return sizeof(table) / sizeof(table[0]);
}

int mount_essential(void) {
    size_t n = mount_table_count();

    for (size_t i = 0; i < n; i++) {
        const mount_entry_t *e = &table[i];
        if (mount(e->source, e->target, e->fstype, e->flags, e->data) < 0) {
            if (errno == EBUSY) {
                log_warn("already mounted: %s", e->target);
                continue;
            }
            log_err("mount %s -> %s: %s", e->source, e->target, strerror(errno));
            return -1;
        }
        log_info("mounted %s", e->target);
    }

    return 0;
}

int unmount_all(void) {
    size_t n = mount_table_count();
    int rc = 0;

    for (size_t i = n; i-- > 0;) {
        const mount_entry_t *e = &table[i];
        if (umount2(e->target, MNT_DETACH) < 0) {
            log_warn("umount %s: %s", e->target, strerror(errno));
            rc = -1;
        } else {
            log_info("unmounted %s", e->target);
        }
    }

    return rc;
}
