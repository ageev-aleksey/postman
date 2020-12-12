#include "stdlib.h"

typedef struct context {
    fd_set master;
    fd_set read_fds;
    fd_set write_fds;
    int fdmax;
} context;

static context app_context;
