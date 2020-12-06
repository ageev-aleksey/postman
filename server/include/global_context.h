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
#include "util.h"
#include <libconfig.h>

#define POSTMAN_VERSION_MAJOR 0
#define POSTMAN_VERSION_MINOR 1

typedef struct user_context {
    smtp_state smtp;
    vector_char  buffer;
    int socket;
    client_addr addr;
} user_context;

typedef struct user_context_entry {
    user_context context;
    TAILQ_ENTRY(user_context_entry) entries;
} user_context_entry;

TAILQ_HEAD(users_list, user_context_entry);
typedef struct users_list users_list;

struct {
    maildir md;
    char *ip;
    int16_t  port;
    char *log_file_path;
    char *self_server_name;
    struct users_list users;
    char *hello_msg;
    size_t hello_msg_size;
} global_config_server;

bool server_config_init(const char *path);
void server_config_free();

bool user_init(user_context *context, struct sockaddr_in *addr, int socket);
user_context_entry* users_find(users_list *users, int sock);
void users_delete(users_list *users, int sock);

void user_disconnected(int sock);

void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error);
void handler_write(event_loop *el, int socket, char* buffer, int size, int writing, client_status status, err_t error);
void handler_read(event_loop *el, int socket, char *buffer, int size, client_status status, err_t error);
void handler_timer(event_loop*, int socket, struct timer_event_entry *descriptor);
#endif //SERVER_USER_CONTEXT_H
