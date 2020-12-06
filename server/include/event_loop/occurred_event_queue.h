//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_OCCURRED_EVENT_QUEUE_H
#define SERVER_OCCURRED_EVENT_QUEUE_H

#include "event_t.h"

#include <sys/queue.h>

typedef struct _occurred_event {
    event_t *event;
} occurred_event;

typedef struct _occurred_event_entry {
    occurred_event element;
    TAILQ_ENTRY(_occurred_event_entry) entries;
} occurred_event_entry;

TAILQ_HEAD(_occurred_event_queue, _occurred_event_entry);
typedef struct _occurred_event_queue occurred_event_queue;

/////////// occurred_event_queue - oeq ///////////
/**
 *
 * @param error
 * @return
 */
occurred_event_queue* oeq_init(err_t *error);
int oeq_size(occurred_event_queue *queue, err_t *error);
bool oeq_pop_begin(occurred_event_queue *queue, event_t **event, err_t *error);
bool oeq_pop_back(occurred_event_queue *queue, event_t **event, err_t *error);

bool oeq_push_begin(occurred_event_queue *queue, event_t *event, err_t *error);
bool oeq_push_back(occurred_event_queue *queue, event_t *event, err_t *error);
/**
 * Осовбождение памяти из подвсех элементов и очереди
 * @param queue очередь, из под которой необходимо освободить память.
 */
void oeq_free(occurred_event_queue*  queue);

#endif //SERVER_OCCURRED_EVENT_QUEUE_H
