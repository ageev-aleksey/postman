#include <CUnit/Basic.h>
#include "logs_tests.h"
#include "log/logs.h"
#include "config/config.h"

int logs_test_init() {
    init_logs();
    config_context.logs_on = 1;
    config_context.debug = 1;
    return 0;
}

int logs_test_clean() {
    return 0;
}

void log_debug_test() {
    LOG_DEBUG("Test %s", "debug");
    log_message *l = pop_log();
    CU_ASSERT_STRING_EQUAL(l->message, "Test debug");
    CU_ASSERT_EQUAL(l->type, LOG_DEBUG);
    free(l->message);
    free(l->thread);
    free(l);
}

void log_info_test() {
    init_logs();
    LOG_INFO("Test %s", "info");
    log_message *l = pop_log();
    CU_ASSERT_STRING_EQUAL(l->message, "Test info");
    CU_ASSERT_EQUAL(l->type, LOG_INFO);
    free(l->message);
    free(l->thread);
    free(l);
}

void log_error_test() {
    init_logs();
    LOG_ERROR("Test %s", "error");
    log_message *l = pop_log();
    CU_ASSERT_STRING_EQUAL(l->message, "Test error");
    CU_ASSERT_EQUAL(l->type, LOG_ERROR);
    free(l->message);
    free(l->thread);
    free(l);
}

void log_warn_test() {
    init_logs();
    LOG_WARN("Test %s", "warn");
    log_message *l = pop_log();
    CU_ASSERT_STRING_EQUAL(l->message, "Test warn");
    CU_ASSERT_EQUAL(l->type, LOG_WARN);
    free(l->message);
    free(l->thread);
    free(l);
}

void log_addinfo_test() {
    init_logs();
    LOG_ADDINFO("Test %s", "addinfo");
    log_message *l = pop_log();
    CU_ASSERT_STRING_EQUAL(l->message, "Test addinfo");
    CU_ASSERT_EQUAL(l->type, LOG_ADDINFO);
    free(l->message);
    free(l->thread);
    free(l);
}

