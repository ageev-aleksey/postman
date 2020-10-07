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


#endif //SERVER_EVENTS_QUEUE_H
