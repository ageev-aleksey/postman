//
// Created by nrx on 08.10.2020.
//

#include "event_loop/registered_events_queue.h"
#include "event_loop/events_queue.h"
#include "util.h"

const char REQ_QUEUE_IS_NULL[] = "pointer of registered_events_queue is null";
const char REQ_SOCK_NOTFOUND[] = "Not found registered events for socket in registered_events_queue";
const char REQ_INVALID_SOCKET_VALUE[] = "Invalid socket";
const char REQ_EVENT_IS_NULL[] = "Event pointer is null";

#define CHECK(q, error) \
do {             \
    if ((q) == NULL) { \
        (err).is_error = true; \
        (err).message =   REQ_QUEUE_IS_NULL;                  \
    }\
} while(0)

#define CHECK_AND_RETURN(q)     \
do {                            \
    error_t err;                \
    CHECK(queue, err);          \
    if (err.is_error) {         \
        if (error != NULL) {    \
            *error = err;       \
        }                       \
        return NULL;            \
    }                           \
} while(0)

registered_events_entry* _req_get_sock(registered_events_queue *queue, int sock) {

    registered_events_entry* el = NULL;
    TAILQ_FOREACH(el, queue, entries) {
        if (el->sock_events.sock == sock) {
            break;
        }
    };

    return el;
}

registered_events_queue* req_init(error_t *error) {
    error_t err;
    registered_events_queue* ptr = s_malloc(sizeof(registered_events_queue), &err);
    if (err.is_error) {
        if (error != NULL) {
            *error = err;
        }
        return NULL;
    }
    TAILQ_INIT(ptr);
    ERROR_SUCCESS(error);
    return ptr;
}

bool req_add_accept(registered_events_queue* queue, int socket, event_sock_accept *event, error_t *error) {
    CHECK_AND_RETURN(queue);
    if (socket < 0) {
        if (error != NULL) {
            error->is_error = true;
            error->message = REQ_INVALID_SOCKET_VALUE;
        }
        return false;
    }
    if (event == NULL) {
        if (error != NULL) {
            error->is_error = true;
            error->message = REQ_EVENT_IS_NULL;
        }
        return false;
    }

    registered_events_entry *ptr = _req_get_sock(queue, socket);

    if (ptr == NULL) {
        error_t err;
        ptr = s_malloc(sizeof(registered_events_entry), &err);

        if (err.is_error) {
            if(error != NULL) {
                *error = err;
            }
            return NULL;
        }

        ptr->sock_events.sock = socket;
        ptr->sock_events.events = eq_init(err);
    }


}