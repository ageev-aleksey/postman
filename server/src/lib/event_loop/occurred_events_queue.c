#include "event_loop/occurred_event_queue.h"
#include "util.h"

const char OEQ_QUEUE_IS_NULL[] = "ptr of occurred event queue is null";
const char OEQ_RETURN_ARGUMENT_IS_NULL[] = "ptr of return argument is null";
const char OEQ_QUEUE_IS_EMPTY[] = "ptr of occurred event queue is empty";
const char OEQ_PUSH_ARGUMENT_IS_NULL[] = "ptr of element wil bi pushing in occurred event queue is null";


occurred_event_queue* oeq_init(error_t *error) {
    error_t err;
    ERROR_SUCCESS(&err);
    occurred_event_queue *q = s_malloc(sizeof(occurred_event_queue), &err);
    if (error != NULL) {
        *error = err;
    }
    TAILQ_INIT(q);
    return q;
 }

bool oeq_pop_begin(occurred_event_queue *queue, event_t **event, error_t *error) {
    CHECK_AND_RETURN(queue, error, OEQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, OEQ_RETURN_ARGUMENT_IS_NULL);
    if (TAILQ_EMPTY(queue)) {
        if (error != NULL) {
            error->error = EMPTY;
            error->message = OEQ_QUEUE_IS_EMPTY;
        }
        *event = NULL;
        return false;
    }
    occurred_event_entry *entry = TAILQ_FIRST(queue);
    TAILQ_REMOVE(queue, entry, entries);
    *event = entry->element.event;
    free(entry);
    ERROR_SUCCESS(error);
    return true;
}

bool oeq_pop_back(occurred_event_queue *queue, event_t **event, error_t *error) {
    CHECK_AND_RETURN(queue, error, OEQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, OEQ_RETURN_ARGUMENT_IS_NULL);
    if (TAILQ_EMPTY(queue)) {
        if (error != NULL) {
            error->error = EMPTY;
            error->message = OEQ_QUEUE_IS_EMPTY;
        }
        *event = NULL;
        return false;
    }
    occurred_event_entry *entry = TAILQ_LAST(queue, _occurred_event_queue);
    TAILQ_REMOVE(queue, entry, entries);
    *event = entry->element.event;
    free(entry);
    ERROR_SUCCESS(error);
    return true;
}


bool oeq_push_begin(occurred_event_queue *queue, event_t *event, error_t *error) {
    CHECK_AND_RETURN(queue, error, OEQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, OEQ_PUSH_ARGUMENT_IS_NULL);
    error_t err;
    ERROR_SUCCESS(&err);
    occurred_event_entry *ptr = s_malloc(sizeof(occurred_event_entry), &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ptr->element.event = event;
    TAILQ_INSERT_HEAD(queue, ptr, entries);
    ERROR_SUCCESS(error);
    return true;
}

bool oeq_push_back(occurred_event_queue *queue, event_t *event, error_t *error) {
    CHECK_AND_RETURN(queue, error, OEQ_QUEUE_IS_NULL);
    CHECK_AND_RETURN(event, error, OEQ_PUSH_ARGUMENT_IS_NULL);
    error_t err;
    ERROR_SUCCESS(&err);
    occurred_event_entry *ptr = s_malloc(sizeof(occurred_event_entry), &err);
    if (err.error) {
        if (error != NULL) {
            *error = err;
        }
        return false;
    }
    ptr->element.event = event;
    TAILQ_INSERT_TAIL(queue, ptr, entries);
    ERROR_SUCCESS(error);
    return true;
}


void oeq_free(occurred_event_queue*  queue) {
    if (queue != NULL) {

        while(!TAILQ_EMPTY(queue)) {
            occurred_event_entry *ptr = NULL;
            ptr = TAILQ_FIRST(queue);
            TAILQ_REMOVE(queue, ptr, entries);
            if (ptr->element.event != NULL) {
                free(ptr->element.event);
            }
            free(ptr);
        }
        free(queue);
    }
}