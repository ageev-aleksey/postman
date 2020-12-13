#include "config.h"
#include "smtp-client.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы SMTP-клиента");
    if (!loading_config()) {
        return -1;
    }

//    smtp_message **smtp_message = malloc(sizeof **smtp_message);
//
//    smtp_open("localhost", "8080", smtp_message);
//
//    free(smtp_message);

    while (1) {

    }

}


