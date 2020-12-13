#include "logs.h"
#include "config/config.h"
#include "smtp-client.h"

int main(int argc, char **argv) {
    if (!loading_config()) {
        return -1;
    }
//    smtp_message **smtp_message = malloc(sizeof **smtp_message);
//
//    smtp_open("localhost", "8080", smtp_message);
//
//    free(smtp_message);

    init_logs();

    pthread_t thread;

    pthread_create(&thread, NULL, print_message, NULL);


    for (int i = 0; i < 1000; i++) {
        LOG_DEBUG("This is debug");
        LOG_INFO("This is info");
        LOG_ERROR("This is error");
        LOG_WARN("This is warning");
    }

    pthread_join(thread, NULL);
}


