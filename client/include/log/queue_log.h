#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "util.h"

#define QMAX_LOGS 1000

typedef enum log_type {
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARN
} log_type;

typedef struct log {
    char *message;
    char *filename;
    int line;
    log_type type;
    struct tm time;
} log;

typedef struct queue {
    log elements[QMAX_LOGS];
    int head;
    int tail;
    int size;
} queue;

void init_logs();
void push_log(log *value);
log* pop_log();
bool is_logs_queue_empty();