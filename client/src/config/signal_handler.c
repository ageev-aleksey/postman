#include <signal.h>
#include "context.h"
#include "logs.h"

void exit_handler(int sig) {
    LOG_INFO("Получен сигнал %d. Завершение работы программы и освобождение всех ресурсов", sig);
    struct timespec timespec;
    timespec.tv_nsec = 0;
    timespec.tv_sec = 10;
    nanosleep(&timespec, &timespec);

    interrupt_thread_local = 1;
    logger_finalize();
}

int init_signals_handler() {
    struct sigaction act = { 0 };
    act.sa_handler = exit_handler;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT);
    sigaddset(&act.sa_mask, SIGKILL);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaction(SIGTERM, &act, 0);
    LOG_INFO("Инициализация обработчиков сигналов", NULL);
}

