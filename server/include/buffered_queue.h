#ifndef SERVER_BUFFERED_QUEUE_H
#define SERVER_BUFFERED_QUEUE_H
#include <sys/queue.h>

#ifndef BUFFERED_QUEUE_BUFFER_SIZE
    #define BUFFERED_QUEUE_BUFFER_SIZE 100
#endif

typedef struct _bf_queue_entry {
    char buffer[BUFFERED_QUEUE_BUFFER_SIZE];
    TAILQ_ENTRY(_bf_queue_entry) entries;
} bf_queue_entry;

TAILQ_HEAD(_bf_queue, _bf_queue_entry);
typedef struct _bf_queue bf_queue;


#endif //SERVER_BUFFERED_QUEUE_H
