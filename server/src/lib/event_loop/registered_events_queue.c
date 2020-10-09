//
// Created by nrx on 08.10.2020.
//

#include "event_loop/registered_events_queue.h"
#include "event_loop/events_queue.h"
#include "util.h"

#include <assert.h>

const char REQ_QUEUE_IS_NULL[] = "pointer of registered_events_queue is null";
const char REQ_SOCK_NOTFOUND[] = "Not found registered events for socket in registered_events_queue";
const char REQ_INVALID_SOCKET_VALUE[] = "Invalid socket";
const char REQ_EVENT_IS_NULL[] = "Event pointer is null";


 int REQ_ACCEPT_EVENT = 0b1;
 int REQ_READ_EVENT = 0b10;
 int REQ_WRITE_EVENT = 0b100;

registered_events_entry* pr_req_get_sock(registered_events_queue *queue, int sock) {

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

    registered_events_entry *ptr = pr_req_get_sock(queue, socket);
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
    ERROR_SUCCESS(error);
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

event_t* pr_req_pop(registered_events_queue* queue, int socket, error_t *error,
                   void(*element_pop)(events_queue *queue, event_t **event, error_t *error)) {
    CHECK_AND_RETURN(queue, error, REQ_QUEUE_IS_NULL);
    if (socket < 0) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = REQ_INVALID_SOCKET_VALUE;
        }
        return NULL;
    }
    registered_events_entry *ptr = pr_req_get_sock(queue, socket);
    ///////////////

    if (ptr != NULL) {
        ///////////////////////////////////////////////////////////
        event_t *res = NULL;
        element_pop(ptr->sock_events.events, &res, error);
        //////////////////////////////////////////////////////////
        if (TAILQ_EMPTY(ptr->sock_events.events)) {
            eq_free(ptr->sock_events.events);
            TAILQ_REMOVE(queue, ptr, entries);
            free(ptr);
        }
        ERROR_SUCCESS(error);
        return res;
    }

    if (error != NULL) {
        error->error = NOT_FOUND;
        error->message = REQ_SOCK_NOTFOUND;
    }
    return NULL;
}




/////////////////////////////

void pr_pop_accept_handler(events_queue *ptr, event_t **event, error_t *error) {
    event_sock_accept *accept = NULL;
    eq_pop_accept(ptr, &accept, error);
    *event = (event_t*) accept;
}

event_sock_accept* req_pop_accept(registered_events_queue* queue, int socket, error_t *error) {
    event_t* ptr = pr_req_pop(queue, socket, error, pr_pop_accept_handler);

    if (ptr != NULL) {
        assert(ptr->event.type == SOCK_ACCEPT);

        ERROR_SUCCESS(error);
        return (event_sock_accept*) ptr;
    }
    return NULL;

}

///////////////////////////
void pr_pop_read_handler(events_queue *ptr, event_t **event, error_t *error) {
    event_sock_read *read = NULL;
    eq_pop_read(ptr, &read, error);
    *event = (event_t*) read;
}

event_sock_read* req_pop_read(registered_events_queue* queue, int socket, error_t *error) {
    event_t* ptr = pr_req_pop(queue, socket, error, pr_pop_read_handler);

    if (ptr != NULL) {
        assert(ptr->event.type == SOCK_READ);

        ERROR_SUCCESS(error);
        return (event_sock_read*) ptr;
    }
    return NULL;
}

///////////////////////////

void pr_pop_write_handler(events_queue *ptr, event_t **event, error_t *error) {
    event_sock_write *read = NULL;
    eq_pop_write(ptr, &read, error);
    *event = (event_t*) read;
}

event_sock_write* req_pop_write(registered_events_queue* queue, int socket, error_t *error) {
    event_t* ptr = pr_req_pop(queue, socket, error, pr_pop_read_handler);

    if(ptr != NULL) {
        assert(ptr->event.type == SOCK_WRITE);

        ERROR_SUCCESS(error);
        return (event_sock_write*) ptr;
    }
    return NULL;
}


int req_reg(registered_events_queue* queue, int socket, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = REQ_QUEUE_IS_NULL;
        }
        return -1;
    }
    int bit_map = 0;
    registered_events_entry *ptr = pr_req_get_sock(queue, socket);
    events_entry *el = NULL;
    TAILQ_FOREACH(el, ptr->sock_events.events, entries) {

        assert(el->event != NULL);

        if (el->event->event.type == SOCK_ACCEPT) {
            bit_map |= REQ_ACCEPT_EVENT;
        } else if (el->event->event.type == SOCK_READ) {
            bit_map |= REQ_READ_EVENT;
        } else if (el->event->event.type == SOCK_WRITE) {
            bit_map |= REQ_WRITE_EVENT;
        }
    }
    ERROR_SUCCESS(error);
    return 0;
}

void req_free(registered_events_queue* queue) {
    if (queue != NULL) {
        registered_events_entry *ptr = NULL;
        TAILQ_FOREACH(ptr, queue, entries) {
            eq_free(ptr->sock_events.events);
        }
        free(queue);
    }
}