//
// Created by nrx on 16.12.2020.
//

#include <stdlib.h>
#include "server/timers.h"


void timers_make_for_socket(timers_t *timers, int socket) {
    timer_type timer;
    timer.socket = socket;
    timer.last_update = time(NULL);
    timers_add(timers, &timer);
}

struct timer_entry* pr_timers_get_by_sock(timers_t *timers, int sock) {
    bool isFound = false;
    struct timer_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, &timers->list, entries) {
        if (ptr->t.socket == sock) {
            isFound = true;
            break;
        }
    }
    if (isFound) {
        return ptr;
    }
    return NULL;
}

void timers_remove_for_socket(timers_t *timers, int sock) {
    pthread_mutex_lock(&timers->pr_mutex);
    struct timer_entry *ptr = pr_timers_get_by_sock(timers, sock);
    if (ptr) {
        TAILQ_REMOVE(&timers->list, ptr, entries);
        free(ptr);
    }

    pthread_mutex_unlock(&timers->pr_mutex);
}
void timers_update_by_socket(timers_t * timers, int sock) {
    pthread_mutex_lock(&timers->pr_mutex);
    struct timer_entry *ptr = pr_timers_get_by_sock(timers, sock);
    if (ptr) {
        ptr->t.last_update = time(NULL);
    }
    pthread_mutex_unlock(&timers->pr_mutex);
}

bool timers_is_elapsed_for_socket(timers_t * timers, int sock, int t) {
    pthread_mutex_lock(&timers->pr_mutex);
    struct timer_entry *ptr = pr_timers_get_by_sock(timers, sock);
    bool is = false;
    if (ptr != NULL) {
        is = (ptr->t.last_update + t) < time(NULL);
    }
    pthread_mutex_unlock(&timers->pr_mutex);
    return is;
}

bool timers_init(timers_t *timers) {
    if (timers != NULL) {
        pthread_mutex_init(&timers->pr_mutex, NULL);
        TAILQ_INIT(&timers->list);
        return true;
    }
    return false;
}

void timers_free(timers_t *timers) {
    pthread_mutex_destroy(&timers->pr_mutex);
    while (!TAILQ_EMPTY(&timers->list)) {
        struct timer_entry *ptr = TAILQ_FIRST(&timers->list);
        TAILQ_REMOVE(&timers->list, ptr, entries);
        free(ptr);
    }
}

bool timers_add(timers_t *timers, const timer_type *timer) {
    if (timers != NULL && timer != NULL) {
        struct timer_entry *t = malloc(sizeof(struct timer_entry));
        t->t = *timer;
        t->is_delete = false;
        t->num_iterators = 0;
        pthread_mutex_lock(&timers->pr_mutex);
            TAILQ_INSERT_TAIL(&timers->list, t, entries);
        pthread_mutex_unlock(&timers->pr_mutex);
        return true;
    }
    return false;
}

timers_iterator timers_get_iterator(timers_t *timers) {
    struct timers_iterator iterator;
    iterator.pr_timers = timers;
    iterator.pr_ptr = NULL;
    return iterator;
}
timer_type timers_next(timers_iterator *itr) {
    timer_type ret;
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    if (itr->pr_ptr == NULL) {

    } else {
        itr->pr_ptr->num_iterators--;
        if (itr->pr_ptr->is_delete && itr->pr_ptr->num_iterators == 0) {
            free(itr->pr_ptr);
        } else {

        }
        itr->pr_ptr = TAILQ_NEXT(itr->pr_ptr, entries);
    }

    ret = itr->pr_ptr->t;
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
    return ret;
}

timer_type timers_before(timers_iterator *itr) {
    timer_type ret;
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    itr->pr_ptr = TAILQ_PREV(itr->pr_ptr, timers_list, entries);
    ret = itr->pr_ptr->t;
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
    return ret;
}

bool timers_has_next(timers_iterator *itr) {
    bool status = false;
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    status = (itr->pr_ptr == TAILQ_LAST(&itr->pr_timers->list, timers_list));
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
    return status;
}

bool timers_has_before(timers_iterator *itr) {
    bool status = false;
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    status = (itr->pr_ptr == TAILQ_FIRST(&itr->pr_timers->list));
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
    return status;
}

void timers_update(timers_iterator *itr, const timer_type *t) {
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    itr->pr_ptr->t = *t;
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
}

void timers_remove(timers_iterator *itr) {
    pthread_mutex_lock(&itr->pr_timers->pr_mutex);
    TAILQ_REMOVE(&itr->pr_timers->list, itr->pr_ptr, entries);
    pthread_mutex_unlock(&itr->pr_timers->pr_mutex);
}