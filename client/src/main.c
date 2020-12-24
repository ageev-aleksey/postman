#include "util/util.h"
#include "log/logs.h"
#include "config/config.h"
#include "context/context.h"

int main(int argc, char **argv) {
    start_logger();
    LOG_INFO("Начало работы MDA", NULL);
    if (!loading_config()) {
        destroy_configuration();
        logger_finalize();
        return -1;
    }
    init_signals_handler();
    init_context();

    while (true) {
        if (is_interrupt()) {
            LOG_INFO("Остановка MDA", NULL);
            destroy_context();
            destroy_configuration();
            logger_finalize();
            return 0;
        }
    }
}