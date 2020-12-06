//
// Created by nrx on 23.10.2020.
//

#include "global_context.h"
#include "log/context.h"
#include "event_loop/event_loop.h"
#include <stdio.h>
#include <unistd.h>
#define ERROR (-1)
#define OK 0
#define BUFFER_READ 5

int main() {
    int status = OK;
    int master_socket = ERROR;
    LOG_INIT();
    smtp_lib_init();

    LOG_INFO("%s", "server start init...");
    if (!server_config_init("./config.cfg")) {
        status = ERROR;
        return ERROR;
    }

    err_t error;
    event_loop *el = el_init(&error);
    if (el == NULL) {
        status = ERROR;
        LOG_ERROR("event_loop init: %s", error.message);
        goto exit;
    }
    master_socket = make_server_socket(server_config.ip,
                                       server_config.port,
                                       &error);
    if (master_socket == ERROR) {
        status = ERROR;
        LOG_ERROR("create server socket: %s", error.message);
        goto exit;
    }
    el_async_accept(el, master_socket, handler_accept, &error);
    if (error.error) {
        status = ERROR;
        LOG_ERROR("el_async_accept: %s", error.message);
        goto exit;
    }
    LOG_INFO("%s", "Server init");
    el_open(el, ONE_THREAD, &error);


exit:
    if (master_socket != ERROR) {
        close(master_socket);
    }
    el_close(el);
    server_config_free();
    smtp_lib_free();
    LOG_FREE();
    return status;
}



//#define _GNU_SOURCE
//
//#include "event_loop/event_loop.h"
//#include "util.h"
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <fcntl.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <string.h>
//#include <search.h>
//
//#define PRINT_ERROR_EXIT(res) \
//do { \
//    if ((res) == -1) { \
//        perror("bind"); \
//        return -1; \
//    } \
//} while(0);
//
//void read_handler(struct _event_loop* loop, int socket, char *buffer, int size, client_status status, err_t er);
//void write_handler(struct _event_loop* loop, int socket, char* buffer, int size, int writing,  err_t error) {
//    printf(" -- Send -> %d \n", writing);
//    free(buffer);
//    buffer = s_malloc(500, NULL);
//    el_async_read(loop, socket, buffer, 500, read_handler, NULL);
//
//}
//
//void read_handler(struct _event_loop* loop, int socket, char *buffer, int size, client_status status, err_t er) {
//    if (status == DISCONNECTED) {
//        // client disconnect
//        close(socket);
//        free(buffer);
//    } else {
//        printf(" -- %s\n", buffer);
//        char *new_buffer = s_malloc(500, NULL);
//        //el_async_write(loop, socket, buffer, 500, write_handler, NULL);
//    }
//}
//
//
//
//void accept_handler(struct _event_loop* loop, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error) {
//    if (error.error) {
//        printf("error");
//    }
//    char ip[12] = {'\0'};
//    inet_ntop(AF_INET, &client_addr.sin_addr, ip, 12);
//    int port = ntohs(client_addr.sin_port);
//    printf("accept connection from: %s:%d \n", ip, port);
//    char *buffer = (char*) malloc(sizeof(char)*500);
//    el_async_read(loop, client_socket, buffer, 500, read_handler, NULL);
//}
//
//int main() {
////    int i = hcreate(10);
////
//    err_t error;
//    event_loop *loop = el_init(&error);
//    if (error.error) {
//        printf("%s", error.message);
//        return -1;
//    }
//    int sock = make_server_socket("127.0.0.1", 8080, &error);
//    if (error.error) {
//        printf("%s", error.message);
//        return -1;
//    }
//    el_async_accept(loop, sock, accept_handler, NULL);
//    el_open(loop, ONE_THREAD, &error);
//    return 0;
//}
