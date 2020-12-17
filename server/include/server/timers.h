//
// Created by nrx on 16.12.2020.
//

#ifndef SERVER_TIMERS_H
#define SERVER_TIMERS_H


#include <pthread.h>
#include <sys/queue.h>
#include <stdbool.h>

typedef struct timer {
    int socket;
    int last_update;
} timer_type;

struct timer_entry {
    timer_type t;
    bool is_delete;
    int num_iterators;
    TAILQ_ENTRY(timer_entry) entries;
};

TAILQ_HEAD(timers_list, timer_entry);
typedef struct timers_list timers_list;

typedef struct timers {
    pthread_mutex_t pr_mutex;
    timers_list list;
} timers_t;

typedef struct timers_iterator {
    struct timer_entry *pr_ptr;
    timers_t *pr_timers;
} timers_iterator;

void timers_make_for_socket(timers_t *timers, int socket);
void timers_remove_for_socket(timers_t *timers, int socket);
void timers_update_by_socket(timers_t * timers, int socket);
bool timers_is_elapsed_for_socket(timers_t * timers, int socket, int t);


bool timers_init(timers_t *timers);
void timers_free(timers_t *timers);
bool timers_add(timers_t *timers, const timer_type *timer);
timers_iterator timers_get_iterator(timers_t *timers);
timer_type timers_next(timers_iterator *itr);
timer_type timers_before(timers_iterator *itr);
bool timers_has_next(timers_iterator *itr);
bool timers_has_before(timers_iterator *itr);
void timers_update(timers_iterator *itr, const timer_type *t);
void timers_remove(timers_iterator *itr);

#endif //SERVER_TIMERS_H
