#include <signal.h>
#include "context/context.h"
#include "log/logs.h"
#include "config.h"

void exit_handler(int sig) {
    LOG_INFO("Получен сигнал %d. Завершение работы программы и освобождение всех ресурсов", sig);
    interrupt_thread_local = 1;
}

int init_signals_handler() {
    struct sigaction act = { 0 };
    act.sa_handler = exit_handler;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    LOG_INFO("Инициализация обработчиков сигналов", NULL);

    return 0;
}

