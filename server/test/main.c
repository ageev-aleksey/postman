#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
//#include "event_loop_test.h"
#include "events_queue_test.h"
int init_suite1(void)
{
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
    return 0;
}

/* Simple test of fprintf().
 * Writes test data to the temporary file and checks
 * whether the expected number of bytes were written.
 */
void testFPRINTF(void)
{
//    int i1 = 10;
//
//    if (NULL != temp_file) {
//
//       // CU_ASSERT(0 == fprintf(temp_file, ""));
//        CU_ASSERT(2 == fprintf(temp_file, "Q\n"));
//        CU_ASSERT(7 == fprintf(temp_file, "i1 = %d", i1));
//    }
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void testFREAD(void)
{
//    unsigned char buffer[20];
//
//    if (NULL != temp_file) {
//        rewind(temp_file);
//        CU_ASSERT(9 == fread(buffer, sizeof(unsigned char), 20, temp_file));
//        CU_ASSERT(0 == strncmp(buffer, "Q\ni1 = 10", 9));
//    }
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */

int main()
{
    CU_pSuite pSuite = NULL;
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    /* add a suite to the registry */
    pSuite = CU_add_suite("Event Loop", event_queue_test_init, event_queue_test_clean);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
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
        CU_cleanup_registry();
        return CU_get_error();
    }


    /* add a suite to the registry */
//    pSuite = CU_add_suite("Event Loop", event_loop_test_init, event_loop_test_clean);
//    if (NULL == pSuite) {
//        CU_cleanup_registry();
//        return CU_get_error();
//    }

    /* add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
//    if ((NULL == CU_add_test(pSuite,
//                             "test of creating pollfd array from list of registered events",
//                             create_pollfd_array_test)) ||
//        NULL == CU_add_test(pSuite,
//                            "test of adding in queue info about pollin event",
//                            create_pollin_occurred_events_test) ||
//        NULL == CU_add_test(pSuite,
//                            "test of adding in queue info about pollout event",
//                            create_pollout_occurred_events_test) ||
//        NULL == CU_add_test(pSuite,
//                            "test of processing of sock read event",
//                            process_sock_read_event_test) ||
//        NULL == CU_add_test(pSuite,
//                            "test of processing of sock write event",
//                            process_sock_write_event_test) ||
//        NULL == CU_add_test(pSuite,
//                            "test of processing of sock accept event",
//                            process_sock_accept_event_test))
//    {
//        CU_cleanup_registry();
//        return CU_get_error();
//    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

