#include <CUnit/Basic.h>
#include "maildir_tests.h"
#include "maildir/maildir.h"
#include "config/config.h"

int maildir_test_init() {
    config_context.logs_on = 0;
    return 0;
}

int maildir_test_clean() {
    return 0;
}

void maildir_init_test() {
    maildir_main *maildir = NULL;
    CU_ASSERT_PTR_NULL(maildir);
    maildir = init_maildir("./test/resources/maildir");
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_PTR_NOT_NULL(maildir->directory);
    finalize_maildir(maildir);
}

void update_maildir_test() {
    maildir_main *maildir = NULL;
    CU_ASSERT_PTR_NULL(maildir);
    maildir = init_maildir("./test/resources/maildir");
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_NOT_EQUAL(maildir->directory, NULL);
    update_maildir(maildir);
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_EQUAL(maildir->users_size, 3);
    CU_ASSERT_PTR_NOT_NULL(maildir->users);
    CU_ASSERT_EQUAL(maildir->users[0].messages_size, 2);
    CU_ASSERT_PTR_NOT_NULL(maildir->servers);
    CU_ASSERT_EQUAL(maildir->servers_size, 2);
    CU_ASSERT_EQUAL(maildir->servers[0].messages_size, 2);
    CU_ASSERT_EQUAL(maildir->servers[1].messages_size, 1);

    finalize_maildir(maildir);
}

void read_maildir_servers_test() {
    maildir_main *maildir = NULL;
    CU_ASSERT_EQUAL(maildir, NULL)
    maildir = init_maildir("./test/resources/maildir");
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_PTR_NOT_NULL(maildir->directory);
    read_maildir_servers(maildir);
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_EQUAL(maildir->users_size, 0);
    CU_ASSERT_PTR_NULL(maildir->users);
    CU_ASSERT_EQUAL(maildir->servers_size, 2);
    CU_ASSERT_PTR_NOT_NULL(maildir->servers);
    CU_ASSERT_EQUAL(maildir->servers[0].messages_size, 2);
    CU_ASSERT_EQUAL(maildir->servers[1].messages_size, 1);

    finalize_maildir(maildir);
}

void get_first_message_test() {
    maildir_main *maildir = NULL;
    CU_ASSERT_PTR_NULL(maildir);
    maildir = init_maildir("./test/resources/maildir");
    CU_ASSERT_PTR_NOT_NULL(maildir);
    CU_ASSERT_PTR_NOT_NULL(maildir->directory);
    read_maildir_servers(maildir);
    CU_ASSERT_EQUAL(maildir->servers_size, 2);
    CU_ASSERT_PTR_NOT_NULL(maildir->servers);
    CU_ASSERT_EQUAL(maildir->servers[0].messages_size, 2);
    message *mes = get_first_message(&maildir->servers[0]);
    CU_ASSERT_NOT_EQUAL(mes, NULL);
    CU_ASSERT_PTR_NOT_NULL(mes->directory);
    CU_ASSERT_PTR_NOT_NULL(mes->addresses);
    CU_ASSERT_STRING_EQUAL(mes->addresses[0], "postman.local");
    CU_ASSERT_STRING_EQUAL(mes->from, "vladovchinnikov950@gmail.com");
    CU_ASSERT_PTR_NOT_NULL(mes->to);
    CU_ASSERT_STRING_EQUAL(mes->to[0], "admin@postman.local");

    free_message(mes);
    finalize_maildir(maildir);
}

void read_message_test() {
    message *mes = read_message("./test/resources/maildir/user1/new/1608921467_1540383426");

    CU_ASSERT_NOT_EQUAL(mes, NULL);
    CU_ASSERT_PTR_NOT_NULL(mes->directory);
    CU_ASSERT_PTR_NOT_NULL(mes->addresses);
    CU_ASSERT_STRING_EQUAL(mes->addresses[0], "postman.local");
    CU_ASSERT_STRING_EQUAL(mes->from, "vladovchinnikov950@gmail.com");
    CU_ASSERT_PTR_NOT_NULL(mes->to);
    CU_ASSERT_STRING_EQUAL(mes->to[0], "admin@postman.local");

    free_message(mes);
}
