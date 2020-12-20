//
// Created by nrx on 05.12.2020.
//

#ifndef SERVER_USER_CONTEXT_H
#define SERVER_USER_CONTEXT_H


#include "server/config.h"
#include "util.h"
#include <pthread.h>
#include <libconfig.h>
#include "event_loop/event_loop.h"
#include "server/users_list.h"


#define POSTMAN_TIMEOUT_OF_TIMER 15


void user_disconnected(int sock);
void handler_accept(event_loop *el, int acceptor, int client_socket, struct sockaddr_in client_addr, err_t error);
void handler_write(event_loop *el, int socket, char* buffer, int size, int writing, client_status status, err_t error);
void handler_read(event_loop *el, int socket, char *buffer, int size, client_status status, err_t error);
void handler_timer(event_loop*, int socket, struct timer_event_entry *descriptor);
void handler_close_socket(event_loop*, int sock, err_t *err);
struct pair handler_smtp(user_context *user, char *message);
#endif //SERVER_USER_CONTEXT_H
