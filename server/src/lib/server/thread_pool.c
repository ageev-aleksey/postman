#include "server/thread_pool.h"
#include <stdlib.h>


bool pr_thread_is_run(struct thread_entry *thread) {
    bool ret = false;
    pthread_mutex_lock(&thread->mutex);
    ret = thread->is_run;
    pthread_mutex_unlock(&thread->mutex);
    return ret;
}

void pr_thread_set_run(struct thread_entry *thread, bool value) {
    pthread_mutex_lock(&thread->mutex);
    thread->is_run = value;
    pthread_mutex_unlock(&thread->mutex);
}

void* pr_thread_input(void *args) {
    thread_entry *thread = args;
    pr_thread_set_run(thread, true);
    void *res = thread->action(thread->args);
    pr_thread_set_run(thread, false);
    return res;
}

void pr_thread_entry_free(struct thread_entry *thread){
    if (thread != NULL) {
        void *ret = NULL;
        pthread_join(thread->thread, &ret);
        pthread_mutex_destroy(&thread->mutex);
    }
}



void thread_pool_init(thread_pool_t *pool) {
    pool->num_threads = 0;
    TAILQ_INIT(&pool->pr_active_threads);
    pthread_mutex_init(&pool->pr_mutex, NULL);
}


void thread_pool_free(thread_pool_t *pool) {
    if (pool != NULL) {
        pthread_mutex_destroy(&pool->pr_mutex);
        struct thread_entry *ptr = NULL;
        while(!TAILQ_EMPTY(&pool->pr_active_threads)) {
            ptr = TAILQ_FIRST(&pool->pr_active_threads);
            TAILQ_REMOVE(&pool->pr_active_threads, ptr, entries);
            pr_thread_entry_free(ptr);
            free(ptr);
        }
    }
}

void pr_thread_entry_init( struct thread_entry * entry) {
    entry->is_run = false;
    pthread_mutex_init(&entry->mutex, NULL);
    entry->action = NULL;
    entry->args = NULL;
    entry->res = NULL;
}

void thread_pool_push(thread_pool_t *pool, thread_action action, void *args) {
    struct thread_entry *ptr = malloc(sizeof(struct thread_entry));
    pr_thread_entry_init(ptr);
    ptr->args = args;
    ptr->action = action;
    pthread_create(&ptr->thread, NULL, pr_thread_input, ptr);
    pthread_mutex_lock(&pool->pr_mutex);
    TAILQ_INSERT_TAIL(&pool->pr_active_threads, ptr, entries);
    pthread_mutex_unlock(&pool->pr_mutex);
}
