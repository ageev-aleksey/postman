#include "smtp-client.h"
#include "util.h"
#include "logs.h"
#include "config.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы SMTP-клиента");
    if (!loading_config()) {
        return -1;
    }

    smtp_context **contexts = malloc(sizeof **contexts);
    smtp_context *context = smtp_open("mail.success.xyz", "25", contexts);

    if (context->state_code == OK) {
        smtp_mail(context, "vladovchinnikov950@gmail.com", "Vladislav Ovchinnikov");
        smtp_addr to_addrs[1];
        to_addrs[0].email = "wedf97@yandex.ru";
        to_addrs[0].name = "Vladislav Ovchinnikov";
        smtp_rcpt(context, to_addrs, 1);
        smtp_data(context, "Hello, Vladislav.\r\n.\r\n");
    }

    while (1) {

    }

}


