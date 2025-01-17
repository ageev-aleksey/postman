//
// Created by nrx on 09.10.2020.
//

#include "event_loop/events_queue.h"
#include "util.h"

const char EQ_QUEUE_IS_NULL[] = "pointer of events_queue is null";
const char EQ_ELEMENT_IS_NULL[] = "pointer of element of events_queue is null";
const char EQ_ELEMENT_NOT_FOUND[] = "not found element in events_queue";
const char EQ_ELEMENT_TYPE_EXISTS[] = "element this type already exists in event_queue";


events_queue* eq_init(err_t *error) {
    err_t err;
    events_queue *ptr = s_malloc(sizeof(events_queue) ,&err);
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

void eq_free(events_queue *queue) {
    while (!TAILQ_EMPTY(queue)) {
        events_entry *el = TAILQ_FIRST(queue);
        TAILQ_REMOVE(queue, el, entries);
        free(el->event);
        free(el);
    }
    free(queue);
}


bool _el_is_contain(events_queue *queue, event_type type) {
    events_entry *el = NULL;
    TAILQ_FOREACH(el, queue, entries) {
        if (el->event->event.type == type) {
            break;
        }
    }
    return el != NULL;
}

bool _eq_add(events_queue *queue, event_t *event, err_t *error) {
    bool is_contain = _el_is_contain(queue, event->event.type);
    if (is_contain) {
        if (error != NULL) {
            error->error = EXISTS;
            error->message = EQ_ELEMENT_TYPE_EXISTS;
        }
        return false;
    }

    err_t  err;
    ERROR_SUCCESS(&err);
    events_entry *entry = s_malloc(sizeof(events_entry), &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }

    entry->event = event;
    TAILQ_INSERT_TAIL(queue, entry, entries);
    ERROR_SUCCESS(error);
    return true;
}

bool eq_push_accept(events_queue *queue, event_sock_accept *event, err_t *error) {
    CHECK_AND_RETURN(queue, error, EQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, EQ_ELEMENT_IS_NULL);
    event->event.type = SOCK_ACCEPT;
    return _eq_add(queue, (event_t*)event, error);
}

bool eq_push_read(events_queue *queue, event_sock_read *event, err_t *error) {
    CHECK_AND_RETURN(queue, error, EQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, EQ_ELEMENT_IS_NULL);
    event->event.type = SOCK_READ;
    return _eq_add(queue, (event_t*)event, error);
}

bool eq_push_write(events_queue *queue, event_sock_write *event, err_t *error) {
    CHECK_AND_RETURN(queue, error, EQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, EQ_ELEMENT_IS_NULL);
    event->event.type = SOCK_WRITE;
    return _eq_add(queue, (event_t*)event, error);
}

// TODO (aageev) нужен ли отдельный обработчик для события отключения?
//  В данный моент реализация выполняет обработку отключения пользователя в обрабочтике чтения или записи
//bool eq_push_disconnect(events_queue *queue, event_sock_disconnect *event, err_t *error) {
//    CHECK_AND_RETURN(queue, error, EQ_QUEUE_IS_NULL);
//    CHECK_AND_RETURN(event, error, EQ_ELEMENT_IS_NULL);
//    event->event.type = SOCK_DISCONNECT;
//    return _eq_add(queue, (event_t*)event, error);
//}

bool _eq_pop(events_queue *queue, event_t **event, event_type type, err_t *error) {
    CHECK_AND_RETURN(queue, error, EQ_QUEUE_IS_NULL);
    events_entry *el = NULL;
    TAILQ_FOREACH(el, queue, entries) {
        if (el->event->event.type == type) {
            break;
        }
    }
    if (el != NULL) {
        TAILQ_REMOVE(queue, el, entries);
        *event = el->event;
        free(el);
        ERROR_SUCCESS(error);
        return true;
    } else {
        if (error != NULL) {
            error->error = NOT_FOUND;
            error->message = EQ_ELEMENT_NOT_FOUND;
        }
        return false;
    }
}

bool eq_pop_accept(events_queue *queue, event_sock_accept **event, err_t *error) {
    return _eq_pop(queue, (event_t**)event, SOCK_ACCEPT, error);
}
bool eq_pop_read(events_queue *queue, event_sock_read **event, err_t *error)  {
    return _eq_pop(queue, (event_t**)event, SOCK_READ, error);
}
bool eq_pop_write(events_queue *queue, event_sock_write **event, err_t *error)  {
    return _eq_pop(queue, (event_t**)event, SOCK_WRITE, error);
}

//bool eq_pop_disconnect(events_queue *queue, event_sock_disconnect **event, err_t *error) {
//    return _eq_pop(queue, (event_t**)event, SOCK_DISCONNECT, error);
//}


