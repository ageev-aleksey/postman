#include <pthread.h>
#include "context.h"

int init_context() {
    FD_ZERO(&app_context.master);
    FD_ZERO(&app_context.read_fds);
    FD_ZERO(&app_context.write_fds);

    return 0;
}

int add_socket(int socket) {
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 50000;

    FD_SET(socket, &app_context.master);

    if (socket > 0) {
        app_context.fdmax = socket;
    }

    return 0;
}