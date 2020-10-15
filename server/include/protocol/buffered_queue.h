#ifndef SERVER_BUFFERED_QUEUE_H
#define SERVER_BUFFERED_QUEUE_H
#include "error_t.h"
#include <sys/queue.h>

#ifndef BFQ_BUFFER_SIZE
    #define BFQ_BUFFER_SIZE 100
#endif

typedef struct _bf_queue_entry {
    char buffer[BFQ_BUFFER_SIZE];
    TAILQ_ENTRY(_bf_queue_entry) entries;
} bf_queue_entry;

TAILQ_HEAD(_bf_queue, _bf_queue_entry);
typedef struct _bf_queue bf_queue;


bool bfq_init(bf_queue **queue, error_t *error);
bool bfq_add(bf_queue *queue, error_t *error);
bool bfq_size(bf_queue *queue, int *size, error_t *error);
bool bfq_glue(bf_queue *queue,char **ptr, error_t *error);
void bfq_free(bf_queue *queue);


#endif //SERVER_BUFFERED_QUEUE_H
