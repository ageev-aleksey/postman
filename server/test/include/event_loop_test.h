//
// Created by nrx on 27.09.2020.
//

#ifndef SERVER_EVENT_LOOP_TEST_H
#define SERVER_EVENT_LOOP_TEST_H

int event_loop_test_init();
int event_loop_test_clean();

void create_pollfd_array_test();
void create_pollin_occurred_events_test();
void create_pollout_occurred_events_test();
void process_sock_read_event_test();
void process_sock_write_event_test();
void process_sock_accept_event_test();

#endif //SERVER_EVENT_LOOP_TEST_H
