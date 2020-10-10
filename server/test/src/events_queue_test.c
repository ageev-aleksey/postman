//
// Created by nrx on 09.10.2020.
//
#include "CUnit/Basic.h"
#include "events_queue_test.h"
#include "util.h"
#include "event_loop/error_t.h"

#define ERR_CHECK(error)                    \
do {                                        \
    if ((error).error) {                    \
        CU_FAIL(errtostr((error).error));   \
    }                                       \
} while(0)


int event_queue_test_init() {
    return 0;
}

int event_queue_test_clean() {
    return 0;
}

events_queue *pr_init() {
    error_t error;
    events_queue *eq = eq_init(&error);
    ERR_CHECK(error);
    return eq;
}

void event_queue_push_test() {
    error_t error;
    events_queue *eq = pr_init();
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERR_CHECK(error);
    accept->event.socket = 10;
    eq_push_accept(eq, accept, &error);
    ERR_CHECK(error);
    if (error.error) {
        CU_FAIL();
    }

    event_sock_accept *el = NULL;
    eq_pop_accept(eq, &el, &error);
    ERR_CHECK(error);
    CU_ASSERT_PTR_NOT_NULL(el);
    CU_ASSERT_EQUAL(el->event.type, SOCK_ACCEPT);
    CU_ASSERT(el->event.socket == 10);
    eq_free(eq);
    free(el);
}

void event_queue_double_push_test() {
    error_t error;
    events_queue *eq = pr_init();
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERR_CHECK(error);
    accept->event.socket = 10;
    //accept->event.type = SOCK_ACCEPT;
    eq_push_accept(eq, accept, &error);
    ERR_CHECK(error);
    accept = s_malloc(sizeof(event_sock_accept), &error);
    ERR_CHECK(error);
    accept->event.socket = 20;
    //accept->event.type = SOCK_ACCEPT;
    eq_push_accept(eq, accept, &error);
    CU_ASSERT(error.error == EXISTS);
    int i = 0;
    events_entry *el = NULL;
    TAILQ_FOREACH(el, eq, entries) {
        i++;
    }
    CU_ASSERT_EQUAL(i, 1);
    free(accept);
    eq_free(eq);
}

void event_queue_pop_test() {
    error_t error;
    events_queue *eq = pr_init();
    event_sock_write *event = s_malloc(sizeof(event_sock_write), &error);
    ERR_CHECK(error);
    event->offset = 100;
    eq_push_write(eq, event, &error);
    ERR_CHECK(error);
    event = NULL;
    eq_pop_write(eq, &event, &error);
    CU_ASSERT_PTR_NOT_NULL(event);
    CU_ASSERT_EQUAL(event->offset, 100);
    eq_free(eq);
    free(event);
}

void event_queue_empty_pop_test() {
    error_t error;
    events_queue *eq = pr_init();
    event_sock_write *event = s_malloc(sizeof(event_sock_write), &error);
    ERR_CHECK(error);
    event->offset = 100;
    eq_push_write(eq, event, &error);
    event_sock_read *event_read = NULL;
    eq_pop_read(eq, &event_read, &error);
    CU_ASSERT_EQUAL(error.error, NOT_FOUND);
    CU_ASSERT_PTR_NULL(event_read);
    eq_free(eq);
}