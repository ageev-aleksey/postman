#include <CUnit/Basic.h>
#include "config_tests.h"
#include "config/config.h"

int config_test_init() {
    config_context.threads = 0;
    config_context.maildir.path = NULL;
    config_context.server_port = NULL;
    config_context.hostname = NULL;
    config_context.debug = 0;
    config_context.logs_on = 0;
    return 0;
}

int config_test_clean() {
    destroy_configuration();
    return 0;
}

void loading_config_test() {
    CU_ASSERT_EQUAL(config_context.threads, 0);
    CU_ASSERT_EQUAL(config_context.debug, 0);
    CU_ASSERT_PTR_NULL(config_context.hostname);
    CU_ASSERT_PTR_NULL(config_context.server_port);
    CU_ASSERT_PTR_NULL(config_context.maildir.path);

    loading_config();

    CU_ASSERT_NOT_EQUAL(config_context.threads, 0);
    CU_ASSERT_PTR_NOT_NULL(config_context.hostname);
    CU_ASSERT_PTR_NOT_NULL(config_context.server_port);
    CU_ASSERT_PTR_NOT_NULL(config_context.maildir.path);
}

void destroy_configuration_test() {
    CU_ASSERT_NOT_EQUAL(config_context.threads, 0);
    CU_ASSERT_PTR_NOT_NULL(config_context.hostname);
    CU_ASSERT_PTR_NOT_NULL(config_context.server_port);
    CU_ASSERT_PTR_NOT_NULL(config_context.maildir.path);

    destroy_configuration();

    CU_ASSERT_EQUAL(config_context.threads, 0);
    CU_ASSERT_EQUAL(config_context.debug, 0);
    CU_ASSERT_PTR_NULL(config_context.hostname);
    CU_ASSERT_PTR_NULL(config_context.server_port);
    CU_ASSERT_PTR_NULL(config_context.maildir.path);
}