#include "protocol/context.h"
#include "util.h"

const char CQ_PTR_IS_NULL[] = "ptr of context is null";
const char CQ_CONTEXT_NOTFOUND[] = "context not found";


context_queue* cq_init(error_t *error) {
    ERROR_SUCCESS(error);
    context_queue  *context = s_malloc(sizeof(context_queue),error);
    if (context != NULL) {
        TAILQ_INIT(context);
    }
    return context;
}

bool cq_add(context_queue *queue, context_t *context, error_t *error) {
    ERROR_SUCCESS(error);
    if (queue == NULL || context == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = CQ_PTR_IS_NULL;
        }
        return false;
    }
    context_entry  *entry = s_malloc(sizeof(context_entry), error);
    if (entry == NULL) {
        return false;
    }
    entry->context = context;
    TAILQ_INSERT_TAIL(queue, entry, entries);
    return true;
}

context_entry* pr_cq_find(context_queue *queue, int socket, error_t *error) {
    ERROR_SUCCESS(error);
    if (queue == NULL || context == NULL) {
        if (error != NULL) {
            error->error = FATAL;
            error->message = CQ_PTR_IS_NULL;
        }
        return false;
    }
    context_entry  *ptr = NULL;
    TAILQ_FOREACH(ptr, queue, entries) {
        if (ptr->context->socket == socket) {
            return ptr;
        }
    }
    if (error != NULL) {
        error->error = NOT_FOUND;
        error->message = CQ_CONTEXT_NOTFOUND;
    }
    return NULL;
}

bool cq_find(context_queue *queue, context_t **context, int socket, error_t *error){
    context_entry *entry = pr_cq_find(queue, socket, error);
    if (entry == NULL) {
        *context = NULL;
        return false;
    }
    *context = entry->context;
    return true;
}

bool cq_del(context_queue *queue, int socket, error_t *error) {
    ERROR_SUCCESS(error);
    context_entry *entry = pr_cq_find(queue, socket, error);
    if (entry == NULL) {
        return  false;
    }

    TAILQ_REMOVE(queue, entry, entries);
    return true;
}

void cq_free(context_queue *queue) {
    if (queue != NULL) {
        while(!TAILQ_EMPTY(queue)) {
            context_entry *entry = TAILQ_FIRST(queue);
            TAILQ_REMOVE(queue, entry, entries);
            free(entry->context);
            free(entry);
        }
        free(queue);
    }
}