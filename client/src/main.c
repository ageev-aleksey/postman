#include "smtp-client.h"
#include "util.h"
#include "logs.h"
#include "config.h"
#include "maildir.h"
#include "message_queue.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы SMTP-клиента", NULL);
    if (!loading_config()) {
        return -1;
    }
    init_signals_handler();
    start_message_queue();

//    smtp_context **contexts = malloc(sizeof **contexts);
//    smtp_context *context = smtp_open("mx.yandex.ru", "25", contexts);
//
//    if (context != NULL && context->state_code == OK) {
//        smtp_mail(context, "vladovchinnikov950@gmail.com", "Vladislav Ovchinnikov");
//        smtp_rcpt(context, "wedf97@yandex.ru", "Vladislav Ovchinnikov");
//        smtp_data(context, "Hello, Vladislav.\r\n.\r\n");
//        smtp_quit(context);
//    }

    while (1) {}

}