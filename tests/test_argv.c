// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
// https://github.com/NurOS-Linux/neoinit

#include "framework.h"
#include "service.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    char **argv;

    /* simple */
    argv = service_parse_argv("/usr/bin/echo hello world");
    CHECK(argv != NULL);
    CHECK(strcmp(argv[0], "/usr/bin/echo") == 0);
    CHECK(strcmp(argv[1], "hello") == 0);
    CHECK(strcmp(argv[2], "world") == 0);
    CHECK(argv[3] == NULL);
    for (int i = 0; argv[i]; i++) free(argv[i]);
    free(argv);

    /* double quotes */
    argv = service_parse_argv("/bin/cmd \"hello world\" arg2");
    CHECK(argv != NULL);
    CHECK(strcmp(argv[0], "/bin/cmd") == 0);
    CHECK(strcmp(argv[1], "hello world") == 0);
    CHECK(strcmp(argv[2], "arg2") == 0);
    CHECK(argv[3] == NULL);
    for (int i = 0; argv[i]; i++) free(argv[i]);
    free(argv);

    /* single quotes */
    argv = service_parse_argv("/bin/cmd 'hello world'");
    CHECK(argv != NULL);
    CHECK(strcmp(argv[1], "hello world") == 0);
    for (int i = 0; argv[i]; i++) free(argv[i]);
    free(argv);

    /* escaped space */
    argv = service_parse_argv("/bin/cmd hello\\ world");
    CHECK(argv != NULL);
    CHECK(strcmp(argv[1], "hello world") == 0);
    for (int i = 0; argv[i]; i++) free(argv[i]);
    free(argv);

    /* single token */
    argv = service_parse_argv("/sbin/init");
    CHECK(argv != NULL);
    CHECK(strcmp(argv[0], "/sbin/init") == 0);
    CHECK(argv[1] == NULL);
    for (int i = 0; argv[i]; i++) free(argv[i]);
    free(argv);

    TEST_DONE();
}
