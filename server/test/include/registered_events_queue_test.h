//
// Created by nrx on 10.10.2020.
//

#ifndef SERVER_REGISTERED_EVENTS_TEST_H
#define SERVER_REGISTERED_EVENTS_TEST_H

int registered_events_queue_test_init();
int registered_events_queue_test_clean();

void registered_events_queue_push_accept_test();
void registered_events_queue_push_read_test();
void registered_events_queue_push_write_test();

void registered_events_queue_pop_accept_test();
void registered_events_queue_pop_read_test();
void registered_events_queue_pop_write_test();

void registered_events_queue_bitmask_test();

#endif //SERVER_REGISTERED_EVENTS_TEST_H
