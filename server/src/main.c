#include "event_loop/event_loop.h"
#include <stdio.h>
#include <stdlib.h>

#define __USE_MISC
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <search.h>

#define PRINT_ERROR_EXIT(res) \
do { \
    if ((res) == -1) { \
        perror("bind"); \
        return -1; \
    } \
} while(0);

void read_handler(event_loop* loop, int socket, char *buffer, int size, async_error error) {
    printf("%s", buffer);
    free(buffer);
   // el_async_read(loop, client_socket, buffer, 500, read_handler);
}



void accept_handler(event_loop *loop, int master_socket, int client_socket, struct sockaddr_in client_addr, async_error error) {
    if (error.is != NO_ERROR) {
        printf("error");
    }
    char ip[12] = {'\0'};
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, 12);
    int port = ntohs(client_addr.sin_port);
    printf("accept connection from: %s:%d", ip, port);
    char *buffer = (char*) malloc(sizeof(char)*500);
    el_async_read(loop, client_socket, buffer, 500, read_handler);
}

int main() {
//    int i = hcreate(10);
//
    event_loop loop;
    el_init(&loop);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, O_NONBLOCK);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    int res = inet_aton("127.0.0.1", &addr.sin_addr);
    PRINT_ERROR_EXIT(res);
    res = bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    PRINT_ERROR_EXIT(res);
    res = listen(sock, 1);
    PRINT_ERROR_EXIT(res);
    el_async_accept(&loop, sock, accept_handler);
    el_open(&loop, true);


    el_run(&loop);



    return 0;
}
