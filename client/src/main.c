#include "config/config.h"
#include "smtp-client.h"
#include "util.h"

int main(int argc, char **argv) {
    if (!loading_config()) {
        return -1;
    }

    smtp_message **smtp_message = malloc(sizeof **smtp_message);

    smtp_open("mx.yandex.ru", "25", smtp_message);

    free(smtp_message);
}


