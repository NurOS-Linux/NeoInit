// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "mount.h"
#include "log.h"

#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

static const mount_entry_t essential[] = {
    { "proc",     "/proc",     "proc",     MS_NOSUID | MS_NODEV | MS_NOEXEC },
    { "sysfs",    "/sys",      "sysfs",    MS_NOSUID | MS_NODEV | MS_NOEXEC },
    { "devtmpfs", "/dev",      "devtmpfs", MS_NOSUID | MS_NOEXEC },
    { "devpts",   "/dev/pts",  "devpts",   MS_NOSUID | MS_NOEXEC },
    { "tmpfs",    "/dev/shm",  "tmpfs",    MS_NOSUID | MS_NODEV },
    { "tmpfs",    "/run",      "tmpfs",    MS_NOSUID | MS_NODEV },
};

static const size_t essential_count = sizeof(essential) / sizeof(essential[0]);

void mount_essential(void) {
    for (size_t i = 0; i < essential_count; i++) {
        const mount_entry_t *m = &essential[i];
        
        /* Ensure target exists */
        mkdir(m->target, 0755);

        if (mount(m->source, m->target, m->fstype, m->flags, NULL) < 0) {
            if (errno != EBUSY && errno != EPERM) {
                log_err("mount %s -> %s: %s", m->source, m->target, strerror(errno));
            }
        } else {
            log_info("mounted %s on %s", m->source, m->target);
        }
    }
}

void unmount_all(void) {
    /* Unmount in reverse order */
    for (int i = (int)essential_count - 1; i >= 0; i--) {
        const mount_entry_t *m = &essential[i];
        if (umount(m->target) < 0) {
            if (errno != EPERM && errno != EINVAL) {
                log_warn("umount %s: %s", m->target, strerror(errno));
            }
        }
    }
}

const mount_entry_t *mount_table_get(void) {
    return essential;
}

size_t mount_table_count(void) {
    return essential_count;
}
