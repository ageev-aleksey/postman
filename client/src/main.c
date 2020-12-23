#include "util/util.h"
#include "log/logs.h"
#include "config/config.h"
#include "context/context.h"

int main(int argc, char **argv) {
    start_logger();
    loading_config();
    LOG_INFO("Начало работы SMTP-клиента", NULL);
    if (!loading_config()) {
        return -1;
    }
    init_signals_handler();
    init_context();

    while (true) {
        if (is_interrupt()) {
            destroy_configuration();
            LOG_INFO("Сервер остановлен", NULL);
            logger_finalize();
            return 0;
        }
    }
}