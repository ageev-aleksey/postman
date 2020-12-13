#include "logs.h"

TAILQ_HEAD(logs_queue, node) head;

void init_logs() {
    TAILQ_INIT(&head);
}

void push_log(log value) {
    node *new = malloc(sizeof(node));
    new->data = value;
    TAILQ_INSERT_TAIL(&head, new, nodes);
}

log pop_log() {
    node *old = TAILQ_FIRST(&head);
    TAILQ_REMOVE(&head, old, nodes);
    log data = old->data;
    free(old);
    return data;
}

bool is_logs_queue_empty() {
    return TAILQ_EMPTY(&head);
}

struct tm* get_time() {
    time_t t;
    struct tm *tm;
    t = time(NULL);
    tm = localtime(&t);

    return tm;
}

_Noreturn void *print_message() {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 500000;

    while (1) {
        nanosleep(&ts, &ts);
        if (!is_logs_queue_empty()) {
            log l = pop_log();

            char buffer[26];
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &l.time);

            switch (l.type) {
                case LOG_INFO:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_BLUE "%s    " COLOR_CYAN " [%s] [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "INFO", l.thread, l.filename, l.line, l.message);
                    break;
                case LOG_ERROR:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_RED "%s   " COLOR_CYAN " [%s] [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "ERROR", l.thread, l.filename, l.line, l.message);
                    break;
                case LOG_DEBUG:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_GREEN "%s   " COLOR_CYAN " [%s] [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "DEBUG", l.thread, l.filename, l.line, l.message);
                    break;
                case LOG_WARN:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_YELLOW "%s " COLOR_CYAN " [%s] [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "WARNING", l.thread, l.filename, l.line, l.message);
                    break;
            }
            //free(l.thread);
        }
    }
}

void log_debug(char *message, char *filename, int line) {
    log l = { NULL, NULL, NULL, line, LOG_DEBUG };
    struct tm *tm = get_time();
    l.time = *tm;
    l.message = message;
    l.filename = filename;
    push_log(l);
}

void log_info(char *message, char *filename, int line) {
    log l = { NULL, NULL, NULL, line, LOG_INFO };
    struct tm *tm = get_time();
    l.time = *tm;
    l.message = message;
    l.filename = filename;

    pthread_t self = pthread_self();

    l.thread = malloc(50);
    sprintf(l.thread, "Thread %lu", (unsigned long int) (self));

    push_log(l);
}

void log_error(char *message, char *filename, int line) {
    log l = { NULL, NULL, NULL, line, LOG_ERROR };
    struct tm *tm = get_time();
    l.time = *tm;
    l.message = message;
    l.filename = filename;
    push_log(l);
}

void log_warn(char *message, char *filename, int line) {
    log l = { NULL, NULL, NULL, line, LOG_WARN };
    struct tm *tm = get_time();
    l.time = *tm;
    l.message = message;
    l.filename = filename;
    push_log(l);
}

void start_logger() {
    pthread_t thread_logger;

    init_logs();
    pthread_create(&thread_logger, NULL, print_message, NULL);
}

