#include "logs.h"
#include "config/config.h"
#include "smtp-client.h"

int main(int argc, char **argv) {
    start_logger();
    if (!loading_config()) {
        return -1;
    }

//    smtp_message **smtp_message = malloc(sizeof **smtp_message);
//
//    smtp_open("localhost", "8080", smtp_message);
//
//    free(smtp_message);

    for (int i = 0; i < 100; i++) {
        LOG_DEBUG("This is debug");
        LOG_INFO("This is info");
        LOG_ERROR("This is error");
        LOG_WARN("This is warning");
    }
}


