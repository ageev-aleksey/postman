//
// Created by nrx on 12.12.2020.
//

#ifndef SERVER_THREAD_POOL_H
#define SERVER_THREAD_POOL_H
#include <sys/queue.h>
#include <pthread.h>
#include <stdbool.h>

typedef void* (*thread_action)(void *args);

typedef struct thread_entry {
    pthread_mutex_t mutex;
    pthread_t thread;
    bool is_run;
    thread_action action;
    void *args;
    void *res;
    TAILQ_ENTRY(thread_entry) entries;
} thread_entry;

TAILQ_HEAD(pth_list, thread_entry);

typedef struct thread_pool {
    struct pth_list pr_active_threads;  /// Список активных потоков
    size_t num_threads;                 /// число актвиных потоков
    pthread_mutex_t pr_mutex;
} thread_pool_t;



void thread_pool_init(thread_pool_t *pool);
void thread_pool_free(thread_pool_t *pool);
void thread_pool_push(thread_pool_t *pool, thread_action action, void *args);



#endif //SERVER_THREAD_POOL_H
