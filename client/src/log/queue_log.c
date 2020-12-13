#include "queue_log.h"

TAILQ_HEAD(head_s, node) head;

void init_logs() {
    TAILQ_INIT(&head);
}

void push_log(log value) {
    node *new = malloc(sizeof(node));
    new->data = value;
    TAILQ_INSERT_TAIL(&head, new, nodes);
}

log pop_log() {
    node *old = TAILQ_FIRST(&head);
    TAILQ_REMOVE(&head, old, nodes);
    log data = old->data;
    free(old);
    return data;
}

bool is_logs_queue_empty() {
   return TAILQ_EMPTY(&head);
}