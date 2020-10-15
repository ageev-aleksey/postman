#include "protocol/buffered_queue.h"

#include "sys/queue.h"

typedef struct __context {
    int socket;
    bf_queue bf;
} context_t;


typedef struct __context_entry {
    context_t context;
    TAILQ_ENTRY(__context_entry) entries;
} context_entry;

TAILQ_HEAD(__context_queue, __context_entry);
typedef struct __context_queue context_queue;

bool context_init(context_queue *queue, error_t *error);
bool context_add(context_queue *queue, context_t *context, error_t *error);
bool context_find(context_queue *queue, int socket, error_t *error);
