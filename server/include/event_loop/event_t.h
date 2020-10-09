//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_EVENT_T_H
#define SERVER_EVENT_T_H

#include "error_t.h"

typedef enum _event_type {
    SOCK_ACCEPT,
    SOCK_READ,
    SOCK_WRITE,
    SOCK_TIMER,
    NONE,
} event_type;

struct _event_loop;
typedef  void (*sock_accept_handler)(struct _event_loop*, int acceptro, int client_socket, struct sockaddr_in client_addr, error_t);
typedef  void (*sock_read_handler)(struct _event_loop* loop, int socket, char *buffer, int size, error_t);
typedef  void (*sock_write_handler)(struct _event_loop*, int socket, int size, error_t);
typedef void (*sock_timer_handler)(struct _event_loop*, int socket, unsigned int time, error_t);
typedef void (*buff_deleter)(void *buffer, int bsize);

typedef struct _sock_event {
    event_type type;
    int socket;
} sock_event;

typedef struct _event_t {
    sock_event  event;
} event_t;

typedef struct _event_sock_accept {
    sock_event event;
    sock_accept_handler handler;
} event_sock_accept;

typedef struct _event_sock_read {
    sock_event event;
    sock_read_handler handler;
    char *buffer;
    int size;
    int offset;
} event_sock_read;

typedef struct _event_sock_write {
    sock_event event;
    sock_write_handler handler;
    int offset; // возможна не полная отправка данных за раз. Это смещенеи в буфере
    int size;
    char *buffer;
    buff_deleter deleter;

} event_sock_write;

#endif //SERVER_EVENT_T_H
