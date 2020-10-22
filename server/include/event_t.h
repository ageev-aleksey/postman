//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_EVENT_T_H
#define SERVER_EVENT_T_H

#include "error_t.h"

#include <netinet/in.h>

typedef enum _event_type {
    SOCK_ACCEPT,
    SOCK_READ,
    SOCK_WRITE,
  // TODO (ageev)  SOCK_TIMER,
   // SOCK_DISCONNECT,
} event_type;

typedef enum _client_status {
    CONNECTED,
    DISCONNECTED
} client_status;



struct _event_loop;
typedef void (*sock_accept_handler)(struct _event_loop*, int acceptor, int client_socket, struct sockaddr_in client_addr, error_t);
typedef void (*sock_read_handler)(struct _event_loop* loop, int socket, char *buffer, int size, client_status, error_t);
typedef void (*sock_write_handler)(struct _event_loop*, int socket, char* buffer, int size, int writing, client_status,  error_t);
//typedef void (*sock_timer_handler)(struct _event_loop*, int socket, unsigned int time, error_t);
//typedef void (*buff_deleter)(void *buffer, int bsize);

typedef struct _sock_event {
    event_type type;
    int socket;
    error_t error;
} sock_event;

typedef struct _event_t {
    sock_event  event;
} event_t;

typedef struct _event_sock_accept {
    sock_event event;
    sock_accept_handler handler;
    int client_socket;
    struct sockaddr_in client_addr;
} event_sock_accept;

typedef struct _event_sock_read {
    sock_event event;
    sock_read_handler handler;
    char *buffer;
    int size;
    int offset;
    client_status  status;
} event_sock_read;

typedef struct _event_sock_write {
    sock_event event;
    sock_write_handler handler;
    int offset; // возможна не полная отправка данных за раз. Это смещенеи в буфере
    int size;
    char *buffer;
    client_status status;
} event_sock_write;

//typedef struct _event_sock_disconnect {
//    sock_event event;
//    sock_disconnect_handler handler;
//} event_sock_disconnect;

#endif //SERVER_EVENT_T_H
