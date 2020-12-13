#include "logs.h"

struct tm* get_time() {
    time_t t;
    struct tm *tm;
    t = time(NULL);
    tm = localtime(&t);

    return tm;
}

_Noreturn void *print_message() {
    while (1) {
        if (!is_logs_queue_empty()) {
            log *l = pop_log();
            char buffer[26];
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &l->time);

            switch (l->type) {
                case LOG_INFO:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_BLUE "%s    " COLOR_CYAN " [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "INFO", l->filename, l->line, l->message);
                    break;
                case LOG_ERROR:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_RED "%s   " COLOR_CYAN " [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "ERROR", l->filename, l->line, l->message);
                    break;
                case LOG_DEBUG:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_GREEN "%s   " COLOR_CYAN " [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "DEBUG", l->filename, l->line, l->message);
                    break;
                case LOG_WARN:
                    fprintf(stdout, COLOR_MAGENTA "[%s] " COLOR_YELLOW "%s " COLOR_CYAN " [%s:%d]: " COLOR_BLINK " %s\n",
                            buffer, "WARNING", l->filename, l->line, l->message);
                    break;
            }
        }
    }
}

void log_debug(char *message, char *filename, int line) {
    log l = { message, filename, line, LOG_DEBUG };
    struct tm *tm = get_time();
    l.time = *tm;
    push_log(&l);
}

void log_info(char *message, char *filename, int line) {
    log l = { message, filename, line, LOG_INFO };
    struct tm *tm = get_time();
    l.time = *tm;
    push_log(&l);
}

void log_error(char *message, char *filename, int line) {
    log l = { message, filename, line, LOG_ERROR };
    struct tm *tm = get_time();
    l.time = *tm;
    push_log(&l);
}

void log_warn(char *message, char *filename, int line) {
    log l = { message, filename, line, LOG_WARN };
    struct tm *tm = get_time();
    l.time = *tm;
    push_log(&l);
}