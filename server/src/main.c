#include "event_loop/event_loop.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void read_handler(struct _event_loop* loop, int socket, char *buffer, int size, error_t er);
void write_handler(struct _event_loop* loop, int socket, char* buffer, int size, int writing,  error_t error) {
    printf(" -- Send -> %d \n", writing);
    free(buffer);
    buffer = s_malloc(500, NULL);
    el_async_read(loop, socket, buffer, 500, read_handler, NULL);

}

void read_handler(struct _event_loop* loop, int socket, char *buffer, int size, error_t er) {
    if (size == 0) {
        // client disconnect
        close(socket);
        free(buffer);
    } else {
        printf(" -- %s\n", buffer);
        char *new_buffer = s_malloc(500, NULL);
        el_async_write(loop, socket, buffer, 500, write_handler, NULL);
    }
}



void accept_handler(struct _event_loop* loop, int acceptor, int client_socket, struct sockaddr_in client_addr, error_t error) {
    if (error.error) {
        printf("error");
    }
    char ip[12] = {'\0'};
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, 12);
    int port = ntohs(client_addr.sin_port);
    printf("accept connection from: %s:%d \n", ip, port);
    char *buffer = (char*) malloc(sizeof(char)*500);
    el_async_read(loop, client_socket, buffer, 500, read_handler, NULL);
}

int main() {
//    int i = hcreate(10);
//
    error_t error;
    event_loop *loop = el_init(&error);
    if (error.error) {
        return -1;
    }
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
    el_async_accept(loop, sock, accept_handler, NULL);
    el_open(loop, ONE_THREAD, &error);

    return 0;
}
