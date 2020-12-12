//
// Created by nrx on 05.12.2020.
//

#ifndef SERVER_USER_CONTEXT_H
#define SERVER_USER_CONTEXT_H
#include "sys/queue.h"
#include "smtp/state.h"
#include "vector_structures.h"
#include "maildir/maildir.h"
#include "event_loop/event_loop.h"
#include "server/users_list.h"
#include "util.h"
#include <pthread.h>
#include <libconfig.h>

#define POSTMAN_VERSION_MAJOR 0
#define POSTMAN_VERSION_MINOR 1



struct {
    maildir md;
    char *ip;
    int16_t  port;
    char *log_file_path;
    char *self_server_name;
    struct users_list users;
    char *hello_msg;
    size_t hello_msg_size;
    event_loop *loop;
} server_config;

struct pair {
    char *buffer;
    smtp_status status;
};

bool server_config_init(const char *path);
void server_config_free();


void user_disconnected(int sock);
void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error);
void handler_write(event_loop *el, int socket, char* buffer, int size, int writing, client_status status, err_t error);
void handler_read(event_loop *el, int socket, char *buffer, int size, client_status status, err_t error);
void handler_timer(event_loop*, int socket, struct timer_event_entry *descriptor);
struct pair handler_smtp(user_context *user, char *message);
#endif //SERVER_USER_CONTEXT_H
