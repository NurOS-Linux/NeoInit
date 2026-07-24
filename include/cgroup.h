#pragma once

#include "service.h"
#include <sys/types.h>

int  cgroup_setup(const service_def_t *def, pid_t pid);
void cgroup_teardown(const char *name);
