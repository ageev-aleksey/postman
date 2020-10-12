#include "CUnit/Basic.h"
#include "event_loop/event_loop.h"
#include "event_loop_test.h"
#include "util.h"
#include <stdlib.h>
#include <arpa/inet.h>

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
//    struct sockaddr_in addr;
//    memset(&addr, 0, sizeof(struct sockaddr_in));
//    addr.sin_family = AF_INET;
//    addr.sin_port = htonl(8081);
//    addr.sin_addr.s_addr = INADDR_ANY;
//    int socket1 = socket(AF_INET, SOCK_STREAM, 0);
//   // int er = bind(socket1, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
////    if (er == -1) {
////        CU_FAIL();
////    }
//    int socket2 = socket(AF_INET, SOCK_STREAM, 0);
//    int er = bind(socket2, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
//    if (er == -1) {
//        CU_FAIL();
//        return;
//    }
//    er = listen(socket2, 1);
//    if (er == -1) {
//        CU_FAIL();
//        return;
//    }
//    struct sockaddr_in addr2 = addr;
//    addr2.sin_addr.s_addr = INADDR_LOOPBACK;
//    er = connect(socket1, (struct sockaddr*)&addr2, sizeof(struct sockaddr));
//
//    if (er == -1) {
//        CU_FAIL();
//    }

    // РУЧНОЕ ТЕСТИРОВАНИЕ С ОТЛАДЧИКОМ
    int socket1 = 10;
    int socket2 = 20;

    error_t error;
    ERROR_SUCCESS(&error);
    event_loop *loop = el_init(&error);
    ERROR_ASSERT(error);

    loop->_index_acepptors_start = 1; // socket2 is acceptor
    sq_add(loop->_sockets_accepts, socket2, NULL, &error);
    event_sock_read *event = s_malloc(sizeof(event_sock_read), &error);
    ERROR_ASSERT(error);

    event->event.socket = socket1;
    event->event.type = SOCK_READ;
    event->size = 1;
    req_push_read(loop->_sock_events, socket1, event, &error);
    ERROR_ASSERT(error);

    struct pollfd fd_array[2] = {0};
    fd_array[0].revents = POLLIN;
    fd_array[0].events = POLLIN;
    fd_array[0].fd = socket1;
    fd_array[1].revents = POLLIN;
    fd_array[1].events = POLLIN;
    fd_array[1].fd = socket2;
    pr_create_pollin_event(loop, fd_array, 0, &error);
    ERROR_ASSERT(error);

    CU_ASSERT_FALSE(TAILQ_EMPTY(loop->_event_queue));
    occurred_event_entry *ptr = TAILQ_FIRST(loop->_event_queue);
    CU_ASSERT_EQUAL(ptr->element.event->event.type, SOCK_READ);
    CU_ASSERT_EQUAL(ptr->element.event->event.socket, socket1);

    pr_create_pollin_event(loop, fd_array, 1, &error);
    ERROR_ASSERT(error);
//
//    occurred_event_entry *ptr2 = TAILQ_LAST(loop->_event_queue, _occurred_event_queue);
//    CU_ASSERT_EQUAL(ptr2->element.event->event.type, SOCK_ACCEPT);
//    CU_ASSERT_EQUAL(ptr2->element.event->event.socket, socket2);

    el_close(loop);

}

void create_pollout_occurred_events_test() {

}
void process_sock_read_event_test() {

}
void process_sock_write_event_test() {

}
void process_sock_accept_event_test() {

}