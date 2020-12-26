#include <CUnit/Basic.h>
#include "smtp_tests.h"
#include "smtp/smtp.h"

int smtp_test_init() {
    return 0;
}

int smtp_test_clean() {
    return 0;
}

void is_smtp_success_test() {
    status_code status = SMTP_SUCCESS;
    CU_ASSERT_TRUE(is_smtp_success(status));
}

void is_smtp_4xx_error_test() {
    status_code status = SMTP_SERVER_IS_NOT_AVAILABLE;
    CU_ASSERT_TRUE(is_smtp_4xx_error(status));
}

void is_smtp_5xx_error_test() {
    status_code status = SMTP_MAILBOX_NAME_IS_INVALID;
    CU_ASSERT_TRUE(is_smtp_5xx_error(status));
}

void smtp_connect_test() {

}

void smtp_send_helo_test() {

}

void smtp_send_mail_test() {

}

void smtp_send_rcpt_test() {

}

void smtp_send_data_test() {

}

void smtp_send_message_test() {

}

void smtp_send_end_message_test() {

}

void smtp_send_rset_test() {

}

void smtp_send_quit_test() {

}

void get_smtp_response_test() {

}