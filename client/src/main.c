#include "smtp-client.h"
#include "util.h"
#include "logs.h"
#include "config.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы SMTP-клиента", NULL);
    if (!loading_config()) {
        return -1;
    }

    smtp_context **contexts = malloc(sizeof **contexts);
    smtp_context *context = smtp_open("172.16.1.3", "25", contexts);

    if (context != NULL && context->state_code == OK) {
        smtp_mail(context, "vladovchinnikov950@gmail.com", "Vladislav Ovchinnikov");
        smtp_rcpt(context, "admin@test.ru", "Vladislav Ovchinnikov");
        smtp_data(context, "Hello, Vladislav.\r\n.\r\n");
        smtp_quit(context);
    }

    struct timespec ts;
    ts.tv_sec = 30;
    nanosleep(&ts, &ts);

}


