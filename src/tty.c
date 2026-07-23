#include "tty.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int tty_setup(const char *tty) {
    char path[256];

    if (!tty || !tty[0])
        return -1;

    if (tty[0] == '/')
        snprintf(path, sizeof(path), "%s", tty);
    else
        snprintf(path, sizeof(path), "/dev/%s", tty);

    setsid();

    int fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0)
        return -1;

    ioctl(fd, TIOCSCTTY, 1);

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO)
        close(fd);

    return 0;
}
