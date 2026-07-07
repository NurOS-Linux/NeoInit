// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Raesir/raesir

#include "framework.h"
#include "client.h"
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define TEST_SOCK "/tmp/servctl_test.sock"

static int ready_pipe[2];

static void *mock_server(void *arg) {
    const char *path = (const char *)arg;
    int lfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    unlink(path);
    bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(lfd, 4);

    write(ready_pipe[1], "", 1);
    close(ready_pipe[1]);

    int cfd;
    while ((cfd = accept(lfd, NULL, NULL)) >= 0) {
        char buf[256];
        ssize_t n = read(cfd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            if (strcmp(buf, "status") == 0)
                write(cfd, "raesir: active\n", 16);
            else if (strcmp(buf, "list") == 0)
                write(cfd, "NAME                 PID      STATUS\n", 37);
            else
                write(cfd, "err: unknown command\n", 21);
        }
        close(cfd);
    }
    close(lfd);
    return NULL;
}

int main(void) {
    pipe(ready_pipe);

    pthread_t tid;
    pthread_create(&tid, NULL, mock_server, (void *)TEST_SOCK);

    char dummy;
    read(ready_pipe[0], &dummy, 1);
    close(ready_pipe[0]);

    char buf[1024];

    CHECK(servctl_run_command(TEST_SOCK, "status", buf, sizeof(buf)) == 0);
    CHECK(strcmp(buf, "raesir: active\n") == 0);

    CHECK(servctl_run_command(TEST_SOCK, "list", buf, sizeof(buf)) == 0);
    CHECK(strcmp(buf, "NAME                 PID      STATUS\n") == 0);

    CHECK(servctl_run_command(TEST_SOCK, "bogus", buf, sizeof(buf)) == 0);
    CHECK(strcmp(buf, "err: unknown command\n") == 0);

    pthread_cancel(tid);
    pthread_join(tid, NULL);
    unlink(TEST_SOCK);

    TEST_DONE();
}
