#include "event_loop.h"
#include <stdio.h>

#define __USE_MISC
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PRINT_ERROR_EXIT(res) \
do { \
    if ((res) == -1) { \
        perror("bind"); \
        return -1; \
    } \
} while(0);



void accept_handler(event_loop *loop, int socket, async_error error) {
    if (error.is != NO_ERROR) {
        printf("error");
    }
    printf("accept %d", socket);
}

int main() {
    event_loop loop;
    el_init(&loop);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, O_NONBLOCK);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htonl(8080);
    int res = inet_aton("127.0.0.1", &addr.sin_addr);
    PRINT_ERROR_EXIT(res);
    listen(sock, 0);
    res = bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    PRINT_ERROR_EXIT(res);
    el_async_accept(&loop, sock, accept_handler);
    el_open(&loop, true);

    return 0;
}
