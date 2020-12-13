#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "util.h"
#include "queue.h"

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

typedef struct node {
    log data;
    TAILQ_ENTRY(node) nodes;
} node;

void init_logs();
void push_log(log value);
log pop_log();
bool is_logs_queue_empty();