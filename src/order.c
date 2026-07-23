#include "order.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

static int find_def(service_def_t **defs, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (defs[i]->name && strcmp(defs[i]->name, name) == 0)
            return i;
    }
    return -1;
}

static void mark_deps(service_def_t **defs, int count, int i, char **names,
                      unsigned char *m) {
    if (!names) return;
    for (int k = 0; names[k]; k++) {
        int j = find_def(defs, count, names[k]);
        if (j >= 0 && j != i)
            m[(size_t)i * (size_t)count + (size_t)j] = 1;
    }
}

int order_levels(service_def_t **defs, int count, int *levels) {
    if (count <= 0) return 0;

    unsigned char *m = calloc((size_t)count * (size_t)count, 1);
    int *indeg = calloc((size_t)count, sizeof(int));
    if (!m || !indeg) {
        free(m);
        free(indeg);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        mark_deps(defs, count, i, defs[i]->after, m);
        mark_deps(defs, count, i, defs[i]->requires, m);
        mark_deps(defs, count, i, defs[i]->wants, m);
    }

    for (int i = 0; i < count; i++) {
        levels[i] = -1;
        for (int j = 0; j < count; j++) {
            if (m[(size_t)i * (size_t)count + (size_t)j])
                indeg[i]++;
        }
    }

    int done = 0;
    int level = 0;
    int max_level = 0;

    while (done < count) {
        int marked = 0;

        for (int i = 0; i < count; i++) {
            if (levels[i] == -1 && indeg[i] == 0) {
                levels[i] = level;
                marked++;
            }
        }

        if (marked == 0) {
            for (int i = 0; i < count; i++) {
                if (levels[i] == -1) {
                    levels[i] = level;
                    done++;
                }
            }
            log_warn("dependency cycle detected, starting remaining services together");
            max_level = level;
            break;
        }

        for (int i = 0; i < count; i++) {
            if (levels[i] != level) continue;
            for (int k = 0; k < count; k++) {
                if (m[(size_t)k * (size_t)count + (size_t)i])
                    indeg[k]--;
            }
        }

        done += marked;
        max_level = level;
        level++;
    }

    free(m);
    free(indeg);
    return max_level + 1;
}
