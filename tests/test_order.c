#include "framework.h"
#include "order.h"
#include "service.h"

#include <string.h>

static service_def_t *mk(const char *name) {
    service_def_t *d = service_alloc();
    d->name = strdup(name);
    return d;
}

int main(void) {
    printf("test_order\n");

    service_def_t *a = mk("a");
    service_def_t *b = mk("b");
    service_def_t *c = mk("c");
    service_def_t *d = mk("d");

    b->requires = service_strv_append(NULL, "a");
    c->after    = service_strv_append(NULL, "b");
    d->wants    = service_strv_append(NULL, "a");

    service_def_t *defs[4] = { c, a, d, b };
    int levels[4] = { 0 };

    int n = order_levels(defs, 4, levels);

    CHECK(n == 3);
    CHECK(levels[1] == 0);
    CHECK(levels[3] == 1);
    CHECK(levels[2] == 1);
    CHECK(levels[0] == 2);

    service_def_t *solo = mk("solo");
    service_def_t *one[1] = { solo };
    int slv[1] = { 0 };
    CHECK(order_levels(one, 1, slv) == 1);
    CHECK(slv[0] == 0);

    service_def_t *x = mk("x");
    service_def_t *y = mk("y");
    x->after = service_strv_append(NULL, "y");
    y->after = service_strv_append(NULL, "x");

    service_def_t *cyc[2] = { x, y };
    int clv[2] = { 0 };

    CHECK(order_levels(cyc, 2, clv) == 1);
    CHECK(clv[0] == 0);
    CHECK(clv[1] == 0);

    service_def_t *ghost = mk("ghost");
    ghost->after = service_strv_append(NULL, "nonexistent");
    service_def_t *g[1] = { ghost };
    int glv[1] = { 0 };
    CHECK(order_levels(g, 1, glv) == 1);
    CHECK(glv[0] == 0);

    service_free(a);
    service_free(b);
    service_free(c);
    service_free(d);
    service_free(solo);
    service_free(x);
    service_free(y);
    service_free(ghost);

    TEST_DONE();
}
