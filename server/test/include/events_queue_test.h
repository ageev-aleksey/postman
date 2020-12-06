//
// Created by nrx on 09.10.2020.
//

#ifndef SERVER_EVENTS_QUEUE_TEST_H
#define SERVER_EVENTS_QUEUE_TEST_H

#include "event_loop/events_queue.h"

int event_queue_test_init();
int event_queue_test_clean();

void event_queue_push_test();
void event_queue_double_push_test();
void event_queue_pop_test();
void event_queue_empty_pop_test();


#endif //SERVER_EVENTS_QUEUE_TEST_H
