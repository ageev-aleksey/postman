#include "server/users_list.h"
#include "server/server.h"
#include "server_test.h"
#include "log/context.h"
#include <string.h>
#include "CUnit/Basic.h"


int server_test_init() {
    server_config.self_server_name = "postman.local";
    maildir_init(&server_config.md, "./maildir_test", NULL);
    smtp_lib_init();
    LOG_INIT();
    return 0;
}

int server_test_clean() {
    smtp_lib_free();
    maildir_free(&server_config.md);
    LOG_FREE();
    return 0;
}

extern struct pair handler_smtp(user_context *user, char *message);
void server_handler_smtp_test() {
    user_context user;
    user.addr.port = 0;
    user.addr.ip[0] = '\0';
    smtp_init(&user.smtp, NULL);
    handler_smtp(&user, "ehlo [127.0.0.1]\r\n");
    handler_smtp(&user, "mail from: <test@test.ru>\r\n");
    handler_smtp(&user, "rcpt to: <user@test.ru>\r\n");
    handler_smtp(&user, "rcpt to: <client@yandex.ru>\r\n");
    handler_smtp(&user, "rcpt to: <mail@mail.ru>\r\n");
    handler_smtp(&user, "rcpt to: <client@postman.local>\r\n");
    handler_smtp(&user, "data\r\n");
    handler_smtp(&user, "Subject: test mail\r\n");
    handler_smtp(&user, "\r\n");
    handler_smtp(&user, "Test message\r\n");
    handler_smtp(&user, "test tes\r\n");
    handler_smtp(&user, ".\r\n");
    smtp_free(&user.smtp);
}

void server_test_sub_str_iterator() {
    const char* str = "12345\r\n123456\r\n12\r\n\r\n00000000";
    struct sub_str_iterator itr;
    itr.end = 0;
    itr.begin = 0;
    itr.sep = "\r\n";
    itr.sep_len = 2;
    itr.str = str;
    itr.str_len = strlen(str);
    sub_str_iterate(&itr);
    CU_ASSERT_EQUAL(itr.begin, 0);
    CU_ASSERT_EQUAL(itr.end, 7);

    sub_str_iterate(&itr);
    CU_ASSERT_EQUAL(itr.begin, 7);
    CU_ASSERT_EQUAL(itr.end, 15);

    sub_str_iterate(&itr);
    CU_ASSERT_EQUAL(itr.begin, 15);
    CU_ASSERT_EQUAL(itr.end, 19);

    sub_str_iterate(&itr);
    CU_ASSERT_EQUAL(itr.begin, 19);
    CU_ASSERT_EQUAL(itr.end, 21);

    sub_str_iterate(&itr);
    CU_ASSERT_EQUAL(itr.begin, 21);
    CU_ASSERT_EQUAL(itr.end, strlen(str));
}