#include "registered_events_queue_test.h"
#include "event_loop/registered_events_queue.h"
#include "CUnit/Basic.h"
#include "util.h"

#define ERROR_ASSERT(err)   \
do {                        \
      if ((err).error) {    \
         CU_FAIL();         \
      }                     \
} while(0)

int registered_events_queue_test_init() {
    return 0;
}

int registered_events_queue_test_clean() {
    return 0;
}

void registered_events_queue_push_accept_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);
    event_sock_accept *el = s_malloc(sizeof(event_sock_accept), &error);
    ERROR_ASSERT(error);
    el->handler = NULL;
    req_push_accept(queue, 10, el, &error);
    ERROR_ASSERT(error);
    registered_events_entry *ptr = TAILQ_FIRST(queue);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_EQUAL(ptr->sock_events.sock, socket);
    CU_ASSERT_PTR_NOT_NULL(ptr->sock_events.events);
    events_entry *eentry = TAILQ_FIRST(ptr->sock_events.events);
    CU_ASSERT_PTR_NOT_NULL(eentry);
    CU_ASSERT_PTR_NOT_NULL(eentry->event);
    CU_ASSERT_EQUAL(eentry->event->event.socket, socket);
    CU_ASSERT_EQUAL(eentry->event->event.type, SOCK_ACCEPT);
    req_free(queue);
}


void registered_events_queue_push_read_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);
    event_sock_read *el = s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    el->handler = NULL;
    req_push_read(queue, 10, el, &error);
    ERROR_ASSERT(error);
    registered_events_entry *ptr = TAILQ_FIRST(queue);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_EQUAL(ptr->sock_events.sock, socket);
    CU_ASSERT_PTR_NOT_NULL(ptr->sock_events.events);
    events_entry *eentry = TAILQ_FIRST(ptr->sock_events.events);
    CU_ASSERT_PTR_NOT_NULL(eentry);
    CU_ASSERT_PTR_NOT_NULL(eentry->event);
    CU_ASSERT_EQUAL(eentry->event->event.socket, socket);
    CU_ASSERT_EQUAL(eentry->event->event.type, SOCK_READ);
    req_free(queue);
}

void registered_events_queue_push_write_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);
    event_sock_write *el = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    el->handler = NULL;
    req_push_write(queue, 10, el, &error);
    ERROR_ASSERT(error);
    registered_events_entry *ptr = TAILQ_FIRST(queue);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    CU_ASSERT_EQUAL(ptr->sock_events.sock, socket);
    CU_ASSERT_PTR_NOT_NULL(ptr->sock_events.events);
    events_entry *eentry = TAILQ_FIRST(ptr->sock_events.events);
    CU_ASSERT_PTR_NOT_NULL(eentry);
    CU_ASSERT_PTR_NOT_NULL(eentry->event);
    CU_ASSERT_EQUAL(eentry->event->event.socket, socket);
    CU_ASSERT_EQUAL(eentry->event->event.type, SOCK_WRITE);
    req_free(queue);
}

void registered_events_queue_pop_accept_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);

    ERROR_ASSERT(error);
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERROR_ASSERT(error);
    event_sock_read *read =  s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    event_sock_write *write = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    accept->event.socket = socket;

    req_push_accept(queue, socket, accept, &error);
    ERROR_ASSERT(error);

    req_push_read(queue, socket, read, &error);
    ERROR_ASSERT(error);

    req_push_write(queue, socket, write, &error);
    ERROR_ASSERT(error);

    event_sock_accept *accept_el = req_pop_accept(queue, socket, &error);
    ERROR_ASSERT(error);
    CU_ASSERT_EQUAL(accept_el->event.socket, socket)
    CU_ASSERT_EQUAL(accept_el->event.type, SOCK_ACCEPT);

    event_sock_accept *accept_el2 = req_pop_accept(queue, socket, &error);
    CU_ASSERT_EQUAL(error.error, NOT_FOUND);
    CU_ASSERT_PTR_NULL(accept_el2);
    free(accept_el);
    req_free(queue);
}

void registered_events_queue_pop_read_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);

    ERROR_ASSERT(error);
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERROR_ASSERT(error);
    event_sock_read *read =  s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    event_sock_write *write = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    accept->event.socket = socket;

    req_push_accept(queue, socket, accept, &error);
    ERROR_ASSERT(error);

    req_push_read(queue, socket, read, &error);
    ERROR_ASSERT(error);

    req_push_write(queue, socket, write, &error);
    ERROR_ASSERT(error);

    event_sock_read *accept_el = req_pop_read(queue, socket, &error);
    ERROR_ASSERT(error);
    CU_ASSERT_EQUAL(accept_el->event.socket, socket)
    CU_ASSERT_EQUAL(accept_el->event.type, SOCK_READ);

    event_sock_read *accept_el2 = req_pop_read(queue, socket, &error);
    CU_ASSERT_EQUAL(error.error, NOT_FOUND);
    CU_ASSERT_PTR_NULL(accept_el2);
    free(accept_el);
    req_free(queue);
}


void registered_events_queue_pop_write_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);
    ERROR_ASSERT(error);

    ERROR_ASSERT(error);
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERROR_ASSERT(error);
    event_sock_read *read =  s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    event_sock_write *write = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    accept->event.socket = socket;

    req_push_accept(queue, socket, accept, &error);
    ERROR_ASSERT(error);

    req_push_read(queue, socket, read, &error);
    ERROR_ASSERT(error);

    req_push_write(queue, socket, write, &error);
    ERROR_ASSERT(error);

    event_sock_write *accept_el = req_pop_write(queue, socket, &error);
    ERROR_ASSERT(error);
    CU_ASSERT_EQUAL(accept_el->event.socket, socket)
    CU_ASSERT_EQUAL(accept_el->event.type, SOCK_WRITE);

    event_sock_write *accept_el2 = req_pop_write(queue, socket, &error);
    CU_ASSERT_EQUAL(error.error, NOT_FOUND);
    CU_ASSERT_PTR_NULL(accept_el2);
    free(accept_el);
    req_free(queue);
}

void registered_events_queue_bitmask_test() {
    int socket = 10;
    error_t error;
    ERROR_SUCCESS(&error);
    registered_events_queue *queue = req_init(&error);

    ERROR_ASSERT(error);
    event_sock_accept *accept = s_malloc(sizeof(event_sock_accept), &error);
    ERROR_ASSERT(error);
    event_sock_read *read =  s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    event_sock_write *write = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    accept->event.socket = socket;

    req_push_accept(queue, socket, accept, &error);
    ERROR_ASSERT(error);

    req_push_read(queue, socket, read, &error);
    ERROR_ASSERT(error);

    req_push_write(queue, socket, write, &error);
    ERROR_ASSERT(error);

    int bit_map = req_reg(queue, socket, &error);
    ERROR_ASSERT(error);
    CU_ASSERT_EQUAL(bit_map, REQ_WRITE_EVENT | REQ_READ_EVENT | REQ_ACCEPT_EVENT);

    req_free(queue);
}