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
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return NULL;
    }
    TAILQ_INIT(ptr);
    ERROR_SUCCESS(error);
    return ptr;
}

bool pr_req_init_before_add(registered_events_queue* queue, int socket, registered_events_entry **entry, error_t *error) {
    CHECK_AND_RETURN(queue, error, REQ_QUEUE_IS_NULL);
    if (socket < 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = REQ_INVALID_SOCKET_VALUE;
        }
        return false;
    }

    registered_events_entry *ptr = _req_get_sock(queue, socket);
    error_t err;
    if (ptr == NULL) {

        ptr = s_malloc(sizeof(registered_events_entry), &err);

        if (err.error) {
            if(error != NULL) {
                *error = err;
            }
            return NULL;
        }

        ptr->sock_events.sock = socket;
        ptr->sock_events.events = eq_init(&err);
        if (err.error) {
            if (error != NULL) {
                *error = err;
            }
            return false;
        }
    }

    *entry = ptr;
    return true;
}

bool req_push_accept(registered_events_queue* queue, int socket, event_sock_accept *event, error_t *error) {
    registered_events_entry *ptr = NULL;
    error_t err;
    ERROR_SUCCESS(&err);
    pr_req_init_before_add(queue, socket, &ptr, &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    eq_push_accept(ptr->sock_events.events, event, &err);

    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ERROR_SUCCESS(error);
    return true;
}


bool req_push_read(registered_events_queue* queue, int socket, event_sock_read *event, error_t *error) {
    registered_events_entry *ptr = NULL;
    error_t err;
    ERROR_SUCCESS(&err);
    pr_req_init_before_add(queue, socket, &ptr, &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    eq_push_read(ptr->sock_events.events, event, &err);

    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ERROR_SUCCESS(error);
    return true;
}


bool req_push_write(registered_events_queue* queue, int socket, event_sock_write *event, error_t *error) {
    registered_events_entry *ptr = NULL;
    error_t err;
    ERROR_SUCCESS(&err);
    pr_req_init_before_add(queue, socket, &ptr, &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    eq_push_write(ptr->sock_events.events, event, &err);

    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ERROR_SUCCESS(error);
    return true;
}

