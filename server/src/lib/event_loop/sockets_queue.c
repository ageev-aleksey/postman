//
// Created by nrx on 07.10.2020.
//

#include "event_loop/sockets_queue.h"
#include "event_loop/error_t.h"
#include "util.h"
#include <stdlib.h>


const char SQ_HANDLER_IS_NULL[] = "handler is null";
const char SQ_INVALID_SOCKET_VALUE[] = "Invalid socket";
const char SQ_SOCKET_EXISTS[] = "Handler for socket already exists";
const char SQ_SOCKET_NOT_FOUND[] = "Socket not found";
const char SQ_QUEUE_IS_NULL[] = "pointer of queue is null";





sockets_queue* sq_init(error_t *error) {
    sockets_queue *ptr = malloc(sizeof(sockets_queue));

    TAILQ_INIT(ptr);
    return ptr;
}

bool sq_add(sockets_queue *queue, int socket, sock_accept_handler handler, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_QUEUE_IS_NULL;
        }
        return NULL;
    }

    if (handler == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_HANDLER_IS_NULL;
        }
        return false;
    }

    if (socket < 0) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_INVALID_SOCKET_VALUE;
        }
        return false;
    }

    socket_entry *el = NULL;
    TAILQ_FOREACH(el, queue, entries) {
        if (el->socket == socket) {
            break;
        }
    }
    if (el != NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_SOCKET_EXISTS;
        }
        return false;
    }

    error_t er;
    el = s_malloc(sizeof(socket_entry), &er);
    if (er.is_error) {
        *error = er;
        return false;
    }
    el->socket = socket;
    el->handler = handler;
    TAILQ_INSERT_TAIL(queue, el, entries);
    if (error != NULL) {
        error->is_error = false;
        error->message = ERROR_SUCCESS;
    }
    return true;
}

sock_accept_handler sq_get(sockets_queue *queue, int socket, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_QUEUE_IS_NULL;
        }
        return NULL;
    }

    socket_entry *el = NULL;
    TAILQ_FOREACH(el, queue, entries) {
        if (el->socket == socket) {
            break;
        }
    }
    if (el == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = SQ_SOCKET_NOT_FOUND;
        }
        return NULL;
    }
    if (error != NULL) {
        error->is_error = false;
        error->message = ERROR_SUCCESS;
    }
    return el->handler;
}

void sq_free(sockets_queue* queue) {
    if (queue == NULL) {
        return;
    }
    socket_entry *el = NULL;
    while (!TAILQ_EMPTY(queue)) {
        el = TAILQ_FIRST(queue);
        TAILQ_REMOVE(queue, el, entries);
        free(el);
    }
    free(queue);
}
