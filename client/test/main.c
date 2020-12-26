#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdbool.h>
#include "util_tests.h"
#include "config_tests.h"
#include "logs_tests.h"
#include "smtp_tests.h"
#include "maildir_tests.h"

bool util_tests_init();
bool smtp_tests_init();
bool logs_tests_init();
bool config_tests_init();
bool maildir_tests_init();

int main() {
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    if (!util_tests_init()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!smtp_tests_init()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!logs_tests_init()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!config_tests_init()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!maildir_tests_init()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();

}

bool util_tests_init() {
    CU_pSuite pSuite = CU_add_suite("UtilTests", util_test_init, util_test_clean);

    if (pSuite == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "convert string to long int", convert_string_to_long_int_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "split string", split_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "trim string", trim_test) == NULL) {
        return false;
    }

    return true;
}

bool config_tests_init() {
    CU_pSuite pSuite = CU_add_suite("ConfigTests", config_test_init, config_test_clean);

    if (pSuite == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "loading config", loading_config_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "destroy config", destroy_configuration_test) == NULL) {
        return false;
    }

    return true;
}

bool logs_tests_init() {
    CU_pSuite pSuite = CU_add_suite("LogsTests", logs_test_init, logs_test_clean);

    if (pSuite == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "log debug", log_debug_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "log info", log_info_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "log error", log_error_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "log warn", log_warn_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "log warn", log_addinfo_test) == NULL) {
        return false;
    }

    return true;
}

bool maildir_tests_init() {
    CU_pSuite pSuite = CU_add_suite("MaildirTests", maildir_test_init, maildir_test_clean);

    if (pSuite == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "maildir init", maildir_init_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "update maildir", update_maildir_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "read maildir servers", read_maildir_servers_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "get first message", get_first_message_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "read message", read_message_test) == NULL) {
        return false;
    }

    return true;
}

bool smtp_tests_init() {
    CU_pSuite pSuite = CU_add_suite("SMTPTests", smtp_test_init, smtp_test_clean);

    if (pSuite == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP connect", smtp_connect_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send helo", smtp_send_helo_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send mail", smtp_send_mail_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send rcpt", smtp_send_rcpt_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send data", smtp_send_data_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send message", smtp_send_message_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", smtp_send_end_message_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", smtp_send_rset_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", smtp_send_quit_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", get_smtp_response_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", is_smtp_success_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", is_smtp_4xx_error_test) == NULL) {
        return false;
    }

    if (CU_add_test(pSuite, "SMTP send end message", is_smtp_5xx_error_test) == NULL) {
        return false;
    }

    return true;
}