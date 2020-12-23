#include "util.h"
#include "logs.h"
#include "config.h"
#include "maildir.h"
#include "message_queue.h"
#include "smtp.h"
#include "context.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы SMTP-клиента", NULL);
    if (!loading_config()) {
        return -1;
    }
    init_signals_handler();
   // start_message_queue();
    init_context();
//    smtp_context *context = smtp_connect("yandex.ru", "25", NULL);
//    smtp_response response = get_smtp_response(context);
//    smtp_send_helo(context);
//    response = get_smtp_response(context);
//    smtp_send_mail(context, "vladovchinnikov950@gmail.com");
//    response = get_smtp_response(context);
//    smtp_send_rcpt(context, "wedf97@yandex.ru");
//    response = get_smtp_response(context);
//    smtp_send_data(context);
//    response = get_smtp_response(context);
//    smtp_send_message(context, "Subject: Hello\r\n");
//    smtp_send_message(context, "From: vladovchinnikov950@gmail.com\r\n");
//    smtp_send_message(context, "To: wedf97@gmail.com\r\n");
//    smtp_send_message(context, "\r\n");
//    smtp_send_message(context, "Hello, world");
//    smtp_send_end_message(context);
//    response = get_smtp_response(context);
//    smtp_send_quit(context);
//    response = get_smtp_response(context);
//    smtp_context **contexts = malloc(sizeof **contexts);
//    smtp_context *context = smtp_open("mx.yandex.ru", "25", contexts);
//
//    if (context != NULL && context->state_code == OK) {
//        smtp_mail(context, "vladovchinnikov950@gmail.com", "Vladislav Ovchinnikov");
//        smtp_rcpt(context, "wedf97@yandex.ru", "Vladislav Ovchinnikov");
//        smtp_data(context, "Hello, Vladislav.\r\n.\r\n");
//        smtp_quit(context);
//    }

    while (true) {}

}