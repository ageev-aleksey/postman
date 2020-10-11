#include "CUnit/Basic.h"
#include "event_loop/event_loop.h"
#include "event_loop_test.h"
#include "util.h"
#include <stdlib.h>

#define ERROR_ASSERT(_error_)   \
do {                            \
    if (error.error) {          \
        CU_FAIL();              \
    }                           \
} while(0)


int event_loop_test_init() {
   return 0;
}

int event_loop_test_clean() {
   return 0;
}

void event_loop_initialize_test() {
    error_t  error;
    ERROR_SUCCESS(&error);
    event_loop *loop = el_init(&error);
    ERROR_ASSERT(error);
    el_close(loop);
}

void create_pollfd_array_test() {
    int socket1 = 10;
    int socket2 = 20;
    error_t error;
    ERROR_SUCCESS(&error);
    event_loop *loop = el_init(&error);
    ERROR_ASSERT(error);

    event_sock_write *write1 = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    req_push_write(loop->_sock_events, socket1, write1, &error);
    ERROR_ASSERT(error);
    event_sock_read  *read1 = s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);
    req_push_read(loop->_sock_events, socket1, read1, &error);

    event_sock_write *write2 = s_malloc(sizeof(event_sock_write), &error);
    ERROR_ASSERT(error);
    req_push_write(loop->_sock_events, socket2, write2, &error);
    ERROR_ASSERT(error);

    struct pollfd *fd_array = NULL;
    int res_size = 0;
    pr_create_pollfd(loop, &fd_array, &res_size, &error);
    ERROR_ASSERT(error);

    CU_ASSERT_EQUAL(res_size, 2);
    for (int i = 0; i < 2; ++i) {
        if (fd_array[i].fd == socket1) {
            CU_ASSERT_EQUAL(fd_array[i].events, POLLIN | POLLOUT);
        } else if (fd_array[i].fd == socket2) {
            CU_ASSERT_EQUAL(fd_array[i].events, POLLOUT);
        } else {
            CU_FAIL();
        }
    }

    if (loop != NULL) {
        el_close(loop);
    }
    if (fd_array != NULL) {
        free(fd_array);
    }

}

void create_pollin_occurred_events_test() {
//    event_loop el;
//    el_init(&el);
//
//    int sock1 = 1;
//    int sock2 = 2;
//    int sock3 = 3;
//    struct pollfd fd_array[] = {
//            {sock1, POLLIN, 0},
//            {sock2, POLLIN, 0},
//            {sock3, POLLIN, 0}
//    };
//    el._index_acepptors_start = 2; // последний элемент fd_array относится к событию принятия подключения от клиента
//
//    registered_events_entry *registered_entry = (registered_events_entry*)malloc(sizeof(registered_events_entry));
//
//     //Создаем список событий для сокета 1
//    registered_entry->sock_events.sock = sock1;
//    registered_entry->sock_events.events = malloc(sizeof(events_queue));
//    TAILQ_INIT(registered_entry->sock_events.events);
//    events_entry *e_entry = (events_entry*) malloc(sizeof(events_entry));
//    e_entry->event = malloc(sizeof(event_sock_read));
//    e_entry->event->event.socket = sock1;
//    e_entry->event->event.type = SOCK_READ;
//    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, e_entry, entries);
//    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);
//
////    // Создаем список событий для сокета 2
//    registered_entry = malloc(sizeof(registered_events_entry));
//    registered_entry->sock_events.sock = sock2;
//    registered_entry->sock_events.events = malloc(sizeof(events_queue));
//    TAILQ_INIT(registered_entry->sock_events.events);
//    e_entry = malloc(sizeof(events_entry));
//    e_entry->event = malloc(sizeof(event_sock_read));
//    e_entry->event->event.socket = sock2;
//    e_entry->event->event.type = SOCK_READ;
//    ((event_sock_read*)(e_entry->event))->handler = NULL;
//    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, e_entry, entries);
//    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);
////
////    // Создаем список событий для сокета 3
//    socket_entry  *se = malloc(sizeof(socket_entry));
//    se->socket = sock3;
//    se->handler = NULL;
//    TAILQ_INSERT_TAIL(el._sockets_accepts, se, entries);

//
//    _create_pollin_event(&el, fd_array, 0, 3);
//    _create_pollin_event(&el, fd_array, 1, 3);
//    _create_pollin_event(&el, fd_array, 2, 3);
//    int size = 0;
//    QUEUE_SIZE(occurred_event_entry, el._event_queue, entries, &size);
//    CU_ASSERT(size == 3)
//    event_t *res[3] = {0};
//    occurred_event_entry *ptr = NULL;
//    int i = 0;
//    TAILQ_FOREACH(ptr, el._event_queue, entries) {
//        res[i] = ptr->element.event;
//        i++;
//    }
//    CU_ASSERT(res[0]->event.type == SOCK_READ);
//    CU_ASSERT(res[1]->event.type == SOCK_READ);
//    CU_ASSERT(res[2]->event.type == SOCK_ACCEPT);
//
//    CU_ASSERT(res[0]->event.socket == sock1);
//    CU_ASSERT(res[1]->event.socket == sock2);
//    CU_ASSERT(res[2]->event.socket == sock3);
//
//    registered_events_entry *re_ptr = NULL;
//    TAILQ_FOREACH(re_ptr, el._sock_events, entries) {
//        events_entry *ee_ptr = NULL;
//        TAILQ_FOREACH(ee_ptr, re_ptr->sock_events.events, entries) {
//            free(ee_ptr->event);
//        }
//    }
//    while (!TAILQ_EMPTY(el._sock_events)) {
//        registered_events_entry *ptr = TAILQ_FIRST(el._sock_events);
//        TAILQ_REMOVE(el._sock_events, ptr, entries);
//        free(ptr);
//    }
//
//    while (!TAILQ_EMPTY(el._sockets_accepts)) {
//        socket_entry *ptr = TAILQ_FIRST(el._sockets_accepts);
//        TAILQ_REMOVE(el._sockets_accepts, ptr, entries);
//        free(ptr);
//    }
  //  el_close(&el);

}

void create_pollout_occurred_events_test() {

}
void process_sock_read_event_test() {

}
void process_sock_write_event_test() {

}
void process_sock_accept_event_test() {

}