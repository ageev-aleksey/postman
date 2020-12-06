#include "protocol/buffered_queue.h"
#include "util.h"

const char BFQ_PTR_IS_NULL[] = "the pointer to which the head is written is null";
const char BFQ_HEAD_IS_NULL[] = "head of buffered queue is null";
const char BFQ_BUFFER_PTR_IS_NULL[] = "the pointer to which the buffer is written is null";
const char BFQ_SIZE_PTR_IS_NULL[] = "the pointer to which the size is written is null";

bool bfq_init(bf_queue **queue, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_PTR_IS_NULL;
        }
        return false;
    }
    ERROR_SUCCESS(error);
    bf_queue *head = s_malloc(sizeof(bf_queue), error);
    if (head != NULL) {
        TAILQ_INIT(head);
        *queue = head;
        return true;
    }
    return false;
}

bool bfq_add(bf_queue *queue, char **buffer, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_HEAD_IS_NULL;
        }
        return false;
    }
    ERROR_SUCCESS(error);
    bf_queue_entry *entry = s_malloc(sizeof(bf_queue_entry), error);
    if (entry == NULL) {
        return false;
    }
    TAILQ_INSERT_TAIL(queue, entry, entries);
    return true;
}

bool bfq_size(bf_queue *queue, int *size, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_HEAD_IS_NULL;
        }
        return false;
    }

    if (size == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_SIZE_PTR_IS_NULL;
        }
        return false;
    }
    ERROR_SUCCESS(error);

    int res = 0;
    if (!TAILQ_EMPTY(queue)) {
        bf_queue_entry *entry = NULL;
        TAILQ_FOREACH(entry, queue, entries) {
            res++;
        }
    }

    *size = res;
    return true;
}

bool bfq_glue(bf_queue *queue,char **ptr, error_t *error) {
    if (queue == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_HEAD_IS_NULL;
        }
        return false;
    }

    if (ptr == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = BFQ_BUFFER_PTR_IS_NULL;
        }
        return false;
    }
    int size = 0;
    if (bfq_size(queue, &size, error)) {
        int buffer_size = BFQ_BUFFER_SIZE * size;
        char *buffer = s_malloc(buffer_size, error);
        if (buffer != NULL) {
            bf_queue_entry *entry = NULL;
            int j = 0;
            TAILQ_FOREACH(entry, queue, entries) {
                for (int i = 0; i < BFQ_BUFFER_SIZE; i++) {
                    buffer[j] = entry->buffer[i];
                    j++;
                }
            }
        }
        *ptr = buffer;
        return true;
    }
    return false;
}

void bfq_free(bf_queue *queue) {
    if (queue != NULL) {
        free(queue);
    }
}