#include "CUnit/Basic.h"
#include "event_loop/event_loop.h"
#include "event_loop_test.h"
#include <stdlib.h>



int event_loop_test_init() {
   return 0;
}

int event_loop_test_clean() {
   return 0;
}

void create_pollfd_array_test() {
//    static event_loop el;
//    el_init(&el);
//
//    registered_events_entry *registered_entry = malloc(sizeof(registered_events_entry));
//    registered_entry->sock_events.sock = 1;
//    registered_entry->sock_events.events = malloc(sizeof(events_queue));
//    TAILQ_INIT(registered_entry->sock_events.events);
//
//    events_entry *entry = malloc(sizeof(events_entry));
//    entry->event = malloc(sizeof(event_t));
//    entry->event->event.socket = 1;
//    entry->event->event.type = SOCK_READ;
//    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, entry, entries);
//
//    entry = malloc(sizeof(events_entry));
//    entry->event = malloc(sizeof(event_t));
//    entry->event->event.socket = 1;
//    entry->event->event.type = SOCK_WRITE;
//    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, entry, entries);
//
//    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);
//
//    registered_entry = malloc(sizeof(registered_events_entry));
//    registered_entry->sock_events.sock = 2;
//    registered_entry->sock_events.events = malloc(sizeof(events_queue));
//    TAILQ_INIT(registered_entry->sock_events.events);
//
//    entry = malloc(sizeof(events_entry));
//    entry->event = malloc(sizeof(event_t));
//    entry->event->event.socket = 1;
//    entry->event->event.type = SOCK_WRITE;
//    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, entry, entries);
//
//    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);
//
//    struct pollfd *fd_array = NULL;
//    int size;
//    _create_pollfd(&el, &fd_array, &size);
//
//    CU_ASSERT(size == 2)
//    CU_ASSERT(fd_array[0].fd == 1)
//    CU_ASSERT(fd_array[0].events == (POLLIN | POLLOUT));
//    CU_ASSERT(fd_array[1].fd == 2)
//    CU_ASSERT(fd_array[1].events == POLLOUT);
//
//    free(fd_array);
//
//    el_close(&el);

}

void create_pollin_occurred_events_test() {
    event_loop el;
    el_init(&el);

    int sock1 = 1;
    int sock2 = 2;
    int sock3 = 3;
    struct pollfd fd_array[] = {
            {sock1, POLLIN, 0},
            {sock2, POLLIN, 0},
            {sock3, POLLIN, 0}
    };
    el._index_acepptors_start = 2; // последний элемент fd_array относится к событию принятия подключения от клиента

    registered_events_entry *registered_entry = (registered_events_entry*)malloc(sizeof(registered_events_entry));

     //Создаем список событий для сокета 1
    registered_entry->sock_events.sock = sock1;
    registered_entry->sock_events.events = malloc(sizeof(events_queue));
    TAILQ_INIT(registered_entry->sock_events.events);
    events_entry *e_entry = (events_entry*) malloc(sizeof(events_entry));
    e_entry->event = malloc(sizeof(event_sock_read));
    e_entry->event->event.socket = sock1;
    e_entry->event->event.type = SOCK_READ;
    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, e_entry, entries);
    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);

//    // Создаем список событий для сокета 2
    registered_entry = malloc(sizeof(registered_events_entry));
    registered_entry->sock_events.sock = sock2;
    registered_entry->sock_events.events = malloc(sizeof(events_queue));
    TAILQ_INIT(registered_entry->sock_events.events);
    e_entry = malloc(sizeof(events_entry));
    e_entry->event = malloc(sizeof(event_sock_read));
    e_entry->event->event.socket = sock2;
    e_entry->event->event.type = SOCK_READ;
    ((event_sock_read*)(e_entry->event))->handler = NULL;
    TAILQ_INSERT_TAIL(registered_entry->sock_events.events, e_entry, entries);
    TAILQ_INSERT_TAIL(el._sock_events, registered_entry, entries);
//
//    // Создаем список событий для сокета 3
    socket_entry  *se = malloc(sizeof(socket_entry));
    se->socket = sock3;
    se->handler = NULL;
    TAILQ_INSERT_TAIL(el._sockets_accepts, se, entries);

//
    _create_pollin_event(&el, fd_array, 0, 3);
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
    el_close(&el);

}

void create_pollout_occurred_events_test() {

}
void process_sock_read_event_test() {

}
void process_sock_write_event_test() {

}
void process_sock_accept_event_test() {

}