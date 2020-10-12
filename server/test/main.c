#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "event_loop_test.h"
#include "events_queue_test.h"
#include "registered_events_queue_test.h"
bool eq_test_init();
bool req_test_init();
bool el_test_init();
int main()
{
    CU_pSuite pSuite = NULL;
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    /* add a suite to the registry */

    if (!eq_test_init() ||
        !req_test_init() ||
        !el_test_init())
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}


bool eq_test_init() {
    CU_pSuite pSuite = NULL;
    pSuite = CU_add_suite("EventsQueue", event_queue_test_init, event_queue_test_clean);
    if (NULL == pSuite) {
        return false;
    }
    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite,
                             "add element in queue",
                             event_queue_push_test)) ||
        (NULL == CU_add_test(pSuite,
                             "add two elements of equals type in queue",
                             event_queue_double_push_test)) ||
        (NULL == CU_add_test(pSuite,
                             "pop element from queue",
                             event_queue_pop_test)) ||
        (NULL == CU_add_test(pSuite,
                             "pop not existing element from queue",
                             event_queue_empty_pop_test)))
    {
        return false;
    }
    return true;
}

bool req_test_init() {
    CU_pSuite pSuite = NULL;
    pSuite = CU_add_suite("RegisteredEventsQueue", registered_events_queue_test_init, registered_events_queue_test_clean);
    if (NULL == pSuite) {
        return false;
    }
    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite,
                             "push element accept type in queue",
                             registered_events_queue_push_accept_test)) ||
         (NULL == CU_add_test(pSuite,
                              "push element read type in queue",
                              registered_events_queue_push_read_test)) ||
         (NULL == CU_add_test(pSuite,
                              "push element write type in queue",
                               registered_events_queue_push_write_test)) ||
         (NULL == CU_add_test(pSuite,
                              "pop element accept type in queue",
                              registered_events_queue_pop_accept_test)) ||
         (NULL == CU_add_test(pSuite,
                              "pop element read type in queue",
                              registered_events_queue_pop_read_test)) ||
         (NULL == CU_add_test(pSuite,
                              "pop element write type in queue",
                              registered_events_queue_pop_write_test)) ||
         NULL == CU_add_test(pSuite,
                             "get bitmask of registered events for socket",
                             registered_events_queue_bitmask_test))
    {
        return false;
    }
    return true;
}

bool el_test_init() {
    CU_pSuite pSuite = NULL;
    pSuite = CU_add_suite("EventLoop", event_loop_test_init, event_loop_test_clean);
    if (NULL == pSuite) {
        return false;
    }

    if (NULL == CU_add_test(pSuite,
                         "test init event loop",
                         event_loop_initialize_test) ||
        NULL == CU_add_test(pSuite,
                            "create pollfd from event_loop structure",
                            create_pollfd_array_test) ||
        NULL == CU_add_test(pSuite,
                            "process pollin",
                            create_pollin_occurred_events_test))
    {
        return false;
    }
    return true;
}
