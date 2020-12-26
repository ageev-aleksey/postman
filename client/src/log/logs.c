#include <signal.h>
#include "util/util.h"
#include "config/config.h"
#include "logs.h"

// TODO: внимательно все проверить, отрефакторить, написать тесты

typedef struct node {
    log_message *data;
    TAILQ_ENTRY(node) nodes;
} node;

int interrupt_thread_local = 0;
pthread_t thread_logger = {0};
pthread_mutex_t mutex_queue;

TAILQ_HEAD(logs_queue, node) head;

void init_logs() {
    TAILQ_INIT(&head);
}

void push_log(log_message *value) {
    pthread_mutex_lock(&mutex_queue);
    node *new = allocate_memory(sizeof(node));
    new->data = value;
    TAILQ_INSERT_TAIL(&head, new, nodes);
    pthread_mutex_unlock(&mutex_queue);
}

log_message *pop_log() {
    pthread_mutex_lock(&mutex_queue);
    node *old = TAILQ_FIRST(&head);
    if (old == NULL) {
        return NULL;
    }
    TAILQ_REMOVE(&head, old, nodes);
    log_message *data = old->data;
    free(old);
    pthread_mutex_unlock(&mutex_queue);
    return data;
}

bool is_logs_queue_empty() {
    return TAILQ_EMPTY(&head);
}

struct tm get_time() {
    time_t t = time(NULL);;
    struct tm *tm = localtime(&t);
    return *tm;
}

void print_message(log_message *l) {
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &l->time);

    switch (l->type) {
        case LOG_INFO:
            printf(COLOR_MAGENTA "[%s] " COLOR_BLUE "%s    " COLOR_CYAN " [%s] [%s:%d]: " " %s\n",
                   buffer, "INFO", l->thread, l->filename, l->line, l->message);
            break;
        case LOG_ERROR:
            printf(COLOR_MAGENTA "[%s] " COLOR_RED "%s   " COLOR_CYAN " [%s] [%s:%d]: " " %s\n",
                   buffer, "ERROR", l->thread, l->filename, l->line, l->message);
            break;
        case LOG_DEBUG:
            printf(COLOR_MAGENTA "[%s] " COLOR_GREEN "%s   " COLOR_CYAN " [%s] [%s:%d]: " " %s\n",
                   buffer, "DEBUG", l->thread, l->filename, l->line, l->message);
            break;
        case LOG_WARN:
            printf(COLOR_MAGENTA "[%s] " COLOR_YELLOW "%s " COLOR_CYAN " [%s] [%s:%d]: " " %s\n",
                   buffer, "WARNING", l->thread, l->filename, l->line, l->message);
            break;
        case LOG_ADDINFO:
            printf(COLOR_CYAN "\t%s\n", l->message);
            break;
    }

    printf(COLOR_RESET);
}

void *logs_queue_func() {
    if (is_interrupt()) {
        pthread_exit((void *) 0);
    }

    sigset_t set, orig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigemptyset(&orig);
    pthread_sigmask(SIG_BLOCK, &set, &orig);

    if (is_interrupt()) {
        pthread_sigmask(SIG_SETMASK, &orig, 0);
        pthread_exit((void *) 0);
    }

    struct timespec ts = { 0 };
    ts.tv_sec = 0;
    ts.tv_nsec = 500000;

    while (true) {
        nanosleep(&ts, &ts);

        if (is_interrupt() && is_logs_queue_empty()) {
            pthread_sigmask(SIG_SETMASK, &orig, 0);
            break;
        }

        if (!is_logs_queue_empty()) {
            log_message *l = pop_log();
            print_message(l);
            free(l->message);
            free(l->thread);
            free(l);
        }
    }

    pthread_exit((void *) 0);
}

void log_debug(char *message, char *filename, int line, ...) {
    if (config_context.debug == 0) {
        return;
    }

    va_list args;
    va_start(args, line);

    log_message *l = allocate_memory(sizeof(log_message));
    l->time = get_time();
    l->filename = filename;
    l->type = LOG_DEBUG;
    l->line = line;

    pthread_t self = pthread_self();

    asprintf(&l->thread, "Thread %lu", (unsigned long int) (self));
    vasprintf(&l->message, message, args);

    if (config_context.logs_on) {
        push_log(l);
    } else {
        free(l->message);
        free(l->thread);
        free(l);
    }
}

void log_info(char *message, char *filename, int line, ...) {
    va_list args;
    va_start(args, line);
    log_message *l = allocate_memory(sizeof(log_message));

    l->time = get_time();
    l->filename = filename;
    l->type = LOG_INFO;
    l->line = line;

    pthread_t self = pthread_self();

    asprintf(&l->thread, "Thread %lu", (unsigned long int) (self));
    vasprintf(&l->message, message, args);

    if (config_context.logs_on) {
        push_log(l);
    } else {
        free(l->message);
        free(l->thread);
        free(l);
    }
}

void log_error(char *message, char *filename, int line, ...) {
    va_list args;
    va_start(args, line);
    log_message *l = allocate_memory(sizeof(log_message));

    l->time = get_time();
    l->filename = filename;
    l->type = LOG_ERROR;
    l->line = line;

    pthread_t self = pthread_self();

    asprintf(&l->thread, "Thread %lu", (unsigned long int) (self));
    vasprintf(&l->message, message, args);

    if (config_context.logs_on) {
        push_log(l);
    } else {
        free(l->message);
        free(l->thread);
        free(l);
    }
}

void log_warn(char *message, char *filename, int line, ...) {
    va_list args;
    va_start(args, line);
    log_message *l = allocate_memory(sizeof(log_message));

    l->time = get_time();
    l->filename = filename;
    l->type = LOG_WARN;
    l->line = line;

    pthread_t self = pthread_self();

    asprintf(&l->thread, "Thread %lu", (unsigned long int) (self));
    vasprintf(&l->message, message, args);

    if (config_context.logs_on) {
        push_log(l);
    } else {
        free(l->message);
        free(l->thread);
        free(l);
    }
}

void log_addinfo(char *message, ...) {
    va_list args;
    va_start(args, message);
    log_message *l = allocate_memory(sizeof(log_message));

    l->time = get_time();
    l->type = LOG_ADDINFO;

    pthread_t self = pthread_self();

    asprintf(&l->thread, "Thread %lu", (unsigned long int) (self));
    vasprintf(&l->message, message, args);

    if (config_context.logs_on) {
        push_log(l);
    } else {
        free(l->message);
        free(l->thread);
        free(l);
    }
}

void start_logger() {
    config_context.logs_on = 1;
    init_logs();
    pthread_create(&thread_logger, NULL, logs_queue_func, NULL);
}

void logger_finalize() {
    LOG_INFO("Остановка логгера", NULL);
    while (!is_logs_queue_empty()) {
        log_message *l = pop_log();
        print_message(l);
        free(l->message);
        free(l->thread);
        free(l);
    }
    pthread_join(thread_logger, NULL);
}

