//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_EVENTS_QUEUE_H
#define SERVER_EVENTS_QUEUE_H

#include <sys/queue.h>
#include "event_t.h"

typedef struct _events_entry {
    event_t *event;
    TAILQ_ENTRY(_events_entry) entries;
} events_entry;

TAILQ_HEAD(_events_queue,  _events_entry);
typedef struct _events_queue events_queue;

events_queue* eq_init(error_t *error);
void eq_free(events_queue *queue);

bool eq_push_accept(events_queue *queue, event_sock_accept *event, error_t *error);
bool eq_push_read(events_queue *queue, event_sock_read *event, error_t *error);
bool eq_push_write(events_queue *queue, event_sock_write *event, error_t *error);

bool eq_pop_accept(events_queue *queue, event_sock_accept **event, error_t *error);
bool eq_pop_read(events_queue *queue, event_sock_read **event, error_t *error);
bool eq_pop_write(events_queue *queue, event_sock_write **event, error_t *error);

#endif //SERVER_EVENTS_QUEUE_H
