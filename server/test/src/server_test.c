#include "server/users_list.h"
#include "server/global_context.h"
#include "server_test.h"
#include "log/context.h"


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

extern char *handler_smtp(user_context *user, char *message);
void server_handler_smtp_test() {
    user_context user;
    user.addr.port = 0;
    user.addr.ip[0] = '\0';
    smtp_init(&user.smtp, NULL);
    handler_smtp(&user, "ehlo [127.0.0.1]\r\n");
    handler_smtp(&user, "mail from: <test@test.ru>\r\n");
    handler_smtp(&user, "rcpt to: <user@test.ru>\r\n");
    handler_smtp(&user, "rcpt to: <client@postman.local>\r\n");
    handler_smtp(&user, "data\r\n");
    handler_smtp(&user, "Subject: test mail\r\n");
    handler_smtp(&user, "\r\n");
    handler_smtp(&user, "Test message\r\n");
    handler_smtp(&user, "test tes\r\n");
    handler_smtp(&user, ".\r\n");
    smtp_free(&user.smtp);
}